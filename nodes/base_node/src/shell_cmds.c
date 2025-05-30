#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <stdlib.h>

#include "mqtt.h"
#include "sensordata.h"
#include "servo.h"
#include "filesystem.h"
#include "bt_interface.h"

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
    data.timestamp = atoi(argv[4]);

    k_msgq_put(&queue_sensor_data, &data, K_FOREVER);
    return 0;
}

static int cmd_sensorlog_enable(const struct shell *sh, size_t argc,
                           char **argv)
{
    file_log_enable(argv[1]);
    return 0;
}

static int cmd_sensorlog_disable(const struct shell *sh, size_t argc,
                           char **argv)
{
    file_log_disable();
    return 0;
}

static int cmd_sampling_set(const struct shell *sh, size_t argc,
                           char **argv)
{

    uint8_t rate = atoi(argv[1]);

    if (rate < 1 || rate > 255) {
        shell_error(sh, "Sampling rate must be between 1 and 255");
        return 1;
    }

    flags = rate;

    return 0;
}

static int cmd_sampling_disable(const struct shell *sh, size_t argc,
                           char **argv)
{
    flags = 0;
    return 0;
}

static int cmd_time_set(const struct shell *sh, size_t argc,
                           char **argv)
{
	k_sem_take(&timer_access, K_FOREVER);
    current_time = atoi(argv[1]);
    k_sem_give(&timer_access);
    return 0;
}

static int cmd_time_get(const struct shell *sh, size_t argc,
                           char **argv)
{
	k_sem_take(&timer_access, K_FOREVER);
    shell_print(sh, "%d", current_time);
    k_sem_give(&timer_access);
    return 0;
}

static int cmd_setpoint_set(const struct shell *sh, size_t argc,
                           char **argv)
{
    int val = atoi(argv[1]);

    if (val < 0 || val > 4095) {
        shell_error(sh, "Set point must be between 1 and 4095");
        return 1;
    }

    set_setpoint(val);

    return 0;
}

static int cmd_setpoint_get(const struct shell *sh, size_t argc,
                           char **argv)
{
    shell_print(sh, "%d", get_setpoint());
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

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensorlog,
        SHELL_CMD_ARG(enable, NULL, "Enable sensor logging to a specified file", cmd_sensorlog_enable, 2, 0),
        SHELL_CMD_ARG(disable, NULL, "Disable sensor logging", cmd_sensorlog_disable, 1, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sampling,
        SHELL_CMD_ARG(set, NULL, "Set sampling rate", cmd_sampling_set, 2, 0),
        SHELL_CMD_ARG(disable, NULL, "Disable sensor sampling", cmd_sampling_disable, 1, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_time,
        SHELL_CMD_ARG(set, NULL, "Set system time", cmd_time_set, 2, 0),
        SHELL_CMD_ARG(get, NULL, "Get system time", cmd_time_get, 1, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_setpoint,
        SHELL_CMD_ARG(set, NULL, "Set setpoint", cmd_setpoint_set, 2, 0),
        SHELL_CMD_ARG(get, NULL, "Get setpoint", cmd_setpoint_get, 1, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(espat, &sub_espat, "ESP-AT commands", NULL);
SHELL_CMD_REGISTER(servo, &sub_servo, "Servo motor commands", NULL);
SHELL_CMD_REGISTER(sensorlog, &sub_sensorlog, "Sensor FS logging commands", NULL);
SHELL_CMD_REGISTER(sampling, &sub_sampling, "Sensor sampling commands", NULL);

SHELL_CMD_REGISTER(time, &sub_time, "Clock time commands", NULL);
SHELL_CMD_REGISTER(setpoint, &sub_setpoint, "Setpoint commands", NULL);
