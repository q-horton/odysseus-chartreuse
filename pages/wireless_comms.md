# Wireless Network Communications

The system consists of a base node, mobile node, and several sensor nodes.

- The mobile node will take the BLE Central role, connecting to one of the 
sensor nodes or to the base node. The connection made will be determined 
based on received signal strength (roughly correlated to proximity), choosing
the highest signal strength above a baseline requirement.

- Each sensor node will transmit their collected data to the mobile node via 
BLE when connected.

- When the mobile node connects to the base node, it will offload the collected
data, again via BLE.

- The mobile node will transmit processed data to the PC over WiFi, using the 
MQTT protocol. This will be done via an MQTT broker hosted on UQ Cloud.

- The PC will transmit data to the web dashboard using HTTP (presumably over
either WiFi or ethernet).

The diagram below illustrates this network:

<img src="/images/wireless.png" width="600"/>
