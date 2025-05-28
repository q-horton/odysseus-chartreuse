#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <stdlib.h>

#include "mqtt.h"
#include "sensordata.h"
#include "servo.h"

static int cmd_espat_send(const struct shell *sh, size_t argc,
                           char **argv)
{
    espat_send(argv[1]);
    return 0;
}

static int cmd_espat_mqtt_conn(const struct shell *sh, size_t argc,
                           char **argv)
{
    espat_mqtt_configure("odysseus-chartreuse", "basenode", "");
    espat_mqtt_connect(argv[1], argv[2]);
    return 0;
}

static int cmd_espat_wifi_conn(const struct shell *sh, size_t argc,
                           char **argv)
{
    espat_wifi_connect(argv[1], argv[2]);
    return 0;
}

static int cmd_espat_mqtt_pub(const struct shell *sh, size_t argc,
                           char **argv)
{
    espat_mqtt_publish(argv[1], argv[2]);
    return 0;

}

static int cmd_servo_pan(const struct shell *sh, size_t argc,
                           char **argv)
{
    set_pan_angle(atoi(argv[1]));
    return 0;
}

static int cmd_servo_tilt(const struct shell *sh, size_t argc,
                           char **argv)
{
    set_tilt_angle(atoi(argv[1]));
    return 0;
}

static int cmd_servo_home(const struct shell *sh, size_t argc,
                           char **argv)
{
    set_pan_angle(PAN_HOME);
    set_tilt_angle(TILT_DAM_UP);
    return 0;
}

static int cmd_espat_pubsensor(const struct shell *sh, size_t argc,
                           char **argv)
{
    struct SensorData data;

    data.moisture = atoi(argv[1]);
    data.temperature = atoi(argv[2]);
    data.pressure = atoi(argv[3]);
    data.time = atoi(argv[4]);

    k_msgq_put(&queue_sensor_data, &data, K_FOREVER);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_espat,
        SHELL_CMD_ARG(send, NULL, "Usage: espat send <MESSAGE>", cmd_espat_send, 2, 0),
        SHELL_CMD_ARG(mqtt_conn, NULL, "Usage: espat mqtt_conn <ip> <port>", cmd_espat_mqtt_conn, 3, 0),
        SHELL_CMD_ARG(wifi_conn, NULL, "Usage: espat wifi_conn <ssid> <psk>", cmd_espat_wifi_conn, 3, 0),
        SHELL_CMD_ARG(mqtt_pub, NULL, "Usage: espat mqtt_pub <topic> <message>", cmd_espat_mqtt_pub, 3, 0),
        SHELL_CMD_ARG(pubsensor, NULL, "test command", cmd_espat_pubsensor, 5, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_servo,
        SHELL_CMD_ARG(pan, NULL, "Set pan servo angle", cmd_servo_pan, 2, 0),
        SHELL_CMD_ARG(tilt, NULL, "Set tilt servo angle", cmd_servo_tilt, 2, 0),
        SHELL_CMD_ARG(home, NULL, "Set servos to home", cmd_servo_home, 1, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(espat, &sub_espat, "ESP-AT commands", NULL);
SHELL_CMD_REGISTER(servo, &sub_servo, "Servo motor commands", NULL);
