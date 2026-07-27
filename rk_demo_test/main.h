#ifndef __MAIN_H__
#define __MAIN_H__

#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <lvgl/lvgl.h>

#include "lv_port_init.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

#endif