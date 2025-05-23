## Data Format
Each Cell is a byte: Timestamp (uint32_t [ms]), Soil Moisture (0-255), Temp (int16_t), Humidity (0-200 -> 0-100% in 0.5% inc.), Encoding (TBD)
Sensor: [timestamp1, timeStamp2, timestamp3, timestamp4, soil_moisture_0xff00, soil_moisture_0x00ff, temp_0xff00, temp_0x00ff, humidity_x2, encoding1, encoding2, encoding3, encoding4]
