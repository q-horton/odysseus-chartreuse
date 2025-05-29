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

// TEST
#define SENSOR_DATA_START 12
#define SENSOR_DATA_SIZE 12
#define SENSOR_PACKETS_PER_PAYLOAD 19
#define TIMESTAMP_OFFSET 0
#define MEAS_ID_OFFSET 4
#define MOISTURE_OFFSET 6
#define TEMP_OFFSET 8
#define PRESSURE_OFFSET 10
#define PRESSURE_BASE 100000

static struct k_thread adv_thread_data;

K_THREAD_STACK_DEFINE(adv_thread_stack, ADV_THREAD_STACK_SIZE);
K_SEM_DEFINE(adv_data_ready_sem, 0, 1);
K_SEM_DEFINE(wait_for_ext_adv_update_sem, 1, 1);

// Prototypes
void print_converted_data(uint8_t*);
void update_adv_payload(void);

// Custom sensor data payload, length + 5 bytes < 255
__aligned(4) static uint8_t adv_payload[ADV_PAYLOAD_BUF_SIZE] = {0};  
__aligned(4) static uint8_t adv_payload_two[ADV_PAYLOAD_BUF_SIZE] = {0}; // For future use
__aligned(4) static uint8_t adv_payload_three[ADV_PAYLOAD_BUF_SIZE] = {0};

static struct bt_data ext_ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload, sizeof(adv_payload)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_two, sizeof(adv_payload_two)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload_three, sizeof(adv_payload_three)),
};

static struct bt_le_ext_adv *adv_set;
uint16_t greatestReceivedMeasId = 0;

void update_adv_payload(void) {

    // Put first current_time into first 4 bytes of adv_payload
    // k_sem_take(&adv_data_ready_sem, K_FOREVER);

    // printk("[MOBILENODE-LOG] RAW DATABUFF...\n");
    // for (int i = 0; i < 1650; i++) {
    //     printk("%02X ", databuffer[i]); // Clear the buffer
    // }
    // printk("\n\n");
    
    // Remaining bytes can remain zero
    for (int i = 0; i < ADV_PAYLOAD_BUF_SIZE; i++) {
        adv_payload[i] = databuffer[i];
    }
    for (int i = 0; i < ADV_PAYLOAD_BUF_SIZE; i++) {
        adv_payload_two[i] = databuffer[i + ADV_PAYLOAD_BUF_SIZE];; // Example data
    }
    for (int i = 0; i < ADV_PAYLOAD_BUF_SIZE; i++) {
        adv_payload_three[i] = databuffer[i + (2 * ADV_PAYLOAD_BUF_SIZE)];; // Example data
    }
    // REPEAT 2 x

    printk("[MOBILENODE-LOG] EXT AD BUF POST UPDATE...\n");
    // print out adv_payload 1, 2, 3
    for (int i = 0; i < ADV_PAYLOAD_BUF_SIZE; i++) {
        printk("%02X ", adv_payload[i]);
    }
    for (int i = 0; i < ADV_PAYLOAD_BUF_SIZE; i++) {
        printk("%02X ", adv_payload_two[i]);
    }
    for (int i = 0; i < ADV_PAYLOAD_BUF_SIZE; i++) {
        printk("%02X ", adv_payload_three[i]);
    }
    printk("\n\n");

    print_converted_data(databuffer);
    
    printk("[MOBILENODE-LOG] Added data to Payload: \n");
}

void print_converted_data(uint8_t *payload) {
    // Print the converted data from databuffer
    for (int i = 0; i < SENSOR_PACKETS_PER_PAYLOAD; i++) {
        int pos = SENSOR_DATA_START + i * SENSOR_DATA_SIZE;

        uint32_t timestamp = *((uint32_t *) (void *) &payload[pos]);
        uint16_t measId = *((uint16_t *) (void *) &payload[pos + MEAS_ID_OFFSET]);
        uint16_t moisture = *((uint16_t *) (void *) &payload[pos + MOISTURE_OFFSET]);
        uint16_t temp = *((uint16_t *) (void *) &payload[pos + TEMP_OFFSET]);
        uint32_t pressure = *((int16_t *) (void *) &payload[pos + PRESSURE_OFFSET]) + PRESSURE_BASE;

        printk("[MOBILENODE-LOG] Sensor Data %d: Timestamp: %u, MeasId: %u, Moisture: %u, Temp: %u, Pressure: %u\n",
            i, timestamp, measId, moisture, temp, pressure);
    }
}

void adv_update_thread(void *a, void *b, void *c) {
    
    int err;

    while (1) {
        k_sem_take(&adv_data_ready_sem, K_FOREVER);
        printk("[MOBILENODE-THREAD] New data ready, updating advertising...\n");
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

        printk("[MOBILENODE-THREAD] Advertising updated\n");

        //k_sem_give(&wait_for_ext_adv_update_sem); // Signal that update is done
    }
}


int ble_ext_adv_sensordata_start(void) {
    int err;
    // For Extended Advertising
    struct bt_le_adv_param adv_params = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);
    
    printk("[SENSORNODE-LOG] Starting SensorNode BLE extended advertiser\n");

    //update_adv_payload();

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
}
