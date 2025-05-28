# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensornode_dev)

# FILE(GLOB app_sources src/*.c src/sensornode_observer.c)
FILE(GLOB app_sources src/main.c)
target_sources(app PRIVATE
  ${app_sources}
)

zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth)
