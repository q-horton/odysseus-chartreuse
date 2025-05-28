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

// array of MAC addresses to filter
const char *nodes_of_interest[] = {
	"C5:84:32:CA:99:CA (random)"
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

        uint8_t counter = payload[0];
        int16_t temperature_raw = payload[1] | (payload[2] << 8);
        uint16_t battery_mv = payload[3] | (payload[4] << 8);
        //float temperature_c = ((float) temperature_raw) / 10.0f;

        printk("\t \t Sensor Data - Counter: %d, Temp: %d.%d°C, Battery: %u mV\n",
               counter, (int) temperature_raw / 10, temperature_raw % 10, battery_mv);

		printk("\t \t [DEBUG] Raw AD data: ");
		for (int i = 0; i < data->data_len; i++) {
			printk("%02X ", payload[i]);
		}
		printk("\n");
    }
    return true;
}

// static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
	
// 	if (rssi < RSSI_THRESHOLD) {
// 		return; // Ignore weak signals
// 	}
	
// 	char addr_str[BT_ADDR_LE_STR_LEN];

// 	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
// 	printk("Device found: %s (RSSI %d), type %u, AD data len %u\n",
// 	       addr_str, rssi, type, ad->len);

// 	// Parse the advertisement data if the device is of interest
// 	for (uint8_t i = 0; i < sizeof(nodes_of_interest) / sizeof(nodes_of_interest[0]); i++) {
// 		if (strcmp(addr_str, nodes_of_interest[i]) == 0) {
// 			//printk("Found target device: %s\n", addr_str);
// 			bt_data_parse(ad, parse_ad_data, NULL);
// 			k_sleep(K_MSEC(1000));
// 			//return;
// 		}
// 	}	
	
// }

#if defined(CONFIG_BT_EXT_ADV)
	// static bool data_cb(struct bt_data *data, void *user_data) {
	// 	char *name = user_data;
	// 	uint8_t len;

	// 	switch (data->type) {
	// 		case BT_DATA_NAME_SHORTENED:
	// 		case BT_DATA_NAME_COMPLETE:
	// 			len = MIN(data->data_len, NAME_LEN - 1);
	// 			(void)memcpy(name, data->data, len);
	// 			name[len] = '\0';
	// 			return false;
	// 		default:
	// 			return true;
	// 	}
	// }

	// static const char *phy2str(uint8_t phy) {
	// 	switch (phy) {
	// 	case BT_GAP_LE_PHY_NONE: return "No packets";
	// 	case BT_GAP_LE_PHY_1M: return "LE 1M";
	// 	case BT_GAP_LE_PHY_2M: return "LE 2M";
	// 	case BT_GAP_LE_PHY_CODED: return "LE Coded";
	// 	default: return "Unknown";
	// 	}
	// }

	static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf) {
		
		if (info->rssi < RSSI_THRESHOLD) {
			//printk("Ignoring weak signal from %s (RSSI %d)\n", le_addr, info->rssi);
			return; // Ignore weak signals
		}
		
		char le_addr[BT_ADDR_LE_STR_LEN];
		char name[NAME_LEN];
		uint16_t data_len;
		//uint8_t data_status;

		(void)memset(name, 0, sizeof(name));
		data_len = buf->len;
		//bt_data_parse(buf, data_cb, name);

		bt_addr_le_to_str(info->addr, le_addr, sizeof(le_addr));
		printk("[DEVICE]: %s, RSSI: %d, Data len: %u\n", le_addr, info->rssi, buf->len);

		//data_status = BT_HCI_LE_ADV_EVT_TYPE_DATA_STATUS(info->adv_props);

		
		// printk("[DEVICE]: %s, AD evt type %u, Tx Pwr: %i, RSSI %i "
		// 	"Data status: %u, AD data len: %u Name: %s "
		// 	"C:%u S:%u D:%u SR:%u E:%u Pri PHY: %s, Sec PHY: %s, "
		// 	"Interval: 0x%04x (%u ms), SID: %u\n",
		// 	le_addr, info->adv_type, info->tx_power, info->rssi,
		// 	data_status, data_len, name,
		// 	(info->adv_props & BT_GAP_ADV_PROP_CONNECTABLE) != 0,
		// 	(info->adv_props & BT_GAP_ADV_PROP_SCANNABLE) != 0,
		// 	(info->adv_props & BT_GAP_ADV_PROP_DIRECTED) != 0,
		// 	(info->adv_props & BT_GAP_ADV_PROP_SCAN_RESPONSE) != 0,
		// 	(info->adv_props & BT_GAP_ADV_PROP_EXT_ADV) != 0,
		// 	phy2str(info->primary_phy), phy2str(info->secondary_phy),
		// 	info->interval, info->interval * 5 / 4, info->sid);

		for (uint8_t i = 0; i < sizeof(nodes_of_interest) / sizeof(nodes_of_interest[0]); i++) {
			if (strcmp(le_addr, nodes_of_interest[i]) == 0) {
				bt_data_parse(buf, parse_ad_data, NULL);
				k_sleep(K_MSEC(1000));
			}
		}

	}

	static struct bt_le_scan_cb scan_callbacks = {
		.recv = scan_recv,
	};
#endif /* CONFIG_BT_EXT_ADV */

int observer_start(void) {

	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		//.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
	int err;

	#if defined(CONFIG_BT_EXT_ADV)
		// Only applies for BLE using extended advertising
		bt_le_scan_cb_register(&scan_callbacks);
		printk("Registered scan callbacks\n");
	#endif /* CONFIG_BT_EXT_ADV */

	err = bt_le_scan_start(&scan_param, NULL); // do nothing if legacy ble device found (previously calls device_found)
	if (err) {
		printk("Start scanning failed (err %d)\n", err);
		return err;
	}
	printk("Started scanning...\n");

	return 0;
}