#ifndef MQTT_H
#define MQTT_H

#include <zephyr/kernel.h>

extern struct k_msgq queue_pub_mqtt;

void espat_send(char *str);
void espat_sanitize_arg(char *str, char *outbuf);
void espat_mqtt_configure(char *clientId, char *username, char *password);
void espat_mqtt_connect(char *ip, char *port);
void espat_wifi_connect(char *ssid, char *psk);
void espat_mqtt_publish(char *topic, char *message);

#endif // MQTT_H
