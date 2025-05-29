#include <zephyr/kernel.h>

#include <stdint.h>

#include "sensordata.h"

#define DATA_RECV_STACK_SIZE 4096
#define DATA_RECV_PRIORITY 2

#define DATA_RECV_QUEUE_SIZE 100

K_MSGQ_DEFINE(queue_sensor_data, sizeof(struct SensorData), DATA_RECV_QUEUE_SIZE, 1);

/*
void data_recv_thread() {

    uint32_t time = 1;

    while (1) {

        struct SensorData data;

        data.timestamp = time;
        data.moisture = (time % 2) ? 50 : 55;
        data.pressure = (time % 2) ? 101400 : 101600;
        data.temperature = (time % 2) ? 25 : 27;
        data.nodeId = 1;

        k_msgq_put(&queue_sensor_data, &data, K_FOREVER);

        time++;

        k_msleep(2000);
    }
}

K_THREAD_DEFINE(data_recv_tid, DATA_RECV_STACK_SIZE, data_recv_thread, NULL, NULL, NULL, DATA_RECV_PRIORITY, 0, 0);
*/
