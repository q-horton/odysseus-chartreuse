#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "filesystem.h"
#include "bt_interface.h"
#include "servo.h"

int main() {
    mount_filesystem();

    set_pan_angle(PAN_HOME);
    set_tilt_angle(TILT_DAM_UP);

    bt_run();
}
