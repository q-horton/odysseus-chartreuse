#include <zephyr/kernel.h>
#include <zephyr/data/json.h>

#include "sensordata.h"

#define Kp 1
#define Ki 1
#define Kd 1

#define SET_POINT 50

struct json_obj_descr sensor_data_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct SensorData, moisture, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, temperature, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, pressure, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, time, JSON_TOK_NUMBER)
};

void encode_sensor_data(struct SensorData *data, char *buf, int bufSize) {
    json_obj_encode_buf(sensor_data_descr, 4, data, buf, bufSize);
}

float pid_update(float value, float time, float *prev_integral, float *prev_error, float *last_time) {

    float error = SET_POINT - value;
    float dt = time - *last_time;

    float integral = *prev_integral + error * dt;
    float derivative = (error - *prev_error)/dt;

    *last_time = time;
    *prev_integral = integral;
    *prev_error = error;

    return Kp*error + Ki*integral + Kd*derivative;
}
