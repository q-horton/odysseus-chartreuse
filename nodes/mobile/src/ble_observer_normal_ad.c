// #include <zephyr/kernel.h>
// #include <zephyr/sys/printk.h>
// #include <zephyr/bluetooth/bluetooth.h>
// #include <zephyr/bluetooth/hci.h>
// #include <zephyr/bluetooth/uuid.h>
// #include <zephyr/bluetooth/gap.h>
// #include <zephyr/logging/log.h>
// #include <shared_vars.h>

// // #define DEVICE_NAME "SensorNode"
// // #define SENSOR_ADV_INTERVAL_MS 1000
// // #define NAME_LEN 30
// #define BASENODE_MAC "C5:84:32:CA:99:CA (random)" // MAC address of BaseNode

// // uint32_t current_time = 0; // Current time in s
// // uint32_t previous_time = 0; 
// // uint64_t current_time_ms = 0;
// // uint8_t config_set_externally = 0;
// // uint8_t polling_flag = 1; // Determines poll rate, ignored for now 
// uint32_t current_time = 0;
// uint8_t flags = 0x10;

// static bool parse_ad_data(struct bt_data *data, void *user_data) {

//     if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) { // 4 bytes for time + 1 byte for flags

//         k_sem_take(&access_sensor_config_data, K_FOREVER); 

//         const uint8_t *payload = data->data;
//         // Based on the above, extract current_time_ms
//         current_time = (payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24));
//         // Extract flags from uint8_t
//         flags = payload[4];
        
//         k_sem_give(&access_sensor_config_data);
//     }
//     return true;
// }

// static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
	
//     printk("[MOBILE-LOG] Device found: %s, RSSI: %d, Type: %d\n", bt_addr_le_str(addr), rssi, type);
//     if (rssi < RSSI_THRESHOLD) {
// 		return; // Ignore weak signals
// 	}
	
// 	char addr_str[BT_ADDR_LE_STR_LEN];
// 	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	
//     if (strcmp(addr_str, BASENODE_MAC) == 0) {
//         bt_data_parse(ad, parse_ad_data, NULL);
//         printk("[MOBILE-LOG] Updated from BaseNode. Time: %u, Flags: 0x%02X\n", current_time, flags);
//         //k_sleep(K_MSEC(1000));
//     }
// }

// int basenode_observer_start(void) {
//     int err;
   
//     // For Normal Observing
//     struct bt_le_scan_param scan_param = {
// 		.type       = BT_LE_SCAN_TYPE_PASSIVE,
// 		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
// 		.window     = BT_GAP_SCAN_FAST_WINDOW,
// 	};

//     err = bt_le_scan_start(&scan_param, device_found);
// 	if (err) {
// 		printk("Start scanning failed (err %d)\n", err);
// 		return err;
// 	}
// 	printk("Started scanning...\n");
//     return 0;
// }
