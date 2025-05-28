#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/logging/log.h>
#include <shared_vars.h>

// #define DEVICE_NAME "SensorNode"
// #define SENSOR_ADV_INTERVAL_MS 1000
// #define NAME_LEN 30
#define RSSI_THRESHOLD -50 // RSSI threshold for filtering devices
// #define NODE_OF_INTEREST "DC:B1:BE:10:38:1E (random)" // MAC address of Mobile Node

// uint32_t current_time = 0; // Current time in s
// uint32_t previous_time = 0; 
// uint64_t current_time_ms = 0;
// uint8_t config_set_externally = 0;
// uint8_t polling_flag = 1; // Determines poll rate, ignored for now 


static bool parse_ad_data(struct bt_data *data, void *user_data) {

    if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) { // 4 bytes for time + 1 byte for flags

        k_sem_take(&config_data, K_FOREVER); 

        const uint8_t *payload = data->data;
        // Based on the above, extract current_time_ms
        current_time = (payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24));
        // Extract flags from uint8_t
        polling_flag = payload[4];

        config_set_externally = 1;

        k_sem_give(&config_data);
    }
    return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
	
    if (rssi < RSSI_THRESHOLD) {
		return; // Ignore weak signals
	}
	
	char addr_str[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	// printk("Device found: %s (RSSI %d), type %u, AD data len %u\n",
	//        addr_str, rssi, type, ad->len);

	// Parse the advertisement data if the device is of interest
    if (strcmp(addr_str, NODE_OF_INTEREST) == 0) {
        //printk("Found target device: %s\n", addr_str);
        bt_data_parse(ad, parse_ad_data, NULL);
        k_sleep(K_MSEC(1000));
        //return;
    }
}

int main(void) {
    int err;
   
    // For Normal Observing
    struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

    err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Start scanning failed (err %d)\n", err);
		return err;
	}
	printk("Started scanning...\n");
}
