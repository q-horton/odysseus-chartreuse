#ifndef RB_SYNC_H_
#define RB_SYNC_H_

#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "utils.h"

#define SP_BUFFER_ITEM_LENGTH 60
#define SP_BUFFER_SIZE (SP_BUFFER_ITEM_LENGTH * sizeof(SensorLoad))

uint8_t rbs_put(SensorLoad* packet);
uint8_t rbs_get(SensorLoad* packet);
bool rbs_data_available(void);

#endif
