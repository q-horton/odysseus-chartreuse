#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#define DEVICE_NAME "SensorNode"
#define SENSOR_ADV_INTERVAL_MS 1000

static uint8_t adv_payload[10];  // Room for custom sensor data
static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload, sizeof(adv_payload)),
};

void update_adv_payload(void) {
    static uint8_t counter = 0;

    int16_t temperature = 250 + counter; // Simulated temperature (25.0 + count)
    uint16_t battery_mv = 3700;          // Simulated battery voltage (mV)

    // Encode data in little-endian:
    adv_payload[0] = counter++;
    adv_payload[1] = temperature & 0xFF;
    adv_payload[2] = (temperature >> 8) & 0xFF;
    adv_payload[3] = battery_mv & 0xFF;
    adv_payload[4] = (battery_mv >> 8) & 0xFF;

    // Remaining bytes can be zeros or other sensors.
}

void main(void) {
    int err;

    printk("Starting SensorNode BLE advertiser\n");

    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialised\n");

    update_adv_payload();

    err = bt_le_adv_start(BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY,
                                          BT_GAP_ADV_FAST_INT_MIN_2,
                                          BT_GAP_ADV_FAST_INT_MAX_2,
                                          NULL),
                          ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising started - Peripheral Simple\n");

    while (1) {
        update_adv_payload();
        bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
        k_sleep(K_MSEC(SENSOR_ADV_INTERVAL_MS));
    }
}
