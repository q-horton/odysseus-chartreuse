#ifndef WIFI_H
#define WIFI_H

void wifi_init(void);

void wifi_wait_for_ip(void);

int wifi_connect(char *ssid, char* passkey);
int wifi_disconnect(void);

#endif // WIFI_H
