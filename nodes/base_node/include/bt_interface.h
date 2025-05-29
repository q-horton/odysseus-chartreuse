#ifndef BT_INTERFACE_H
#define BT_INTERFACE_H

#include <zephyr/kernel.h>
#include <stdint.h>

int bt_run(void);

extern uint8_t flags;
extern uint32_t current_time;

extern struct k_sem timer_access;

#endif // BT_INTERFACE_H
