#include <zephyr/kernel.h>
#include <zephyr/data/json.h>

#include "sensordata.h"
#include "mqtt.h"
#include "filesystem.h"

#define DATA_PROC_STACK_SIZE 4096
#define DATA_PROC_PRIORITY 2

#define Kp 1
#define Ki 1
#define Kd 1

#define SET_POINT 50

struct json_obj_descr sensor_data_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct SensorData, nodeId, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, moisture, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, temperature, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, pressure, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct SensorData, timestamp, JSON_TOK_NUMBER),
};

void encode_sensor_data(struct SensorData *data, char *buf, int bufSize) {
    json_obj_encode_buf(sensor_data_descr, 5, data, buf, bufSize);
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

void data_proc_thread() {

    struct SensorData data;

    while (k_msgq_get(&queue_sensor_data, &data, K_FOREVER) == 0) {
        file_log_sensor_data(&data);
        k_msgq_put(&queue_pub_mqtt, &data, K_FOREVER);

    }
}

K_THREAD_DEFINE(data_proc_tid, DATA_PROC_STACK_SIZE, data_proc_thread, NULL, NULL, NULL, DATA_PROC_PRIORITY, 0, 0);
