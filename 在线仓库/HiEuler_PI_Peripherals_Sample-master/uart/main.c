#include "hi_uart.h"

static void uart_usage(char *prg_name)
{
    printf("usage: %s <device_node> <baud_rate>\n", prg_name);
    printf("   eg: %s /dev/ttyAMA3 115200\n", prg_name);
}

int main(int argc, char *argv[])
{
    if (argc != 3 || !strncmp(argv[1], "-h", 2))
    {
        uart_usage(argv[0]);
        return -1;
    }

    int ret = 0;
    char hiserialdev[32] = "/dev/ttyAMA3";

    memset(hiserialdev,0,sizeof(hiserialdev));
    sprintf(hiserialdev,"%s",argv[1]);
    int speed = atoi(argv[2]);

    int hiserfd = hi_serial_open(hiserialdev);
    hi_serial_init(hiserfd, speed, 0, 8, 1, 'N');

    hi_serial_send(hiserfd, "Serial Start", strlen("Serial Start"));

    while(1)
    {
        char recvbuf[256]= {0};
        ret = hi_serial_recv(hiserfd, recvbuf, sizeof(recvbuf));
        if(ret <= 0)
        {
            continue;
        }

        printf("Recv Data: %s ret: %d\n",recvbuf, ret);
        hi_serial_send(hiserfd, "hieuler", strlen("hieuler"));
    }

    hi_serial_close(hiserfd);
}
