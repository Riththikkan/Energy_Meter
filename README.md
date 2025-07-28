
# ⚡ Roomsync 2.0 – IoT-Based Smart Energy Audit and Environmental Monitoring System

An advanced ESP32-based energy monitor that tracks voltage, current, power, energy (kWh), temperature, humidity, and **automatically calculates the electricity bill**. Data is displayed on an LCD and also sent to **Firebase Realtime Database** for remote monitoring.



---

## 🔧 Features

- 🔌 **Real-time Monitoring** of:
  - Voltage (using ZMPT101B)
  - Current (via analog input with sensor like ACS712)
  - Active Power (W)
  - Energy Consumption (kWh)
- 🌡️ Temperature & Humidity readings (DHT11)
- 📟 16x2 I²C LCD UI to display data locally
- ☁️ **Firebase Integration** (ESP32 Firebase Library)
- 💰 **Bill Calculation** based on tiered pricing
- 🔁 EEPROM Storage to persist kWh & bill during reboots
- ⏱ Updates every 5 seconds using non-blocking timer

---

## 📦 Hardware Requirements

| Component        | Quantity | Description                                  |
|------------------|----------|----------------------------------------------|
| ESP32 Dev Board  | 1        | Wi-Fi capable microcontroller                |
| ZMPT101B Module  | 1        | Voltage sensor (analog output)              |
| Current Sensor   | 1        | e.g., ACS712 5A/20A/30A module               |
| DHT11            | 1        | Temperature & humidity sensor               |
| LCD 16x2 (I2C)   | 1        | For local display                            |
| Resistors, wires | —        | General wiring and sensor connections        |
| 5V Power Supply  | 1        | For powering the ESP32 and sensors           |

---

## 🖥️ LCD Display Cycles

- Voltage (Vrms) and Current (Irms)
- Power (W) and Energy Used (kWh)
- Calculated Bill (Rs)
- Temperature (°C) and Humidity (%)

Each screen displays for 3 seconds in loop.

---

## 📲 Firebase Integration

- Data is pushed to Firebase Realtime Database every 5 seconds:
```

/EnergyData/
├── Voltage
├── Current
├── Power
├── KWh
├── Bill
├── temp
└── humidity
/conditions/
├── Voltage
└── KWh

````
- Data resets the temporary bill every **1 minute**

---

## 💸 Electricity Bill Calculation

Tiered pricing structure:
| Units Range (kWh) | Rate (Rs/kWh) |
|-------------------|---------------|
| 0 - 30            | 12.00         |
| 31 - 60           | 30.00         |
| 61 - 90           | 38.00         |
| 91 - 120          | 41.00         |
| 121 - 180         | 59.00         |
| 181+              | 89.00         |

---

## 📁 EEPROM Persistence

Values stored:
- `kWh` — Total energy consumption
- `bill` — Last bill amount

These are saved automatically and loaded on startup.

---

## 🚀 Setup Instructions

1. **Clone the Repo**
 ```bash
 git clone https://github.com/<your-username>/esp32-energy-monitor.git
 cd esp32-energy-monitor
````

2. **Install Libraries**
   In Arduino IDE or PlatformIO:

   * `FirebaseESP32`
   * `LiquidCrystal_I2C`
   * `DHT sensor library`
   * `TickTwo`
   * `Wire`, `WiFi`, `EEPROM` (pre-installed)

3. **Update Credentials**
   In the sketch, replace:

   ```cpp
   #define WIFI_SSID "YourWiFi"
   #define WIFI_PASSWORD "YourPassword"
   config.host = "https://your-firebase-project.firebaseio.com";
   config.api_key = "YOUR_FIREBASE_API_KEY";
   auth.user.email = "your@email.com";
   auth.user.password = "yourpassword";
   ```

4. **Upload the Code**
   Select "ESP32 Dev Module" and upload via USB.

5. **Open Serial Monitor**
   Observe IP and readings.

---

## 🧪 Calibration Notes

* **Voltage Sensor (ZMPT101B)**:

  * Tune sensitivity value for accuracy: `voltageSensor.setSensitivity(490.0);`
* **Current Sensor**:

  * Adjust voltage offset and calculation in `calculateIrms()`

---

## 🧠 Code Architecture Overview

* `setup()`:

  * Initializes Wi-Fi, Firebase, DHT11, LCD, EEPROM
* `loop()`:

  * Uses `TickTwo` timer to update every 5s
* `sendEnergyDataToFirebase()`:

  * Reads sensors, calculates power, updates LCD & Firebase
* `calculateBill()`:

  * Computes current bill using tiered slab logic
* EEPROM:

  * Uses `.put()` and `.get()` to store `kWh` and `bill` values

---

## 📊 Output Example (Serial Monitor)


Vrms: 228.22V    Irms: 0.1025A   Power: 23.389W
kWh: 0.00345kWh  Bill: 0.04Rs
Temperature: 28.50°C   Humidity: 60.30%




## 🤝 Contributing

Pull requests, issue reports, and suggestions are welcome!
Help make energy usage monitoring more accessible 💡⚡

---

## 👨‍💻 Author

**Riththikkan Sairam**
🔗 [LinkedIn](www.linkedin.com/in/riththikkansairam2002) | ✉️ [riththikkansairam@gmail.com](mailto:riththikkansairam@gmail.com)


---


