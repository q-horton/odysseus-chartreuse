/*
* TODO: 
* - Add Handling for base node (change advertisement data to extended when close, fetch time update)
* - Merge with encoding from sensornode
*/

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include "shared_vars.h"

#define SENSOR_ADV_INTERVAL_MS 1000
#define DEVICE_NAME "MobileNode"

K_SEM_DEFINE(access_sensor_config_data, 1, 1); // Semaphore for synchronization

static uint8_t mobile_adv_payload_general[5] = {0};  // Room for mobile node data
static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, mobile_adv_payload_general, sizeof(mobile_adv_payload_general)),
};

// Function Prototypes
int sensornode_observer_start(void);
int basenode_observer_start(void);

// Global Variables
static struct k_timer second_timer;

// static uint32_t current_time = 0;
// uint8_t flags = 0x10; // Polling Rate (hrs) / Disable Sensors

void timer_handler(struct k_timer *dummy) {
    
	k_sem_take(&access_sensor_config_data, K_FOREVER);
	current_time += 1;
	// Wraparound once max value reached
	if (current_time >= 0xFFFFFFFF) {
        current_time = 1;  // Reset to 1 to avoid zero timestamp
    }
	k_sem_give(&access_sensor_config_data); 
}

static void update_mobile_adv_payload_general(void) {
	
	k_sem_take(&access_sensor_config_data, K_FOREVER); // Wait for access to the timer and flags
	mobile_adv_payload_general[0] = (current_time >> 0) & 0xFF;   // Byte 0 (LSB)
    mobile_adv_payload_general[1] = (current_time >> 8) & 0xFF;   
    mobile_adv_payload_general[2] = (current_time >> 16) & 0xFF;  
    mobile_adv_payload_general[3] = (current_time >> 24) & 0xFF;
	mobile_adv_payload_general[4] = flags;  
	k_sem_give(&access_sensor_config_data); // Allow access to the timer and flags
	// Print updated count value and flags
	printk("[MOBILE-LOG] Adv Payload Updated: Time: %u, Flags: 0x%02X\n", current_time, flags);
}

int main(void) {
	
	int err;
	printk("[MOBILE-LOG] Starting MobileNode...\n");

	k_timer_init(&second_timer, timer_handler, NULL);
    k_timer_start(&second_timer, K_SECONDS(1), K_SECONDS(1));

	// Initialise the Bluetooth Subsystem 
	err = bt_enable(NULL);
	if (err) {
		printk("[MOBILE-ERR] Bluetooth init failed (err %d).\n", err);
		return -1;
	}

	// Start Extended Advertising Observer
	(void) sensornode_observer_start();

	// Start Legacy Advertising Observer
	basenode_observer_start();

	// Begin advertising Timestamp and flags
	err = bt_le_adv_start(
		BT_LE_ADV_PARAM(
			BT_LE_ADV_OPT_USE_IDENTITY,
			BT_GAP_ADV_FAST_INT_MIN_2,
			BT_GAP_ADV_FAST_INT_MAX_2,
			NULL),
		ad, ARRAY_SIZE(ad), NULL, 0);

    if (err) {
        printk("[MOBILE-ERR] Advertising failed to start (err %d).\n", err);
        return -1;
    }

	while (1) {
        update_mobile_adv_payload_general();
        bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
        k_sleep(K_MSEC(SENSOR_ADV_INTERVAL_MS));
    }

	printk("Exiting %s thread.\n", __func__);

	return 0;
}