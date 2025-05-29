/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// MODIFIED FROM SAMPLE CODE, ADDED RSSI THRESHOLD, DATA PARSING, NODE SELECTION

#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <string.h>

#define NAME_LEN 30
#define RSSI_THRESHOLD -50 // RSSI threshold for filtering devices
#define MOBILE_NODE_MAC "DC:B1:BE:10:38:1E (random)"
#define MIN_INTERVAL_FOR_UPDATES 10

// array of MAC addresses to filter
const char *nodes_of_interest[] = {
	"DC:B1:BE:10:38:1E (random)"
};

// structure to store sensordata from various nodes scraped
struct node_of_interest {
	char mac_id[NAME_LEN];
	// timestamp
	uint32_t timestamp;
	uint8_t counter;
	int16_t temperature_raw;
	uint16_t battery_mv;
};
// store all sensordata scraped from the nodes of interest
struct node_of_interest nodes[10]; // array of 10 nodes

const bt_addr_le_t target_addr = {
	.type = BT_ADDR_LE_RANDOM, 
	.a = { .val = {0xC5, 0x84, 0x32, 0xCA, 0x99, 0xCA} }
};


static bool parse_ad_data(struct bt_data *data, void *user_data) {

    if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) {
        const uint8_t *payload = data->data;

		printk("[BASENODE-DEBUG] Received From Mobile: ");
		for (int i = 0; i < data->data_len; i++) {
			printk("%02X ", payload[i]);
		}
		printk("\n");
    }
    return true;
}

static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf) {
	
	if (info->rssi < RSSI_THRESHOLD) {
		//printk("Ignoring weak signal from %s (RSSI %d)\n", le_addr, info->rssi);
		return; // Ignore weak signals
	}

	if (!(info->adv_props & BT_GAP_ADV_PROP_EXT_ADV)) { 
        // Not extended advertising, skip it
        return;
    }
	
	char le_addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(info->addr, le_addr, sizeof(le_addr));
	printk("[BASENODE-LOG] DEVICE: %s, RSSI: %d, Data len: %u\n", le_addr, info->rssi, buf->len);
	
    if (strcmp(le_addr, MOBILE_NODE_MAC) == 0) {
        bt_data_parse(buf, parse_ad_data, NULL);
        //k_sleep(K_MSEC(1000));
    }
}

static struct bt_le_scan_cb scan_callbacks = {
	.recv = scan_recv,
};

int observer_start(void) {

	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		//.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
	int err;

	// Only applies for BLE using extended advertising
	bt_le_scan_cb_register(&scan_callbacks);
	printk("[BASENODE-LOG] Registered scan callbacks\n");
	
	err = bt_le_scan_start(&scan_param, NULL); // do nothing if legacy ble device found (previously calls device_found)
	if (err) {
		printk("Start scanning failed (err %d)\n", err);
		return err;
	}
	printk("[BASENODE-LOG] Started scanning...\n");

	return 0;
}