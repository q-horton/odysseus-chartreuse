#include <zephyr/shell/shell.h>

#include <stdlib.h>

#include "mqtt.h"

static int cmd_mqtt_connect(const struct shell *sh, size_t argc,
                           char **argv)
{
    int ret;

    char *addr = argv[1];
    int port = atoi(argv[2]);

    ret = mqtt_connect_to_broker(addr, port);
    if (ret != 0) {
        shell_error(sh, "Error (%d): MQTT connect failed: %s", ret, strerror(-ret));
    }
}

static int cmd_mqtt_publish(const struct shell *sh, size_t argc,
                           char **argv)
{
    mqtt_send_data(argv[1], argv[2]);

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_mqtt,
        SHELL_CMD_ARG(connect, NULL, "Usage: mqtt connect <broker ip> <port>", cmd_mqtt_connect, 3, 0),
        SHELL_CMD_ARG(publish, NULL, "Usage: mqtt publish <topic> <data>", cmd_mqtt_publish, 3, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(mqtt, &sub_mqtt, "MQTT commands", NULL);
