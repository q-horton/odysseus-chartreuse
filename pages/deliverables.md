# Deliverables

The success of the project will be assessed using the following Delieverables and KPIs:

1. Sensor Node Firmware
- Deliverable: Firmware to read soil moisture and humidity, store data, and transmit to mule nodes using Zephyr.
- KPI: Percentage (%) data transmitted to mule node successfully from sensor node - >95%.

2. Mule Node Networking
- Deliverable: Mobile node software to collect data from sensors and forward to the base node.
- KPI: Percentage ($) collected sensor data forwarded to base node without loss or corruption - >95%.

3. Base Node & Dam Controller Logic
- Deliverable: Central node receives data, stores it, and controls dam gates based on environmental conditions.
- KPI: Percentage (%) error between Dam gate control action and the manually calculated expected outputs / training data output - within 20%.

4. Data Logging & Monitoring Interface
- Deliverable: Interface to log and display sensor data, gate actions, and node diagnostics.
- KPI: Percentage incoming data and system actions loss / corruption in logging and display - >95%. 

5. Fault Tolerance & Data Integrity
- Deliverable: Add error checking, retries, and redundancy to ensure reliable data transfer and recovery.
- KPI: System maintains >90% data integrity under simulated packet loss or node failure.
