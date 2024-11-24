# HomeClimate - Climate Monitoring System

This project is an IoT-based **Home Climate Monitoring System** that leverages an **ESP32 microcontroller** to collect, analyze, and publish sensor data to a **Mosquitto MQTT broker** running as an add-on in **Home Assistant**. The system integrates multiple environmental sensors and peripherals to provide real-time monitoring and alerts for various environmental parameters. Additionally an **OLED display (SH1106)** is used to visualize sensor readings directly on the device.

---

## **Overview**

The Home Climate Monitoring System gathers data using the following sensors and modules:

- **BME680**: Measures temperature, humidity, pressure, and gas resistance.
- **MQ-2**: Detects levels of LPG, Carbon Monoxide (CO), and smoke.
- **KY-038**: Captures sound levels to detect loud noises.
- **NeoPixels**: Displays status visually using LEDs.
- **Buzzer**: Provides audible alerts for abnormal conditions.
- **SH1106 OLED Display**: Displays real-time sensor readings and status information directly on the device.

The collected data is sent to **Home Assistant** using the **MQTT protocol** for integration into a centralized smart home system. The system also provides real-time alerts through NeoPixels, a buzzer, and on-screen OLED updates when unsafe thresholds are detected.

---

## **Features**

### 1. **Sensors and Modules**

| Component       | Functionality                                                                                                |
| --------------- | ------------------------------------------------------------------------------------------------------------ |
| **BME680**      | - Measures temperature, humidity, pressure, and gas resistance.<br>- Detects air quality and altitude.       |
| **MQ-2**        | - Detects the presence of gases such as LPG, CO, and smoke.<br>- Monitors air pollution and fire risks.      |
| **KY-038**      | - Detects sound levels in decibels (dB).<br>- Monitors noise pollution or sudden loud noises.                |
| **SH1106 OLED** | - Displays sensor readings directly on the device.<br>- Shows animations for user feedback.                  |
| **NeoPixels**   | - Visual status indicator for safe, warning, and danger levels.<br>- Displays animations and status changes. |
| **Buzzer**      | - Audible alerts for warnings and dangers.<br>- Triggers for abnormal conditions.                            |

---

### 2. **MQTT Integration**

- The system uses an **MQTT broker** (Mosquitto) as part of the **Home Assistant** ecosystem to enable real-time monitoring and control.
- **Key Features**:
  - Publishes sensor data to structured MQTT topics.
  - Enables Home Assistant to visualize data and create automations.
  - Triggers alerts and actions based on sensor readings.

#### MQTT Topic Structure

The following hierarchical structure is used for MQTT topics, organized by sensor type:

- **BME680 Sensor Topics**:

  - `home/sensors/bme680/temperature`
  - `home/sensors/bme680/humidity`
  - `home/sensors/bme680/pressure`
  - `home/sensors/bme680/gas`
  - `home/sensors/bme680/altitude`

- **MQ-2 Sensor Topics**:

  - `home/sensors/mq2/lpg`
  - `home/sensors/mq2/co`
  - `home/sensors/mq2/smoke`

- **KY-038 Sensor Topics**:
  - `home/sensors/ky038/sound`

#### Home Assistant Integration

- The system is configured in **Home Assistant** to visualize sensor data and manage automations:
  - Dashboards are designed for real-time monitoring of environmental data.
  - Alerts and notifications are triggered for:
    - Unsafe gas concentrations (e.g., high LPG or CO levels).
    - Excessive sound levels detected by the KY-038 sensor.
- Automations can activate actions such as:
  - Sending notifications to mobile devices.
  - Activating ventilation systems or alarms when thresholds are exceeded.

---

### 3. **Alerts and Notifications**

- **Visual Alerts**:
  - **NeoPixels**:
    - **Blue**: Safe.
    - **Green**: Warning.
    - **Red**: Danger.
- **Audible Alerts**:
  - Buzzer sounds for warning and danger levels.
- **OLED Display Alerts**:
  - Real-time sensor data is displayed.
  - Animations indicate system status and updates.

---

## **Hardware Components**

| **Component**       | **Specification**                               |
| ------------------- | ----------------------------------------------- |
| **Microcontroller** | ESP32-WROOM-32E                                 |
| **Sensors**         | BME680, MQ-2, KY-038                            |
| **Peripherals**     | SH1106 OLED Display, NeoPixels (5 LEDs), Buzzer |
| **Power Supply**    | 5V USB-C                                        |

---

## **Software Requirements**

1. **Firmware**:
   - Developed using **PlatformIO** in **VS Code**.
   - Arduino framework.
2. **MQTT Broker**:

   - **Mosquitto MQTT broker** running as a Home Assistant add-on.

3. **Home Assistant**:
   - Running on a self-hosted server.
   - Used to visualize and manage sensor data.

---
# uop-iot-homeclimate
