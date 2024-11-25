#include "sensor_processing.h"
#include "hardware_init.h"
#include "helper_functions.h"
#include "oled_display.h"
#include "serial_monitor.h"

/*
 * ==================================================
 * FUNCTION: PROCESS SOUND SENSOR
 * ==================================================
 * Description:
 *   Reads data from the KY-038 Sound Sensor and converts it to decibels.
 *   Displays the sound level on the OLED and prints it to the Serial Monitor.
 *   Issues a status (e.g., LOUD or Normal) based on a defined threshold.
 */

void processSoundSensor()
{
    int rawSound = analogRead(KY038_PIN);
    sound = convertRawSoundToDecibels(rawSound);

    // Display on OLED
    displaySoundSensorReading(sound);

    // Print to Serial Monitor
    Serial.printf("Sound Level: %.1f dB\n", sound);
}

/*
 * ==================================================
 * FUNCTION: PROCESS BME680 SENSOR
 * ==================================================
 * Description:
 *   Reads environmental data from the BME680 sensor, including:
 *   - Temperature (°C)
 *   - Humidity (%)
 *   - Pressure (hPa)
 *   - Gas resistance (kOhms)
 *   - Altitude (meters)
 *   Displays the readings on the OLED and prints any failures to the Serial Monitor.
 */

void processBME680()
{
    if (bme.performReading())
    {
        temperature = bme.temperature;
        humidity = bme.humidity;
        pressure = bme.pressure / 100.0;
        gas = bme.gas_resistance / 1000.0;
        altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

        // Display on OLED
        displayBME680Readings(temperature, humidity, pressure, gas, altitude);

        // Print to Serial Monitor
        printBME680Readings(temperature, humidity, pressure, gas, altitude);
    }
    else
    {
        Serial.println("BME680 failed to perform reading!");
    }
}

/*
 * ==================================================
 * FUNCTION: PROCESS MQ-2 SENSOR
 * ==================================================
 * Description:
 *   Reads gas concentration data from the MQ-2 sensor, including:
 *   - LPG (ppm)
 *   - CO (ppm)
 *   - Smoke (ppm)
 *   Displays the readings on the OLED and triggers safety alerts if thresholds
 *   are exceeded (via NeoPixels and buzzer).
 */

void processMQ2()
{
    MQ2.update();

    MQ2.setA(574.25);
    MQ2.setB(-2.222);
    lpg = MQ2.readSensor();

    MQ2.setA(36974);
    MQ2.setB(-3.109);
    co = MQ2.readSensor();

    MQ2.setA(3616.1);
    MQ2.setB(-2.675);
    smoke = MQ2.readSensor();

    // Display on OLED
    displayMQ2Readings(lpg, co, smoke);

    // Print to Serial Monitor
    printMQ2Readings(lpg, co, smoke);

    // Trigger alerts if needed
    checkSafetyAndAlert(lpg, co, smoke);
}