#include "hardware_init.h"
#include "helper_function.h"
//
#include "wifi_setup.h"
#include "ota_setup.h"
#include "bitmap_logo.h"
#include "bitmap_parrot.h"
//
#include "../lib/mqtt/mqtt_config.h"
#include "../lib/mqtt/mqtt_functions.h"

/*
 * ==================================================================================
 * ██████████████████████████ FUNCTION PROTOTYPES █████████████████████████████
 * ==================================================================================
 */

// Serial Monitor Functions
void printBME680Readings(float temperature, float humidity, float pressure, float gas, float altitude);
void printMQ2Readings(float lpg, float co, float smoke);
void printSoundSensorReadings(float soundLevel);

/*
 * ==================================================================================
 * ██████████████████████████ VOID SETUP () █████████████████████████████
 * ==================================================================================
 */

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Connect to Wi-Fi
  connectToWiFi();

  // Setup OTA
  setupOTA();

  // Setup MQTT
  setupMQTT(client);

  initializeBuzzer();
  initializeNeoPixels();
  initializeOLED();
  initializeBME680();
  initializeMQ2();
  initializeSoundSensor();
}

/*
 * ==================================================================================
 * ██████████████████████████ VOID LOOP () █████████████████████████████
 * ==================================================================================
 */

void loop()
{
  // Handle OTA updates
  handleOTA();

  // Reconnect Wi-Fi if needed
  checkWiFi();

  // Reconnect MQTT if needed
  if (!client.connected())
  {
    reconnectMQTT(client);
  }
  client.loop();

  // Process Sensors and Display on OLED
  displayWelcomeLogo();
  //
  displayWaveAnimation();
  processSoundSensor();
  //
  displayWaveAnimation();
  processBME680();
  //
  displayWaveAnimation();
  processMQ2();

  // Publish updated sensor readings to MQTT
  publishMQTTReadings(client, temperature, humidity, pressure, gas, altitude, lpg, co, smoke, sound);

  // Gif plays as delay
  displayParrotGif();
}

/*
 * ==================================================================================
 * ██████████████████████████ SERIAL MONITOR FUNCTIONS █████████████████████████████
 * ==================================================================================
 *
 * This section focuses on functions responsible for printing data and messages
 * to the Serial Monitor. These functions are used for debugging, monitoring
 * sensor outputs, and providing real-time feedback to developers.
 *
 * ╔═══════════════════════════════════════════════════════════════════════════════╗
 * ║ - Printing sensor readings (temperature, humidity, gas levels, etc.).         ║
 * ║ - Displaying system status messages (e.g., warnings, errors).                 ║
 * ║ - Providing diagnostic information during testing and development.            ║
 * ╚═══════════════════════════════════════════════════════════════════════════════╝
 *
 * ==================================================================================
 * ██████████████████████████ SERIAL MONITOR FUNCTIONS █████████████████████████████
 * ==================================================================================
 */

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
