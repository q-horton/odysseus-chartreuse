/**
 * Copyright (c) 2024 Croxel, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>

LOG_MODULE_REGISTER();

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

int main(void) {
	const struct device* const humtemp = device_get_binding("dht12");
	const struct device* const press = device_get_binding("bmp280");
	const struct device* const mag = device_get_binding("bmm150");

	printk("Device Pointer DHT: %p\n", humtemp);
	printk("Device Pointer BMP: %p\n", press);
	printk("Device Pointer BMM: %p\n", mag);
	if(!device_is_ready(humtemp)) {
		LOG_ERR("DHT12: Device not ready.\n");
	}
	if(!device_is_ready(press)) {
		LOG_ERR("BMP280: Device not ready.\n");
	}
	if(!device_is_ready(mag)) {
		LOG_ERR("BMM150: Device not ready.\n");
	}
	
	int err;
	for(int i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if(!adc_is_ready_dt(&adc_channels[0])) {
			LOG_ERR("ADC Controller %s is not ready\n", adc_channels[0].dev->name);
		}

		err = adc_channel_setup_dt(&adc_channels[0]);
		if(err) {
			LOG_ERR("Couldn't set up adc channel, code %d", err);
		}
	}

	struct sensor_value hum_val;
	struct sensor_value temp_val;
	struct sensor_value press_val;
	struct sensor_value mag_val;

	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

	while(true) {
		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

			err = adc_read_dt(&adc_channels[i], &sequence);
			if (err < 0) {
				printk("Could not read (%d)\n", err);
				continue;
			}

			int32_t val_mv = (int32_t)buf;
			err = adc_raw_to_millivolts_dt(&adc_channels[i], &val_mv);
			LOG_INF("Soil Moisture (ADC Channel %d): %d (%d mV)\n", 
					adc_channels[i].channel_id, (int32_t)buf, val_mv);
		}
//		// Humidity Temp
//		if(sensor_sample_fetch(humtemp)) {
//			LOG_WRN("DHT12: Sensor unable to fetch.\n");
//		}
//
//		sensor_channel_get(humtemp, SENSOR_CHAN_HUMIDITY, &hum_val);
//		int64_t humidity = sensor_value_to_micro(&hum_val);
//		if (humidity != 0) LOG_INF("DHT12 Hum: %lld\n", humidity);
//
//		sensor_channel_get(humtemp, SENSOR_CHAN_AMBIENT_TEMP, &temp_val);
//		int64_t temperature = sensor_value_to_milli(&temp_val);
//		if (temperature != 0) LOG_INF("DHT12 Temp: %lld\n", temperature);
//		
		// Pressure
		if(sensor_sample_fetch(press)) {
			LOG_WRN("BMP280: Sensor unable to fetch.\n");
		}

		sensor_channel_get(press, SENSOR_CHAN_PRESS, &press_val);
		int64_t pressure = sensor_value_to_micro(&press_val);
		if (pressure != 0) LOG_INF("BMP280 Press: %lld.%06lld\n", pressure / 1000000, pressure % 1000000);

		sensor_channel_get(press, SENSOR_CHAN_AMBIENT_TEMP, &temp_val);
		int64_t temperature = sensor_value_to_micro(&temp_val);
		if (temperature != 0) LOG_INF("BMP280 Temp: %lld.%06lld\n", temperature / 1000000, temperature % 1000000);
//		
//		// Magnetic
//		if(sensor_sample_fetch(press)) {
//			//LOG_WRN("BMM150: Sensor unable to fetch.\n");
//		}
//
//		sensor_channel_get(mag, SENSOR_CHAN_MAGN_X, &mag_val);
//		int64_t magnet = sensor_value_to_milli(&mag_val);
//		if (magnet != 0) LOG_INF("BMM150 Mag: %lld\n", magnet);

		k_msleep(3000);
	}

	return 0;
}
