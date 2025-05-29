#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gap.h>
#include <shared_vars.h>

#define ADV_THREAD_STACK_SIZE 1024
#define ADV_THREAD_PRIORITY 5
#define ADV_PAYLOAD_BUF_SIZE 240 

K_THREAD_STACK_DEFINE(adv_thread_stack, ADV_THREAD_STACK_SIZE);
static struct k_thread adv_thread_data;

// Semaphore declared in shared_vars.h or another file
//extern struct k_sem adv_data_ready_sem;


K_SEM_DEFINE(adv_data_ready_sem, 0, 1);

// #define SENSOR_ADV_INTERVAL_MS 1000
// #define NAME_LEN 30
// #define RSSI_THRESHOLD -50 // RSSI threshold for filtering devices
// #define MOBILE_NODE_MAC_ADDR "DC:B1:BE:10:38:1E (random)" // MAC address of Mobile Node
// #define MIN_INTERVAL_FOR_UPDATES 10 // Minimum interval in seconds for updates

// // --- Global Variables ---
// K_SEM_DEFINE(config_data, 1, 1); // Semaphore for synchronization

// uint32_t current_time = 0; // Current time in s
// uint32_t previous_time = 0; 
// uint64_t current_time_ms = 0;
// uint8_t config_set_externally = 0;
// uint8_t polling_flag = 1; // Determines poll rate, ignored for now 

// Custom sensor data payload, length + 5 bytes < 255
__aligned(4) static uint8_t adv_payload[ADV_PAYLOAD_BUF_SIZE] = {0};  
// To use max of 1650, define multuple adv_payloads, and add in ext_ad[] below (TBD)
__aligned(4) static uint8_t adv_payload_two[ADV_PAYLOAD_BUF_SIZE] = {0}; // For future use
__aligned(4) static uint8_t adv_payload_three[ADV_PAYLOAD_BUF_SIZE] = {0};
// static uint8_t adv_payload_4[240]; // For future use

static struct bt_data ext_ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload, sizeof(adv_payload)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_two, sizeof(adv_payload_two)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_three, sizeof(adv_payload_three)),
    //BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_3, sizeof(adv_payload_3)),
    // BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_4, sizeof(adv_payload_4)),
};

static struct bt_le_ext_adv *adv_set;
// static struct k_timer second_timer;

// void timer_handler(struct k_timer *dummy) {
//     current_time++;
// }

void update_adv_payload(void) {

    // Put first current_time into first 4 bytes of adv_payload
    //k_sem_take(&config_data, K_FOREVER);

    // Remaining bytes can remain zero
    for (int i = 0; i < sizeof(adv_payload); i++) {
        adv_payload[i] = databuffer[i];
    }
    for (int i = 0; i < sizeof(adv_payload_two); i++) {
        adv_payload_two[i] = databuffer[i + ADV_PAYLOAD_BUF_SIZE];; // Example data
    }
    for (int i = 0; i < sizeof(adv_payload_three); i++) {
        adv_payload_three[i] = databuffer[i + (2 * ADV_PAYLOAD_BUF_SIZE)];; // Example data
    }
    // REPEAT 2 x
    
    printk("[SENSORNODE-LOG] Added data to Payload: ");
}

// static bool parse_ad_data(struct bt_data *data, void *user_data) {

//     if (data->type == BT_DATA_MANUFACTURER_DATA && data->data_len >= 5) {
//         k_sem_take(&config_data, K_FOREVER); 
//         const uint8_t *payload = data->data;

//         // Based on the above, extract current_time_ms
//         uint32_t newtime = (payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24));
//         int32_t time_delta = (int32_t)(newtime - current_time);

//         if (time_delta > MIN_INTERVAL_FOR_UPDATES || time_delta < -MIN_INTERVAL_FOR_UPDATES || polling_flag != payload[4]) {
//             printk("[SENSORNODE-LOG] Received new time: %u, Polling flag: %u\n", newtime, payload[4]);
//             current_time = newtime;
//             polling_flag = payload[4];
//             config_set_externally = 1;
//         }
//         k_sem_give(&config_data);
//     }
//     return true;
// }

// static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ext_ad) {
	
//     if (rssi < RSSI_THRESHOLD) {
// 		return; // Ignore weak signals
// 	}
// 	char addr_str[BT_ADDR_LE_STR_LEN];
// 	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

// 	// Parse the advertisement data if the device is of interest
//     if (strcmp(addr_str, MOBILE_NODE_MAC_ADDR) == 0) {
//         bt_data_parse(ext_ad, parse_ad_data, NULL);
//         k_sleep(K_MSEC(1000));
//         //return;
//     }
// }

void adv_update_thread(void *a, void *b, void *c) {
    int err;

    while (1) {

        k_sem_take(&adv_data_ready_sem, K_FOREVER);

        printk("[SENSORNODE-THREAD] New data ready, updating advertising...\n");

        // Stop advertising before changing payload
        bt_le_ext_adv_stop(adv_set);
        k_sleep(K_MSEC(50)); // Short delay

        update_adv_payload();

        err = bt_le_ext_adv_set_data(adv_set, ext_ad, ARRAY_SIZE(ext_ad), NULL, 0);
        if (err) {
            printk("Failed to update extended adv data (err %d)\n", err);
            continue;
        }

        err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
        if (err) {
            printk("Failed to restart extended advertising (err %d)\n", err);
            continue;
        }

        printk("[SENSORNODE-THREAD] Advertising updated\n");
    }
}


int ble_ext_adv_sensordata_start(void) {
    int err;

    // k_timer_init(&second_timer, timer_handler, NULL);
    // k_timer_start(&second_timer, K_SECONDS(1), K_SECONDS(1));

    // For Extended Advertising
    struct bt_le_adv_param adv_params = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);
    
    // // For Observing
    // struct bt_le_scan_param scan_param = {
	// 	.type       = BT_LE_SCAN_TYPE_PASSIVE,
	// 	.interval   = BT_GAP_SCAN_FAST_INTERVAL,
	// 	.window     = BT_GAP_SCAN_FAST_WINDOW,
	// };

    printk("[SENSORNODE-LOG] Starting SensorNode BLE extended advertiser\n");

    // err = bt_enable(NULL);
    // if (err) {
    //     printk("Bluetooth init failed (err %d)\n", err);
    //     return -1;
    // }

    printk("[SENSORNODE-LOG] Bluetooth initialized\n");

    update_adv_payload();

    err = bt_le_ext_adv_create(&adv_params, NULL, &adv_set);
    if (err) {
        printk("Failed to create extended advertiser (err %d)\n", err);
        return -1;
    }

    err = bt_le_ext_adv_set_data(adv_set, ext_ad, ARRAY_SIZE(ext_ad), NULL, 0);
    if (err) {
        printk("Failed to set extended adv data (err %d)\n", err);
        return -1;
    }

    err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
    if (err) {
        printk("Failed to start extended advertising (err %d)\n", err);
        return -1;
    }

    printk("[SENSORNODE-LOG] Extended advertising started\n");

    // create a thread to update the advertisement data periodically

    k_thread_create(&adv_thread_data, adv_thread_stack,
        K_THREAD_STACK_SIZEOF(adv_thread_stack),
        adv_update_thread,
        NULL, NULL, NULL,
        ADV_THREAD_PRIORITY, 0, K_NO_WAIT);

    return 0;
    // while (1) {

    //     // turn into thread with sem so waits for databuff to be filled

    //     printk("[SENSORNODE-LOG] Updating advertisement data...\n");
    //     bt_le_ext_adv_stop(adv_set);
    //     k_sleep(K_MSEC(100)); // Short delay for service to stop
    //     update_adv_payload();
   
    //     err = bt_le_ext_adv_set_data(adv_set, ext_ad, ARRAY_SIZE(ext_ad), NULL, 0);
    //     if (err) {
    //         printk("Failed to update extended adv data (err %d)\n", err);
    //         return -1;
    //     }

    //     err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
    //     if (err) {
    //         printk("Failed to start extended advertising (err %d)\n", err);
    //         return -1;
    //     }

    //     k_sleep(K_MSEC(SENSOR_ADV_INTERVAL_MS - 100));
    // }
}
