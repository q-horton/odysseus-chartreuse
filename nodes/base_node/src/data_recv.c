#include <zephyr/kernel.h>

#include <stdint.h>

#include "sensordata.h"

#define DATA_RECV_STACK_SIZE 4096
#define DATA_RECV_PRIORITY 2

#define DATA_RECV_QUEUE_SIZE 100

K_MSGQ_DEFINE(queue_sensor_data, sizeof(struct SensorData), DATA_RECV_QUEUE_SIZE, 1);
