#include "serial_monitor.h"
#include "helper_functions.h"

/*
 * ==================================================
 * FUNCTION: PRINT BME680 DATA
 * ==================================================
 * Description:
 *   Prints sensor readings from the BME680 to the Serial Monitor. The readings
 *   include temperature, humidity, pressure, gas resistance, and altitude,
 *   formatted for easy debugging and monitoring.
 */

void printBME680Readings(float temperature, float humidity, float pressure, float gas, float altitude)
{
    Serial.println(F("BME680 Sensor Readings:"));
    Serial.printf("Temperature: %.1f °C\n", temperature);
    Serial.printf("Relative Humidity: %.1f %%\n", humidity);
    Serial.printf("Barometric Pressure: %.1f hPa\n", pressure);
    Serial.printf("Gas Resistance: %.1f kOhms\n", gas);
    Serial.printf("Altitude: %.1f m\n", altitude);
    Serial.println("----------------------------");
}

/*
 * ==================================================
 *  FUNCTION: PRINT MQ-2 DATA
 * ==================================================
 * Description:
 *   Prints sensor readings from the MQ-2 sensor to the Serial Monitor. The
 *   readings include LPG, CO, and smoke levels, along with warnings if any
 *   values exceed safe thresholds.
 */

void printMQ2Readings(float lpg, float co, float smoke)
{
    Serial.println(F("MQ-2 Sensor Readings:"));
    Serial.printf("LPG: %.1f ppm\n", lpg);
    Serial.printf("CO: %.1f ppm\n", co);
    Serial.printf("Smoke: %.1f ppm\n", smoke);

    if (lpg > 1000 || co > 50 || smoke > 200)
    {
        Serial.println("Warning: Unsafe gas levels detected!");
    }
    else
    {
        Serial.println("Gas levels are within safe limits.");
    }
    Serial.println("----------------------------");
}

/*
 * ==================================================
 *  FUNCTION: PRINT KY-038 DATA
 * ==================================================
 * Description:
 *   Prints sensor readings from the KY-038 sound sensor to the Serial Monitor.
 *   Indicates whether the sound level exceeds the threshold for loudness.
 */

void printSoundSensorReadings(float soundLevel)
{
    Serial.println(F("KY-038 Sound Sensor Readings:"));
    Serial.printf("Sound Level: %.1f dB\n", soundLevel);

    if (soundLevel > LOUD_THRESHOLD)
    {
        Serial.println("Warning: Loud sound detected!");
    }
    else
    {
        Serial.println("Sound levels are within safe limits.");
    }
    Serial.println("----------------------------");
}
