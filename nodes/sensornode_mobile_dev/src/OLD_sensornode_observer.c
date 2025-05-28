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
#define NODE_OF_INTEREST "DC:B1:BE:10:38:1E (random)" // MAC address of Mobile Node

// array of MAC addresses to filter
const char *nodes_of_interest[] = {
	"C5:84:32:CA:99:CA (random)",
};

const bt_addr_le_t target_addr = {
	.type = BT_ADDR_LE_RANDOM, 
	.a = { .val = {0xC5, 0x84, 0x32, 0xCA, 0x99, 0xCA} }
};

static bool parse_ad_data(struct bt_data *data, void *user_data) {

    if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) {
        const uint8_t *payload = data->data;

        uint8_t counter = payload[0];
        int16_t temperature_raw = payload[1] | (payload[2] << 8);
        uint16_t battery_mv = payload[3] | (payload[4] << 8);
        //float temperature_c = ((float) temperature_raw) / 10.0f;

        printk("\t \t Sensor Data - Counter: %d, Temp: %d.%d°C, Battery: %u mV\n",
               counter, (int) temperature_raw / 10, temperature_raw % 10, battery_mv);
    }
    return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
	if (rssi < RSSI_THRESHOLD) {
		return; // Ignore weak signals
	}
	
	char addr_str[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Device found: %s (RSSI %d), type %u, AD data len %u\n",
	       addr_str, rssi, type, ad->len);

	
	// Check if the device is the one we are interested in - NODE_OF_INTEREST
	//printk("Comparing %s with %s\n", addr_str, NODE_OF_INTEREST);

	// if (bt_addr_le_cmp(addr, &target_addr) == 0) {
    // 	printk("Found target device: %s\n", addr_str);
	// }

	// Parse the advertisement data if the device is of interest
	// Check if the device is in the list of nodes of interest
	for (uint8_t i = 0; i < sizeof(nodes_of_interest) / sizeof(nodes_of_interest[0]); i++) {
		if (strcmp(addr_str, nodes_of_interest[i]) == 0) {
			//printk("Found target device: %s\n", addr_str);
			bt_data_parse(ad, parse_ad_data, NULL);
			k_sleep(K_MSEC(1000));
			//return;
		}
	}
	// if (strcmp(addr_str, NODE_OF_INTEREST) == 0) {
	// 	bt_data_parse(ad, parse_ad_data, NULL);
	// 	k_sleep(K_MSEC(1000));
	// }
	
	
}


int observer_start(void) {

	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
	int err;

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Start scanning failed (err %d)\n", err);
		return err;
	}
	printk("Started scanning...\n");

	return 0;
}
