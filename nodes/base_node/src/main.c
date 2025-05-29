#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "filesystem.h"
#include "bt_interface.h"

int main() {
    mount_filesystem();

    bt_run();
}
