// shared_vars.h
#ifndef SHARED_VARS_H
#define SHARED_VARS_H

#define NAME_LEN 30
#define RSSI_THRESHOLD -50 // RSSI threshold for filtering devices
#define MAX_SENSOR_NODES 5 // Maximum number of SensorNodes to track

// From main.c, ble_observer_ext_ad.c
extern uint8_t databuffer[1650];
extern uint16_t databuffer_loc;

// From main.c, ble_observer_normal_ad.c
extern uint32_t current_time; // Current time in seconds
extern uint8_t flags; // Polling Rate (hrs) / Disable Sensors
extern struct k_sem access_sensor_config_data; // Semaphore for time, flags


#endif
