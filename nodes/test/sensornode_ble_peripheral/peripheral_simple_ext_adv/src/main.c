#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DEVICE_NAME "SensorNode"
#define SENSOR_ADV_INTERVAL_MS 1000

// Custom sensor data payload, length + 5 bytes < 255
__aligned(4) static uint8_t adv_payload[230] = {0};  
// To use max of 1650, define multuple adv_payloads, and add in ad[] below (TBD)
__aligned(4) static uint8_t adv_payload_two[230] = {0}; // For future use
__aligned(4) static uint8_t adv_payload_3[230] = {0}; // For future use
//static uint8_t adv_payload_4[240]; // For future use
//static uint8_t adv_payload_5[240]; // For future use
//static uint8_t adv_payload_6[240]; // For future use

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload, sizeof(adv_payload)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_two, sizeof(adv_payload_two)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_3, sizeof(adv_payload_3)),
    // BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_4, sizeof(adv_payload_4)),
    // BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_5, sizeof(adv_payload_5)),
    // BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_6, sizeof(adv_payload_6)),
};

static struct bt_le_ext_adv *adv_set;

void update_adv_payload(void) {
    static uint8_t counter = 0;

    int16_t temperature = 250 + counter; // Simulated temperature (25.0 + count)
    uint16_t battery_mv = 3700;          // Simulated battery voltage (mV)

    adv_payload[0] = counter++;
    adv_payload[1] = temperature & 0xFF;
    adv_payload[2] = (temperature >> 8) & 0xFF;
    adv_payload[3] = battery_mv & 0xFF;
    adv_payload[4] = (battery_mv >> 8) & 0xFF;

    // Remaining bytes can remain zero
    for (int i = 5; i < sizeof(adv_payload); i++) {
        adv_payload[i] = 0xFF;
    }

    // // fill adv_payload_two with some data for future use
    for (int i = 0; i < sizeof(adv_payload_two); i++) {
        adv_payload_two[i] = counter; // Example data
    }

    // print the payload for debugging
    printk("Updated advertisement payload: ");
    for (int i = 0; i < sizeof(adv_payload); i++) {
        printk("%02X ", adv_payload[i]);
    }
    for (int i = 0; i < sizeof(adv_payload_two); i++) {
        printk("%02X ", adv_payload_two[i]);
    }
    printk("\n");
}

void main(void) {
    int err;
    // struct bt_le_adv_param adv_params = {
    //     .options = BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY,
    //     .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
    //     .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
    //     .id = BT_ID_DEFAULT,
    //     .sid = 0,
    // };

    struct bt_le_adv_param adv_params = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);

    printk("Starting SensorNode BLE extended advertiser\n");

    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    update_adv_payload();

    err = bt_le_ext_adv_create(&adv_params, NULL, &adv_set);
    if (err) {
        printk("Failed to create extended advertiser (err %d)\n", err);
        return;
    }

    err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Failed to set extended adv data (err %d)\n", err);
        return;
    }

    err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
    if (err) {
        printk("Failed to start extended advertising (err %d)\n", err);
        return;
    }

    printk("Extended advertising started\n");

    while (1) {
        printk("Updating advertisement data...\n");
        update_adv_payload();
        bt_le_ext_adv_stop(adv_set);
        //LOG_HEXDUMP_INF(adv_payload, 10, "First 10 Bytes of Payload");
        err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
        if (err) {
            printk("Failed to update extended adv data (err %d)\n", err);
            return;
        }
        err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
        if (err) {
            printk("Failed to start extended advertising (err %d)\n", err);
            return;
        }

        k_sleep(K_MSEC(SENSOR_ADV_INTERVAL_MS));
    }
}
