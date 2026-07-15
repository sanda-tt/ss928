#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip0/"
#define PWM_EXPORT_PATH PWM_CHIP_PATH "export"
#define PWM_UNEXPORT_PATH PWM_CHIP_PATH "unexport"

#define PWM1_PERIOD_PATH PWM_CHIP_PATH "pwm1/period"
#define PWM1_DUTY_CYCLE_PATH PWM_CHIP_PATH "pwm1/duty_cycle"
#define PWM1_ENABLE_PATH PWM_CHIP_PATH "pwm1/enable"

#define PWM15_PERIOD_PATH PWM_CHIP_PATH "pwm15/period"
#define PWM15_DUTY_CYCLE_PATH PWM_CHIP_PATH "pwm15/duty_cycle"
#define PWM15_ENABLE_PATH PWM_CHIP_PATH "pwm15/enable"


void hi_export_pwm(int num)
{
    int export_fd = open(PWM_EXPORT_PATH, O_WRONLY);
    if (export_fd == -1) {
        perror("Error opening export");
        exit(EXIT_FAILURE);
    }

    dprintf (export_fd, "%d", num);

    close(export_fd);
}

void hi_disable_pwm(int num);

void hi_unexport_pwm(int num)
{
    int unexport_fd = open(PWM_UNEXPORT_PATH, O_WRONLY);
    if (unexport_fd == -1) {
        perror("Error opening unexport");
        exit(EXIT_FAILURE);
    }

    hi_disable_pwm(num);
    dprintf (unexport_fd, "%d", num);

    close(unexport_fd);
}

void hi_set_pwm_period(int num, int period)
{
    int period_fd;
    switch (num) {
        case 1:
            period_fd = open(PWM1_PERIOD_PATH, O_WRONLY);
            break;
        case 15:
            period_fd = open(PWM15_PERIOD_PATH, O_WRONLY);
            break;
        default:
            period_fd = open(PWM1_PERIOD_PATH, O_WRONLY);
    }

    if (period_fd == -1) {
        perror("Error opening period");
        exit(EXIT_FAILURE);
    }

    dprintf(period_fd, "%d", period);

    close(period_fd);
}

void hi_set_pwm_duty_cycle(int num, int duty_cycle)
{
    int duty_cycle_fd;
    switch (num) {
        case 1:
            duty_cycle_fd = open(PWM1_DUTY_CYCLE_PATH, O_WRONLY);
            break;
        case 15:
            duty_cycle_fd = open(PWM15_DUTY_CYCLE_PATH, O_WRONLY);
            break;
        default:
            duty_cycle_fd = open(PWM1_DUTY_CYCLE_PATH, O_WRONLY);
    }

    if (duty_cycle_fd == -1) {
        perror("Error opening duty cycle");
        exit(EXIT_FAILURE);
    }

    dprintf(duty_cycle_fd, "%d", duty_cycle);

    close(duty_cycle_fd);
}

void hi_enable_pwm(int num)
{
    int enable_fd;
    switch (num) {
        case 1:
            enable_fd = open(PWM1_ENABLE_PATH, O_WRONLY);
            break;
        case 15:
            enable_fd = open(PWM15_ENABLE_PATH, O_WRONLY);
            break;
        default:
            enable_fd = open(PWM1_ENABLE_PATH, O_WRONLY);
    }

    if (enable_fd == -1) {
        perror("Error opening enable");
        exit(EXIT_FAILURE);
    }

    if (write(enable_fd, "1", 2) == -1) {
        perror("Error enabling PWM");
        close(enable_fd);
        exit(EXIT_FAILURE);
    }

    close(enable_fd);
}

void hi_disable_pwm(int num)
{
    int enable_fd;
    switch (num) {
        case 1:
            enable_fd = open(PWM1_ENABLE_PATH, O_WRONLY);
            break;
        case 15:
            enable_fd = open(PWM15_ENABLE_PATH, O_WRONLY);
            break;
        default:
            enable_fd = open(PWM1_ENABLE_PATH, O_WRONLY);
    }

    if (enable_fd == -1) {
        perror("Error opening enable");
        exit(EXIT_FAILURE);
    }

    if (write(enable_fd, "0", 2) == -1) {
        perror("Error disabling PWM");
        close(enable_fd);
        exit(EXIT_FAILURE);
    }

    close(enable_fd);
}

static void pwm_usage(char *prg_name)
{
    printf("usage : %s <operation> <pwm_num> [<period> <duty_cycle>]\n", prg_name);
    printf("operation :\n");
    printf("    open  : export and enable PWM with specified period and duty_cycle\n");
    printf("    close : disable and unexport PWM\n");
    printf("\n");
    printf("pwm_num : PWM channel number, e.g., 1 or 15\n");
    printf("period  : PWM period in nanoseconds (required for 'open')\n");
    printf("duty_cycle : PWM duty cycle in nanoseconds (required for 'open')\n");
    printf("\n");
    printf("example:\n");
    printf("    %s open 1 20000000 2500000\n", prg_name);
    printf("    %s close 1\n", prg_name);
}


int main(int argc, char** argv)
{
    if ((argc == 3) && !strcmp(argv[1], "close")) {
        int num = atoi (argv[2]);
        hi_unexport_pwm(num);
        return 0;
    }
    else if ((argc == 5) && !strcmp(argv[1], "open")) {
        int num = atoi (argv[2]);
        int period = atoi (argv[3]);
        int duty_cycle = atoi (argv[4]);
        hi_export_pwm(num);
        hi_set_pwm_period(num, period);
        hi_set_pwm_duty_cycle(num, duty_cycle);
        hi_enable_pwm(num);
    }
    else {
        pwm_usage(argv[0]);
        return 1;
    }

    return 0;
}