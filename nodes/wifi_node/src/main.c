#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "wifi.h"
#include "mqtt.h"

//#define SSID "csse4011"
//#define PASSWORD "csse4011wifi"
#define SSID "NOKIA-F7C1"
#define PASSWORD "59a372e43d"

#define MQTT_BROKER_PORT 1883
#define MQTT_BROKER_ADDR "192.168.0.114"

int main() {
    int ret;

    wifi_init();

    ret = wifi_connect(SSID, PASSWORD);
    if (ret < 0) {
        printk("Error (%d): WiFi connection failed\n", ret);
        return 0;
    }

    wifi_wait_for_ip();

    return 0;
}
