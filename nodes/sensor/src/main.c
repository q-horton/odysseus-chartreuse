#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "bt_handler.h"
#include "rb_sync.h"
#include "sensors.h"
#include "sys_time.h"

LOG_MODULE_REGISTER(Main);

extern struct k_msgq sample_stream;

int main(void) {
	SensorLoad sl;

	while(1) {
		if(rbs_data_available()) {
			if(!rbs_get(&sl)) {
				k_msgq_put(&sample_stream, &sl, K_FOREVER);
				LOG_INF("Sensors:\r\n-----\r\nTime: %d\r\nSoil Moisture: %d\r\nTemperature: %d\r\nHumidity: %d\r\nPressure: %d\r\n",
						sl.timestamp, sl.soil_moisture, sl.temp, sl.humidity, sl.pressure + (100 * 1000));
			}
		}
		k_msleep(1000);
	}
}

