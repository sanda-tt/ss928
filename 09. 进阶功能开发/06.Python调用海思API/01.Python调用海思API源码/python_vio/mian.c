#include "python_vio.h"

static sample_vi_cfg g_vi_config;

int g_sig_flag = 0;
sample_qt_switch_t g_qt_switch = {TD_FALSE, TD_TRUE};

static td_void sample_vio_handle_sig(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_sig_flag = 1;
    }
}

static td_void sample_register_sig_handler(td_void (*sig_handle)(int))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

int main(int argc, td_char *argv[])
{
    int ret;
    int sns_dev = 0;  //sensor0/1

    sample_register_sig_handler(sample_vio_handle_sig);

    ret = sample_vio_start_one_sensor(sns_dev, &g_vi_config, &g_qt_switch);
    if (ret != TD_SUCCESS) {
        printf("\033[0;31mstart one sensor failed!\033[0;39m\n");
        g_sig_flag = 1;
    }

    printf("\033[0;32mstart one sensor success!\033[0;39m\n");

    while (!g_sig_flag) {
        sleep(1);
    }

    sample_vio_stop_one_sensor(&g_vi_config, &g_qt_switch);

    printf("\033[0;32mstop one sensor success!\033[0;39m\n");

    return ret;
}
