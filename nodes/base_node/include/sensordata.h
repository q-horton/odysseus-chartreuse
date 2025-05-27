#ifndef SENSORDATA_H
#define SENSORDATA_H

struct SensorData {
    int moisture;
    int temperature;
    int pressure;
    int time;
};

void encode_sensor_data(struct SensorData *data, char *buf, int bufSize);

#endif // SENSORDATA_H
