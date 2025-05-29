#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <stdint.h>

extern struct k_msgq queue_sensor_data;

struct SensorData {
    int nodeId;
    uint32_t timestamp;
    uint32_t temperature;
    uint32_t pressure;
    uint32_t moisture;
};

void encode_sensor_data(struct SensorData *data, char *buf, int bufSize);

#endif // SENSORDATA_H
