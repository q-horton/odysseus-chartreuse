#ifndef SENSORS_H_
#define SENSORS_H_

#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>

#include "rb_sync.h"
#include "sys_time.h"
#include "utils.h"

#define MICRO_SCALING 1000000

void update_polling_rate(int rate);
int get_polling_rate(void);

#endif
