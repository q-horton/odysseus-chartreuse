#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/drivers/uart.h>

static const struct device *const at_uart = DEVICE_DT_GET(DT_NODELABEL(at_uart));

#define MSG_SIZE 100

// queue to store uart messages from ESP-AT
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

// semaphore to tell when ESP-AT is available
K_SEM_DEFINE(sem_at, 1, 1);

// rx buffer for uart messages
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(at_uart)) {
		return;
	}

	if (!uart_irq_rx_ready(at_uart)) {
		return;
	}

	while (uart_fifo_read(at_uart, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			rx_buf[rx_buf_pos] = '\0';
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
	}
}

void send_to_at(char *str) {
    k_sem_take(&sem_at, K_FOREVER);

    for (char *c = str; *c != '\0'; c++) {
        uart_poll_out(at_uart, *c);
    }
    uart_poll_out(at_uart, '\r');
    uart_poll_out(at_uart, '\n');
}

void sanitize_at_arg(char *str, char *outbuf) {
    int outbufPos = 0;

    for (char *c = str; *c != '\0'; c++) {
        if (*c == '"') {
            outbuf[outbufPos] = '\\';
            outbuf[outbufPos+1] = '"';
            outbufPos += 2;
        }
        else {
            outbuf[outbufPos] = *c;
            outbufPos++;
        }
    }

    outbuf[outbufPos] = '\0';
}

void mqtt_thread() {
    char tx_buf[MSG_SIZE];

    uart_irq_callback_user_data_set(at_uart, serial_cb, NULL);
    uart_irq_rx_enable(at_uart);

    while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {

        if (strncmp(tx_buf, "\nOK", 3) == 0) {
            k_sem_give(&sem_at);
        }
        else if (strncmp(tx_buf, "\nERROR", 6) == 0) {
            k_sem_give(&sem_at);
        }

        printk("Got: %s\n", tx_buf);
	}
}

K_THREAD_DEFINE(mqtt_tid, 4096, mqtt_thread, NULL, NULL, NULL, 2, 0, 0);
