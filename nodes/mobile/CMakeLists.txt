# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(final_mobile)

target_sources(app PRIVATE 
    src/main.c
    src/ble_observers.c
    src/ble_sensordata_ext_ad.c
)

# Include directories
target_include_directories(app PRIVATE 
    ../../../include
    ./src
)


