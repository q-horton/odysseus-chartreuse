#ifndef MQTT_NODE_H
#define MQTT_NODE_H

int mqtt_connect_to_broker(char* broker_ip, int port);
void mqtt_send_data(char* topic, char* data);

#endif // MQTT_NODE_H
