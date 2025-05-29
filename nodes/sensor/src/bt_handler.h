#ifndef BT_HANDLER_H_
#define BT_HANDLER_H_

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sensors.h"
#include "sys_time.h"
#include "utils.h"

#define DEVICE_NAME "SensorNode-OC"
#define SENSOR_ADV_INTERVAL_MS 1000
#define NAME_LEN 30
#define RSSI_THRESHOLD -50 // RSSI threshold for filtering devices
#define MOBILE_NODE_MAC_ADDR "DC:B1:BE:10:38:1E (random)" // MAC address of Mobile Node
#define MIN_INTERVAL_FOR_UPDATES 10 // Minimum interval in seconds for updates

#define SENSOR_NODE_ID 0x01

#define PAYLOAD_SIZE 240
#define NUM_PAYLOADS 3

#endif
