#include "bt_handler.h"
#include "zephyr/bluetooth/gap.h"

LOG_MODULE_REGISTER(BluetoothHandler);

K_SEM_DEFINE(config_data, 1, 1); // Semaphore for synchronization
K_MSGQ_DEFINE(sample_stream, sizeof(SensorLoad), 10, 1);

uint8_t polling_flag = 1; // Determines poll rate, ignored for now 

// Custom sensor data payload, length + 5 bytes < 255
__aligned(4) static uint8_t adv_payload[PAYLOAD_SIZE] = {0};  
// To use max of 1650, define multuple adv_payloads, and add in ad[] below (TBD)
__aligned(4) static uint8_t adv_payload_two[PAYLOAD_SIZE] = {0}; // For future use
__aligned(4) static uint8_t adv_payload_three[PAYLOAD_SIZE] = {0}; // For future use
// static uint8_t adv_payload_4[240]; // For future use

uint8_t* payloads[NUM_PAYLOADS] = {adv_payload, adv_payload_two, adv_payload_three};

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload, sizeof(adv_payload)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_two, sizeof(adv_payload_two)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_three, sizeof(adv_payload_three)),
    // BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_4, sizeof(adv_payload_4)),
};

static struct bt_le_ext_adv *adv_set;

void init_adv_payload(void) {
	k_sem_take(&config_data, K_FOREVER);
	// Remaining bytes can remain zero
	for(int j = 0; j < NUM_PAYLOADS; j++) {
		for(int i = 0; i < PAYLOAD_SIZE; i++) {
			if(j == 0 && i == 0) payloads[j][i] = SENSOR_NODE_ID;
			else payloads[j][i] = 0;
		}
	}
	k_sem_give(&config_data);
}

void update_adv_data(SensorLoad sample, uint16_t index) {
	index++;
	uint8_t packet = (index * sizeof(sample)) / PAYLOAD_SIZE;
	uint8_t packet_index = (index * sizeof(sample)) % PAYLOAD_SIZE;
	LOG_INF("Packet: %d; Packet index: %d\n", packet, packet_index);
	uint8_t* byte_stream = (void*)&sample;
    k_sem_take(&config_data, K_FOREVER);
	for(uint8_t i = 0; i < sizeof(sample); i++) {
		payloads[packet][packet_index + i] = byte_stream[i];
	}
    k_sem_give(&config_data);
}

static bool parse_ad_data(struct bt_data *data, void *user_data) {

    if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) {
        k_sem_take(&config_data, K_FOREVER); 
        const uint8_t *payload = data->data;

        // Based on the above, extract current_time_ms
        uint32_t newtime = (payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24));
        int32_t time_delta = (int32_t)(newtime - get_sys_time());

        if (time_delta > MIN_INTERVAL_FOR_UPDATES || time_delta < -MIN_INTERVAL_FOR_UPDATES || get_polling_rate() != payload[4]) {
            LOG_INF("[SENSORNODE-LOG] Received new time: %u, Polling flag: %u\n", newtime, payload[4]);
            update_sys_time(newtime);
            update_polling_rate(payload[4]);
        }
        k_sem_give(&config_data);
    }
    return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
	
    if (rssi < RSSI_THRESHOLD || type == BT_GAP_ADV_TYPE_EXT_ADV) {
		return; // Ignore weak signals
	}
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

	// Parse the advertisement data if the device is of interest
    if (strcmp(addr_str, MOBILE_NODE_MAC_ADDR) == 0) {
        bt_data_parse(ad, parse_ad_data, NULL);
        k_sleep(K_MSEC(1000));
        //return;
    }
}

void bt_handler_t(void) {
	int err;

	// For Extended Advertising
    struct bt_le_adv_param adv_params = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);
    
    // For Observing
    struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

    LOG_INF("[SENSORNODE-LOG] Starting SensorNode BLE extended advertiser\n");

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return;
    }

    LOG_INF("[SENSORNODE-LOG] Bluetooth initialized\n");

    init_adv_payload();

    err = bt_le_ext_adv_create(&adv_params, NULL, &adv_set);
    if (err) {
        LOG_ERR("Failed to create extended advertiser (err %d)\n", err);
        return;
    }

    err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Failed to set extended adv data (err %d)\n", err);
        return;
    }

    err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
    if (err) {
        LOG_ERR("Failed to start extended advertising (err %d)\n", err);
        return;
    }

    LOG_INF("[SENSORNODE-LOG] Extended advertising started\n");

    err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		LOG_ERR("Start scanning failed (err %d)\n", err);
		return;
	}
	LOG_INF("[SENSORNODE-LOG] Started scanning...\n");

	int index = 0;

	while(1) {
		LOG_INF("[SENSORNODE-LOG] Updating advertisement data...\n");
        bt_le_ext_adv_stop(adv_set);
        k_msleep(100); // Short delay for service to stop
		
		for (int i = 0; i < k_msgq_num_used_get(&sample_stream); i++) {
			SensorLoad sample;
			k_msgq_get(&sample_stream, &sample, K_FOREVER);
			LOG_INF("Index: %d", index);
			update_adv_data(sample, index);
			index = (index + 1) % (PAYLOAD_SIZE * NUM_PAYLOADS / sizeof(sample) - 1);
		}
   
        err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
        if (err) {
            LOG_ERR("Failed to update extended adv data (err %d)\n", err);
            return;
        }

        err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
        if (err) {
            LOG_ERR("Failed to start extended advertising (err %d)\n", err);
            return;
        }

        k_msleep(SENSOR_ADV_INTERVAL_MS - 100);
	}
}

K_THREAD_DEFINE(bt_handler_id, STACK_SIZE, bt_handler_t, NULL, NULL, NULL, PRIORITY, 0, 0);
