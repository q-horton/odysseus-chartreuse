# System Integration

### **Data Requirement**

* **Soil Moisture Level (%):** Indicates soil saturation; high levels may signal reduced absorption capacity before rainfall.

* **Relative Humidity (%):** High humidity levels may indicate imminent rainfall or low evaporation rates.

* **Temperature (°C):** Helps contextualise moisture and humidity changes, used for more accurate environmental predictions.

---

### **Sensors Utilised**

1. **Sparkfun Soil Moisture Sensor**

   * Type: Analogue Resistive soil moisture sensor.
   * Measures: Volumetric water content in soil.

2. **Humidity Sensor**

   * Type: Digital humidity and temperature sensor (e.g. AHT22).
   * Measures: Relative humidity (%) and ambient temperature (°C), which helps assess evaporation and weather patterns.

3. **Temperature Sensor**

   * Type: Digital temperature sensor.
   * Measures: Ambient temperature (°C), further helps assess evaporation and weather patterns.

---

### **Integration**

* **Hardware Interface:**

  * **Analog sensors** connected via ADC (Analog-to-Digital Converter) pins on the microcontroller (e.g. Sparkfun Soil Probe).
  * **Digital sensors** use **I2C**, **SPI** protocols, depending on specifics of sensors used.
  * 3.3V / 5V Power lines utilised, regulated standalone supply via Buck Converter or from Microcontroller Output

* **Zephyr OS**
  * Sensors utilise Zephyr’s device tree for GPIO, I2C/SPI interface config. Drivers customised or adapted from Zephyr's sensor subsystem.
  * Sensor data read periodically using timed threads and stored locally before transmission.

* **Data Handling:**
  * Data is timestamped and buffered in local memory.
  * Transmitted via wireless interface (e.g., BLE) to mule nodes when in range (RSSI > Threshold).
