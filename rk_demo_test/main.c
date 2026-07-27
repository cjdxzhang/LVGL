#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <lvgl/lv_conf.h>
#include <lvgl/lvgl.h>

#include "gui_guider.h"
#include "lv_port_init.h"
#include "backlight_control.h"

lv_ui guider_ui;

static int quit = 0;

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);

    /* 桶体开机默认位于基站，屏幕物理朝向对应 180°。 */
    lv_port_init(0, 0, 180);
    setup_ui(&guider_ui);

	while (!quit) {
		lv_task_handler();
		usleep(5000);
	}
	backlight_control_deinit();

	return 0;
}
