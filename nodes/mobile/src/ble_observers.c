
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/logging/log.h>
#include <shared_vars.h>
#include <string.h>

// #define DEVICE_NAME "SensorNode"
// #define SENSOR_ADV_INTERVAL_MS 1000
// #define NAME_LEN 30
#define BASENODE_MAC "DC:E9:F5:83:72:9C (random)" // MAC address of BaseNode
#define MIN_INTERVAL_FOR_UPDATES 10

typedef struct {
	const char *mac_addr;
	bool collected;
} sensor_nodes_t;

sensor_nodes_t sensornodes_status[MAX_SENSOR_NODES] = {
	{"C5:84:32:CA:99:CA (random)", false}
	// Add more MACs here
};

// EXTERN Variables
uint8_t databuffer[1650] = {0}; // Buffer to hold advertisement data
uint16_t databuffer_loc = 0;
uint32_t current_time = 0;
uint8_t flags = 0x10;

static bool parse_config_data_from_basenode_ad(struct bt_data *data, void *user_data) {

    if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) { // 4 bytes for time + 1 byte for flags

        k_sem_take(&access_sensor_config_data, K_FOREVER); 

        const uint8_t *payload = data->data;
		 // Based on the above, extract current_time_ms
        uint32_t newtime = (payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24));
        int32_t time_delta = (int32_t)(newtime - current_time);

        if (time_delta > MIN_INTERVAL_FOR_UPDATES || time_delta < -MIN_INTERVAL_FOR_UPDATES || flags != payload[4]) {
           	printk("[MOBILE-LOG] Updated from BaseNode. Time: %u, Flags: 0x%02X\n", current_time, flags);
            current_time = newtime;
            flags = payload[4];
        }
        
        k_sem_give(&access_sensor_config_data);
    }
    return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
	
    if (rssi < RSSI_THRESHOLD) {
		return; // Ignore weak signals
	}
	
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	
    if (strcmp(addr_str, BASENODE_MAC) == 0) {
        bt_data_parse(ad, parse_config_data_from_basenode_ad, NULL);
        //k_sleep(K_MSEC(1000));

		// Disable advertising after receiving config data
		//bt_le_adv_stop();

		// Potentiaally use a flag to check if already off, so dont repeat operations
		// 
		// Enable extended advertising

		// Send data to BaseNode

		// Disable extended advertising

		// Enable legacy advertising
    }
}

static bool parse_ad_data(struct bt_data *data, void *user_data) {

    if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) {
        const uint8_t *payload = data->data;

		printk("[MOBILE-DEBUG] Raw AD data: ");
		for (int i = 0; i < data->data_len; i++) {
			printk("%02X ", payload[i]);

			if (databuffer_loc < sizeof(databuffer)) {
				databuffer[databuffer_loc++] = payload[i];
			} else {
				printk("\n[MOBILE-ERR] Data buffer overflow!\n");
				return false; 
			}
		}
		printk("\n");
    }

	printk("[MOBILE-LOG] Parsed SensorData\n");
	k_sem_give(&adv_data_ready_sem); // Signal that new data is ready
    return true;
}

static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf) {
	
	if (info->rssi < RSSI_THRESHOLD) {
		return; // Ignore weak signals
	}
	
	char le_addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(info->addr, le_addr, sizeof(le_addr));
	printk("[MOBILE-LOG]: DEVICE %s, RSSI: %d, Data len: %u\n", le_addr, info->rssi, buf->len);

	for (uint8_t i = 0; i < MAX_SENSOR_NODES; i++) {
		// Check if the sensor node is of interest and not yet collected
		if (!sensornodes_status[i].collected && strcmp(le_addr, sensornodes_status[i].mac_addr) == 0) { 
			bt_data_parse(buf, parse_ad_data, NULL);
			sensornodes_status[i].collected = true;  // Mark as collected
			//k_sleep(K_MSEC(1000)); // Optional pause
			break; // Stop loop once matched
		}
	}

}

static struct bt_le_scan_cb scan_callbacks = {
	.recv = scan_recv,
};

int ble_observers_start(void) {

	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		//.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	int err;
	// Only applies for BLE using extended advertising
	bt_le_scan_cb_register(&scan_callbacks);
	printk("[MOBILE-LOG] Registered scan callbacks\n");

	err = bt_le_scan_start(&scan_param, device_found); // do nothing if legacy ble device found (previously calls device_found)
	if (err) {
		printk("[MOBILE-ERR] Start scanning failed (err %d)\n", err);
		return err;
	}
	printk("[MOBILE-LOG] Started scanning...\n");

	return 0;
}