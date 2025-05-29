#include "rb_sync.h"

LOG_MODULE_DECLARE(Main);

RING_BUF_DECLARE(sp_buffer, SP_BUFFER_SIZE);

K_SEM_DEFINE(spb_protect, 1, 1);

uint8_t rbs_put(SensorLoad* packet) {
	uint8_t* data = (void*)packet;
	uint32_t size = sizeof(*packet);
	if(!k_sem_take(&spb_protect, K_FOREVER)) {
		ring_buf_put(&sp_buffer, data, size);
		k_sem_give(&spb_protect);
		return 0;
	}
	return 1;
}

uint8_t rbs_get(SensorLoad* packet_buffer) {
	if(!k_sem_take(&spb_protect, K_FOREVER)) {
		uint32_t size = sizeof(*packet_buffer);
		uint8_t data[size];
		ring_buf_get(&sp_buffer, data, size);
		k_sem_give(&spb_protect);
		memcpy(packet_buffer, data, size);
		return 0;
	}
	return 1;
}

bool rbs_data_available(void) {
	return ring_buf_size_get(&sp_buffer) != 0;
}
