#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>

#include "sensordata.h"
#include "zephyr/sys/printk.h"

LOG_MODULE_REGISTER(basenode_fs);

#define STORAGE_PARTITION_ID FIXED_PARTITION_ID(storage_partition)
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(lfs_data);

#define SENSOR_DATA_FILENAME_BUFLEN 256
#define SENSOR_LOG_BUF_LEN 256

char sensorDataFileName[SENSOR_DATA_FILENAME_BUFLEN];

bool sensorLoggingEnabled = false;

static struct fs_mount_t mountPoint = {
    .type = FS_LITTLEFS,
    .fs_data = &lfs_data,
    .storage_dev = (void *)STORAGE_PARTITION_ID,
    .mnt_point = "/lfs"
};

int mount_filesystem(void) {
    int rc;

    // mount littlefs filesystem
    rc = fs_mount(&mountPoint);

    if (rc < 0) {
        return rc;
    }

    return 0;
}

void file_log_enable(char *filepath) {
    strncpy(sensorDataFileName, filepath, SENSOR_DATA_FILENAME_BUFLEN);
    sensorLoggingEnabled = true;
}

void file_log_disable() {
    sensorLoggingEnabled = false;
}

void file_log_sensor_data(struct SensorData *data) {
    int rc;

    // only log if enabled
    if (!sensorLoggingEnabled) {
        return;
    }

    struct fs_file_t logFile;
    fs_file_t_init(&logFile);

    char buf[SENSOR_LOG_BUF_LEN];

    rc = fs_open(&logFile, sensorDataFileName, FS_O_CREATE | FS_O_APPEND);
    if (rc < 0) {
        LOG_ERR("error: cannot open sensor log file");
        return;
    }

    encode_sensor_data(data, buf, SENSOR_LOG_BUF_LEN); 

    rc = fs_write(&logFile, buf, strlen(buf));
    if (rc < 0) {
        LOG_ERR("error: sensor log file cannot be written");
        return;
    }

    // add a newline at the end
    const char nl = '\n';
    fs_write(&logFile, &nl, 1);

    fs_close(&logFile); 
}
