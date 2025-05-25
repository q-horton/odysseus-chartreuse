#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <stdlib.h>

#include "mqtt.h"

static int cmd_send_at(const struct shell *sh, size_t argc,
                           char **argv)
{
    send_to_at(argv[1]);

    return 0;
}

static int cmd_send_mqtt_conf(const struct shell *sh, size_t argc,
                           char **argv)
{
    char buf[200];

    snprintf(buf, 200, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", argv[1], argv[2], argv[3]);

    send_to_at(buf);
    return 0;
}

static int cmd_send_mqtt_conn(const struct shell *sh, size_t argc,
                           char **argv)
{
    char buf[200];

    snprintf(buf, 200, "AT+MQTTCONN=0,\"%s\",%s,1", argv[1], argv[2]);

    send_to_at(buf);
    return 0;
}

static int cmd_send_wifi_conn(const struct shell *sh, size_t argc,
                           char **argv)
{
    char buf[200];

    snprintf(buf, 200, "AT+CWMODE=1");
    send_to_at(buf);

    snprintf(buf, 200, "AT+CWJAP=\"%s\",\"%s\"", argv[1], argv[2]);
    send_to_at(buf);
    return 0;
}

static int cmd_mqtt_pub(const struct shell *sh, size_t argc,
                           char **argv)
{
    char buf[200];
    char sanitisedData[100];

    sanitize_at_arg(argv[2], sanitisedData);

    snprintf(buf, 200, "AT+MQTTPUB=0,\"%s\",\"%s\",1,0", argv[1], sanitisedData);
    send_to_at(buf);
    return 0;

}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_at,
        SHELL_CMD_ARG(send, NULL, "Usage: at send <MESSAGE>", cmd_send_at, 2, 0),
        SHELL_CMD_ARG(mqtt_conf, NULL, "Usage: at mqtt_conf <client> <user> <psk>", cmd_send_mqtt_conf, 4, 0),
        SHELL_CMD_ARG(mqtt_conn, NULL, "Usage: at mqtt_conn <ip> <port>", cmd_send_mqtt_conn, 3, 0),
        SHELL_CMD_ARG(wifi_conn, NULL, "Usage: at wifi_conn <ssid> <psk>", cmd_send_wifi_conn, 3, 0),
        SHELL_CMD_ARG(mqtt_pub, NULL, "Usage: at mqtt_pub <topic> <message>", cmd_mqtt_pub, 3, 0),
        SHELL_SUBCMD_SET_END
);


SHELL_CMD_ARG_REGISTER(at, &sub_at, "ESP-AT commands", cmd_send_at, 2, 0);

