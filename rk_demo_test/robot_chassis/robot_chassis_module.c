#include "robot_chassis_types.h"

_Static_assert(ROBOT_CHASSIS_FRAME_SUFFIX_SIZE == 4U, "底盘帧结束符长度必须为 4 字节");
_Static_assert(ROBOT_CHASSIS_UPGRADE_HEX_CHUNK_MAX == 512U * 1024U,
               "升级十六进制字符串分段上限必须为 512 KiB");

