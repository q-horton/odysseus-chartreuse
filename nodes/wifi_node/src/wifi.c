#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>

#include <string.h>
#include <stdint.h>

#include "wifi.h"
#include "zephyr/net/net_if.h"
#include "zephyr/net/net_ip.h"
#include "zephyr/net/net_mgmt.h"
#include "zephyr/net/wifi.h"

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

static K_SEM_DEFINE(sem_wifi, 0, 1);
static K_SEM_DEFINE(sem_ipv4, 0, 1);

static void on_wifi_connection_event(struct net_mgmt_event_callback *cb, 
        uint32_t mgmt_event, 
        struct net_if *iface) 
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
        if (status->status) {
            printk("Error (%d): Connection request failed\n", status->status);
        } 
        else {
            printk("Connection successful.\n");
            k_sem_give(&sem_wifi);
        }
    }
    else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
        if (status->status) {
            printk("Error (%d): Disconnect request failed\n", status->status);
        } 
        else {
            printk("Disconnect successful.\n");
            k_sem_take(&sem_wifi, K_NO_WAIT);
        }
    }
}

static void on_ipv4_obtained(struct net_mgmt_event_callback *cb, 
        uint32_t mgmt_event, 
        struct net_if *iface) 
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        k_sem_give(&sem_ipv4);
    }
}

void wifi_init() {
    net_mgmt_init_event_callback(&wifi_cb, on_wifi_connection_event, NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_init_event_callback(&ipv4_cb, on_ipv4_obtained, NET_EVENT_IPV4_ADDR_ADD);

    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);
}

int wifi_connect(char *ssid, char* passkey) {
    int rv;
    struct net_if *iface;
    struct wifi_connect_req_params params;

    iface = net_if_get_default();

    params.ssid = (const uint8_t *)ssid;
    params.ssid_length = strlen(ssid);
    params.psk = (const uint8_t *)passkey;
    params.psk_length = strlen(passkey);

    params.security = WIFI_SECURITY_TYPE_PSK;
    params.band = WIFI_FREQ_BAND_UNKNOWN;
    params.channel = WIFI_CHANNEL_ANY;
    params.mfp = WIFI_MFP_OPTIONAL;

    rv = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
    k_sem_take(&sem_wifi, K_FOREVER);

    return rv;
}

void wifi_wait_for_ip(void) {
    struct wifi_iface_status status;
    struct net_if *iface;

    char ip_addr[NET_IPV4_ADDR_LEN];
    char gw_addr[NET_IPV4_ADDR_LEN];

    iface = net_if_get_default();

    k_sem_take(&sem_ipv4, K_FOREVER);

    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status))) {
        printk("Error: WiFi status request failed\n");
    }

    memset(ip_addr, 0, sizeof(ip_addr));

    if (net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[0].ipv4.address.in_addr, ip_addr, sizeof(ip_addr)) == NULL) {
        printk("Error: could not convert IP address to string\n");
    }

    memset(gw_addr, 0, sizeof(gw_addr));

    if (net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, gw_addr, sizeof(gw_addr)) == NULL) {
        printk("Error: could not convert gateway address to string\n");
    }


    printk("Wifi Status (%d):\n", status.state);
    if (status.state == WIFI_STATE_COMPLETED) {
        printk("    SSID: %-32s\n", status.ssid);
        printk("    IP address: %s\n", ip_addr);
        printk("    Gateway address: %s\n", gw_addr);
    }
}

int wifi_disconnect(void) {
    int ret;
    struct net_if *iface = net_if_get_default();

    ret = net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);
    return ret;
}
