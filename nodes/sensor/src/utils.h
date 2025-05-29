#ifndef SENSOR_UTILS_H_
#define SENSOR_UTILS_H_

#include <stdint.h>

/* Use 1KiB Stack */
#define STACK_SIZE 1024

/* Consistent Priority */
#define PRIORITY 7

typedef struct {
	uint32_t timestamp;
	uint16_t soil_moisture;
	uint16_t temp;
	uint16_t humidity;
	int16_t pressure;
} SensorLoad;

#endif
