#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "servo.h"

const struct device *spi_slave_dev = DEVICE_DT_GET(DT_NODELABEL(slave_spi));

int main() {

    set_pan_angle(90);

    while (1) {
        k_msleep(1000);

        set_tilt_angle(0);

        k_msleep(1000);
        set_tilt_angle(45);

        k_msleep(1000);
        set_tilt_angle(90);

        k_msleep(1000);
        set_tilt_angle(135);

        k_msleep(1000);
        set_tilt_angle(180);
    }
}
