#ifndef SYS_TIME_C_
#define SYS_TIME_C_

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "utils.h"

uint32_t get_sys_time(void);
void update_sys_time(uint32_t time_ms);

#endif
