#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>

#include "lv_port_init.h"
#include "uart_manager.h"
#include "screens/screen_home.h"

#if ROCKIT_EN
#include "rk_defines.h"
#include "rk_mpi_sys.h"
#endif

#if WIFIBT_EN
#include "wifibt.h"
#endif

static int quit = 0;

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

int main(int argc, char **argv)
{
    // 将标准输出（stdout）设置为行缓冲（Line Buffered）模式。
    setvbuf(stdout, NULL, _IOLBF, 0);
    // ctrl c to exit
    signal(SIGINT, sigterm_handler);
    // kill to exit
    signal(SIGTERM, sigterm_handler);

    struct sched_param param;
    int max_priority;

    // 将当前进程设置为实时调度策略 SCHED_FIFO，并使用系统允许的最高优先级
    max_priority = sched_get_priority_max(SCHED_FIFO);
    param.sched_priority = max_priority;

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
    {
        perror("sched_setscheduler failed");
    }

#if ROCKIT_EN
    RK_MPI_SYS_Init();
#endif
#if WIFIBT_EN
    run_wifibt_server();
#endif

    if (uart_manager_init("/dev/ttyS3", 115200) == 0) {
        fprintf(stderr, "[MAIN] UART Manager initialized successfully\n");
    } else {
        fprintf(stderr, "[MAIN] UART initialization failed\n");
    }

    lv_port_init(800, 480, 0);

    screen_home_create();

    while (!quit)
    {
        uint32_t sleep_time_ms = lv_timer_handler();
        if (sleep_time_ms == LV_NO_TIMER_READY) {
            sleep_time_ms = LV_DEF_REFR_PERIOD;
        }
        usleep(sleep_time_ms * 1000);
    }

#if ROCKIT_EN
    RK_MPI_SYS_Exit();
#endif

    return 0;
}
