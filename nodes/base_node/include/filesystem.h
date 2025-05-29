#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "sensordata.h"

int mount_filesystem(void);
void file_log_sensor_data(struct SensorData *data);
void file_log_enable(char *filepath);
void file_log_disable();

#endif // FILESYSTEM_H
