#include "sys_time.h"

LOG_MODULE_DECLARE(Main);

K_SEM_DEFINE(timer_protect, 0, 1);

volatile uint32_t current_time = 0;

static struct k_timer msec_timer;

void timer_handler(struct k_timer* dummy) {
	k_sem_take(&timer_protect, K_FOREVER);
	current_time++;
	k_sem_give(&timer_protect);
}

uint32_t get_sys_time(void) {
	k_sem_take(&timer_protect, K_FOREVER);
	uint32_t time = current_time;
	k_sem_give(&timer_protect);
	return time;
}

void update_sys_time(uint32_t time_ms) {
	k_sem_take(&timer_protect, K_FOREVER);
	current_time = time_ms;
	k_sem_give(&timer_protect);
}

void timer_management_t(void) {
	k_timer_init(&msec_timer, timer_handler, NULL);
	k_timer_start(&msec_timer, K_MSEC(1), K_MSEC(1));
	k_sem_give(&timer_protect);
	
	while(1) {
		k_msleep(100);
	}

}

K_THREAD_DEFINE(timer_man_id, STACK_SIZE, timer_management_t, NULL, NULL, NULL, PRIORITY, 0, 0);
