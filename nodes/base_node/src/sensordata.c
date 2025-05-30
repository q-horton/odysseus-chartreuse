#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/data/json.h>

#include "sensordata.h"
#include "mqtt.h"
#include "filesystem.h"
#include "servo.h"

#include "zephyr/logging/log.h"

LOG_MODULE_REGISTER(basenode_pid);

#define DATA_PROC_STACK_SIZE 4096
#define DATA_PROC_PRIORITY 2

#define Kp 0.01
#define Ki 0.002
#define Kd 0.05

#define ANGLE_OFFSET 25
#define ANGLE_SCALER 2.5

#define INTEGRAL_PREV_WEIGHT 0.05

K_MUTEX_DEFINE(mut_setpoint);

int setPoint = 1000;

void set_setpoint(int val) {
    k_mutex_lock(&mut_setpoint, K_FOREVER);
    setPoint = val;
    k_mutex_unlock(&mut_setpoint);
}

int get_setpoint(void) {
    int val;

    k_mutex_lock(&mut_setpoint, K_FOREVER);
    val = setPoint;
    k_mutex_unlock(&mut_setpoint);

    return val;
}

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

void pid_update(int value, uint32_t time, int *angle) {
    static int prev_error = 0;
    static int prev_integral = 0;
    static uint32_t last_time = 0;

    k_mutex_lock(&mut_setpoint, K_FOREVER);
    int error = setPoint - value;
    k_mutex_unlock(&mut_setpoint);

    double dt = (time - last_time)/1000.0;

    if (dt == 0) {
        return;
    }

    double integral = Ki * (prev_integral + INTEGRAL_PREV_WEIGHT * error * dt);
    double derivative = ((double)(error - prev_error))/((double) dt);

    last_time = time;
    prev_integral = integral;
    prev_error = error;

    double pid = Kp*error + Ki*integral + Kd*derivative;


    double angleVal = ANGLE_SCALER * (ANGLE_OFFSET + pid) + TILT_DAM_UP;

    if (angleVal > TILT_DAM_DOWN) {
        angleVal = TILT_DAM_DOWN;
    }
    else if (angleVal < TILT_DAM_UP) {
        angleVal = TILT_DAM_UP;
    }

    *angle = (int) angleVal;

    LOG_INF("PID (error %d) (int %f) (deriv %f) (pid %f) (dt %f) (angle %f)", error, integral, derivative, pid, dt, angleVal);
}

void data_proc_thread() {
    struct SensorData data;

    int servoAngle = TILT_DAM_UP;

    while (k_msgq_get(&queue_sensor_data, &data, K_FOREVER) == 0) {
        file_log_sensor_data(&data);

        pid_update(data.moisture, data.timestamp, &servoAngle);
        set_tilt_angle(servoAngle);

        k_msgq_put(&queue_pub_mqtt, &data, K_FOREVER);
    }
}

K_THREAD_DEFINE(data_proc_tid, DATA_PROC_STACK_SIZE, data_proc_thread, NULL, NULL, NULL, DATA_PROC_PRIORITY, 0, 0);
