#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>

#include "mqtt.h"

#define MQTT_THREAD_STACK_SIZE 4096
#define MQTT_THREAD_PRIORITY 1

/* Buffers for MQTT client. */
static uint8_t rx_buffer[256];
static uint8_t tx_buffer[256];

/* MQTT client context */
static struct mqtt_client client_ctx;

/* MQTT Broker address information. */
static struct sockaddr_storage broker;

static struct pollfd fds[1];

static K_SEM_DEFINE(sem_mqtt, 0, 1);

void mqtt_evt_handler(struct mqtt_client *client,
        const struct mqtt_evt *evt)
{
    //printk("MQTT Event: %d\n", evt->type);

    switch (evt->type) {
        /* Handle events here. */
        case MQTT_EVT_CONNACK:
            k_sem_give(&sem_mqtt);
            break;

        case MQTT_EVT_DISCONNECT:
            k_sem_take(&sem_mqtt, K_NO_WAIT);
            break;

        default:
            break;
    }
}

int mqtt_connect_to_broker(char* broker_ip, int port) {
    int ret;

    mqtt_client_init(&client_ctx);

    struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;
    broker4->sin_family = AF_INET;
    broker4->sin_port = htons(port);
    inet_pton(AF_INET, broker_ip, &broker4->sin_addr);

    // MQTT client configuration
    client_ctx.broker = &broker;
    client_ctx.evt_cb = mqtt_evt_handler;
    client_ctx.client_id.utf8 = (uint8_t *)"zephyr_mqtt_client";
    client_ctx.client_id.size = sizeof("zephyr_mqtt_client") - 1;
    client_ctx.password = NULL;
    client_ctx.user_name = NULL;
    client_ctx.protocol_version = MQTT_VERSION_3_1_1;
    client_ctx.transport.type = MQTT_TRANSPORT_NON_SECURE;

    // MQTT buffers configuration
    client_ctx.rx_buf = rx_buffer;
    client_ctx.rx_buf_size = sizeof(rx_buffer);
    client_ctx.tx_buf = tx_buffer;
    client_ctx.tx_buf_size = sizeof(tx_buffer);

    ret = mqtt_connect(&client_ctx);
    if (ret != 0) {
        return ret;
    }

    fds[0].fd = client_ctx.transport.tcp.sock;
    fds[0].events = ZSOCK_POLLIN;
    poll(fds, 1, 5000);
    
    mqtt_input(&client_ctx);

    
    return ret;
}

void mqtt_send_data(char* topic, char* data) {
    struct mqtt_publish_param param = { 0 };
    param.message.topic.topic.utf8 = (uint8_t *)topic;
    param.message.topic.topic.size =
        strlen(param.message.topic.topic.utf8);
    param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.payload.data = data;
    param.message.payload.len =
        strlen(param.message.payload.data);
    param.message_id = 43;
    param.dup_flag = 0U;
    param.retain_flag = 0U;
    
    mqtt_publish(&client_ctx, &param);
}

void mqtt_thread() {
    while (1) {

        if (k_sem_count_get(&sem_mqtt) == 1) {
            mqtt_live(&client_ctx);
        }

        k_msleep(1000);    
    }
}

K_THREAD_DEFINE(mqtt_tid, MQTT_THREAD_STACK_SIZE,
                mqtt_thread, NULL, NULL, NULL,
                MQTT_THREAD_PRIORITY, 0, 0);
