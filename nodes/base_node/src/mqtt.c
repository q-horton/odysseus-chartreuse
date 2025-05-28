#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <string.h>

#include "sensordata.h"

static const struct device *const espat_uart = DEVICE_DT_GET(DT_NODELABEL(espat_uart));

#define MSG_SIZE 100
#define ESP_MSG_BUF_SIZE 256
#define SENSOR_QUEUE_SIZE 50

// queue to store uart messages from ESP-AT
K_MSGQ_DEFINE(queue_espat_uart, MSG_SIZE, 10, 4);

// queue to store sensor data to be published
K_MSGQ_DEFINE(queue_sensor_data, sizeof(struct SensorData), SENSOR_QUEUE_SIZE, 1);

// semaphore to tell when ESP-AT is available
K_SEM_DEFINE(sem_espat, 1, 1);

// rx buffer for uart messages
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(espat_uart)) {
		return;
	}

	if (!uart_irq_rx_ready(espat_uart)) {
		return;
	}

	while (uart_fifo_read(espat_uart, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			rx_buf[rx_buf_pos] = '\0';
			k_msgq_put(&queue_espat_uart, &rx_buf, K_NO_WAIT);
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
	}
}

void espat_send(char *str) {
    k_sem_take(&sem_espat, K_FOREVER);

    for (char *c = str; *c != '\0'; c++) {
        uart_poll_out(espat_uart, *c);
    }
    uart_poll_out(espat_uart, '\r');
    uart_poll_out(espat_uart, '\n');
}

void espat_mqtt_configure(char *clientId, char *username, char *password) {
    char buf[ESP_MSG_BUF_SIZE];

    snprintf(
            buf, ESP_MSG_BUF_SIZE, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", 
            clientId, username, password
    );

    espat_send(buf);
}

void espat_mqtt_connect(char *ip, char *port) {
    char buf[ESP_MSG_BUF_SIZE];

    snprintf(buf, ESP_MSG_BUF_SIZE, "AT+MQTTCONN=0,\"%s\",%s,1", ip, port);
    espat_send(buf);
}

void espat_wifi_connect(char *ssid, char *psk) {
    char buf[ESP_MSG_BUF_SIZE];

    snprintf(buf, ESP_MSG_BUF_SIZE, "AT+CWMODE=1");
    espat_send(buf);

    snprintf(buf, ESP_MSG_BUF_SIZE, "AT+CWJAP=\"%s\",\"%s\"", ssid, psk);
    espat_send(buf);
}

void espat_mqtt_publish(char *topic, char *message) {
    char cmdBuf[ESP_MSG_BUF_SIZE];

    snprintf(cmdBuf, ESP_MSG_BUF_SIZE, "AT+MQTTPUBRAW=0,\"%s\",%d,1,0", topic, strlen(message));
    espat_send(cmdBuf);
    espat_send(message);
}

void espat_thread() {
    char tx_buf[MSG_SIZE];

    uart_irq_callback_user_data_set(espat_uart, serial_cb, NULL);
    uart_irq_rx_enable(espat_uart);

    while (k_msgq_get(&queue_espat_uart, &tx_buf, K_FOREVER) == 0) {

        if (strncmp(tx_buf, "\nOK", 3) == 0) {
            k_sem_give(&sem_espat);
        }
        else if (strncmp(tx_buf, "\nERROR", 6) == 0) {
            k_sem_give(&sem_espat);
        }
        else if (strncmp(tx_buf, "\n+MQTTPUB:OK", 12) == 0) {
            k_sem_give(&sem_espat);
        }

        printk("Rcvd from ESP-AT: %s\n", tx_buf);
	}
}

void mqtt_pub_thread() {

    struct SensorData sensorData;
    char sensorDataBuf[ESP_MSG_BUF_SIZE];

    while (1) {
        while (k_msgq_get(&queue_sensor_data, &sensorData, K_FOREVER) == 0) {
            encode_sensor_data(&sensorData, sensorDataBuf, ESP_MSG_BUF_SIZE);
            espat_mqtt_publish("sensors", sensorDataBuf);
        }

        k_msleep(10);
    }
}

K_THREAD_DEFINE(espat_tid, 4096, espat_thread, NULL, NULL, NULL, 2, 0, 0);
K_THREAD_DEFINE(mqtt_pub_tid, 4096, mqtt_pub_thread, NULL, NULL, NULL, 2, 0, 0);
