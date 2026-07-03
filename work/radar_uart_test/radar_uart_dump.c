#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#define RX_BUF_SIZE 4096

static volatile sig_atomic_t g_stop = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_stop = 1;
}

static speed_t baud_to_flag(int baud)
{
    switch (baud) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
#ifdef B230400
    case 230400: return B230400;
#endif
#ifdef B460800
    case 460800: return B460800;
#endif
#ifdef B576000
    case 576000: return B576000;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
#ifdef B1000000
    case 1000000: return B1000000;
#endif
    default:
        return 0;
    }
}

static int configure_uart(int fd, int baud)
{
    struct termios tio;
    speed_t speed = baud_to_flag(baud);

    if (speed == 0) {
        fprintf(stderr, "Unsupported baud rate: %d\n", baud);
        return -1;
    }

    if (tcgetattr(fd, &tio) != 0) {
        perror("tcgetattr");
        return -1;
    }

    cfmakeraw(&tio);
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;

    if (cfsetispeed(&tio, speed) != 0 || cfsetospeed(&tio, speed) != 0) {
        perror("cfset*speed");
        return -1;
    }

    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        perror("tcsetattr");
        return -1;
    }

    tcflush(fd, TCIOFLUSH);
    return 0;
}

static uint16_t sum16(const uint8_t *data, size_t len)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += data[i];
    }
    return (uint16_t)sum;
}

static uint8_t sum8(const uint8_t *data, size_t len)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += data[i];
    }
    return (uint8_t)sum;
}

static int16_t le_s16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t le_u16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t le_u32(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void print_hex(const char *prefix, const uint8_t *data, size_t len)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("%ld.%03ld %s (%zu):", (long)tv.tv_sec, (long)(tv.tv_usec / 1000), prefix, len);
    for (size_t i = 0; i < len; ++i) {
        printf(" %02X", data[i]);
    }
    putchar('\n');
}

static void print_report(uint8_t type, const uint8_t *payload, size_t len)
{
    printf("  REPORT type=0x%02X payload_len=%zu\n", type, len);

    if (type == 0x07 && len >= 4) {
        uint16_t obj_num = le_u16(payload);
        uint16_t reserved = le_u16(payload + 2);
        size_t available = (len - 4) / 4;
        size_t count = obj_num < available ? obj_num : available;

        printf("    BSD obj_num=%u reserved=0x%04X decoded=%zu\n", obj_num, reserved, count);
        for (size_t i = 0; i < count; ++i) {
            const uint8_t *obj = payload + 4 + i * 4;
            printf("    obj[%zu]: range=%d m angle=%d deg velocity=%d m/s id=%d\n",
                   i,
                   (int)(int8_t)obj[0],
                   (int)(int8_t)obj[1],
                   (int)(int8_t)obj[2],
                   (int)(int8_t)obj[3]);
        }
        return;
    }

    if ((type == 0x00 || type == 0x03) && len >= 8) {
        printf("    detected=%u det_result=0x%02X range=%u mm angle=%d deg velocity=%d\n",
               payload[0], payload[1], le_u16(payload + 2), le_s16(payload + 4), le_s16(payload + 6));
        if (type == 0x00 && len >= 20) {
            printf("    rb_conf=%u angle_conf=%u frame_idx=%u\n",
                   payload[14], payload[15], le_u32(payload + 16));
        }
        return;
    }
}

static size_t parse_frames(uint8_t *buf, size_t len)
{
    size_t pos = 0;

    while (pos < len) {
        if (buf[pos] != 0x5A && buf[pos] != 0x59) {
            ++pos;
            continue;
        }

        if (buf[pos] == 0x5A) {
            if (len - pos < 4) {
                break;
            }

            uint8_t payload_len = buf[pos + 1];
            size_t frame_len = 2u + payload_len + 1u;
            if (payload_len == 0) {
                ++pos;
                continue;
            }
            if (len - pos < frame_len) {
                break;
            }

            uint8_t check = sum8(buf + pos, 2u + payload_len);
            if (check == buf[pos + frame_len - 1]) {
                print_hex("FRAME 0x5A", buf + pos, frame_len);
                print_report(buf[pos + 2], buf + pos + 3, payload_len - 1u);
                pos += frame_len;
            } else {
                printf("  Bad 0x5A checksum at buffer offset %zu\n", pos);
                ++pos;
            }
            continue;
        }

        if (buf[pos] == 0x59) {
            if (len - pos < 5) {
                break;
            }

            uint8_t param_len = buf[pos + 2];
            size_t frame_len = 5u + param_len;
            if (len - pos < frame_len) {
                break;
            }

            uint16_t expect = sum16(buf + pos, 3u + param_len);
            uint16_t got = le_u16(buf + pos + 3u + param_len);
            if (expect == got) {
                print_hex("CI response", buf + pos, frame_len);
                printf("    cmd=0x%02X param_len=%u\n", buf[pos + 1], param_len);
                pos += frame_len;
            } else {
                printf("  Bad 0x59 checksum at buffer offset %zu\n", pos);
                ++pos;
            }
        }
    }

    if (pos > 0 && pos < len) {
        memmove(buf, buf + pos, len - pos);
    }

    return len - pos;
}

static int send_probe(int fd)
{
    const uint8_t get_version[] = {0x58, 0xFE, 0x00, 0x56, 0x01};
    ssize_t ret = write(fd, get_version, sizeof(get_version));
    if (ret != (ssize_t)sizeof(get_version)) {
        perror("write probe");
        return -1;
    }
    print_hex("TX get_version", get_version, sizeof(get_version));
    return 0;
}

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [device] [baud] [--probe]\n", prog);
    fprintf(stderr, "Example: sudo %s /dev/ttyAMA4 921600 --probe\n", prog);
}

int main(int argc, char **argv)
{
    const char *device = "/dev/ttyAMA4";
    int baud = 921600;
    bool probe = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--probe") == 0) {
            probe = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '/') {
            device = argv[i];
        } else {
            baud = atoi(argv[i]);
            if (baud <= 0) {
                usage(argv[0]);
                return 1;
            }
        }
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror(device);
        return 1;
    }

    if (configure_uart(fd, baud) != 0) {
        close(fd);
        return 1;
    }

    printf("Listening on %s at %d bps, 8N1. Press Ctrl+C to stop.\n", device, baud);
    if (probe) {
        (void)send_probe(fd);
    }

    uint8_t frame_buf[RX_BUF_SIZE];
    size_t frame_len = 0;
    struct pollfd pfd = {.fd = fd, .events = POLLIN};

    while (!g_stop) {
        int pret = poll(&pfd, 1, 500);
        if (pret < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("poll");
            break;
        }
        if (pret == 0 || !(pfd.revents & POLLIN)) {
            continue;
        }

        uint8_t chunk[512];
        ssize_t n = read(fd, chunk, sizeof(chunk));
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                continue;
            }
            perror("read");
            break;
        }
        if (n == 0) {
            continue;
        }

        print_hex("RX", chunk, (size_t)n);

        if (frame_len + (size_t)n > sizeof(frame_buf)) {
            fprintf(stderr, "Parser buffer overflow, dropping buffered bytes\n");
            frame_len = 0;
        }
        memcpy(frame_buf + frame_len, chunk, (size_t)n);
        frame_len += (size_t)n;
        frame_len = parse_frames(frame_buf, frame_len);
    }

    close(fd);
    puts("Stopped.");
    return 0;
}
