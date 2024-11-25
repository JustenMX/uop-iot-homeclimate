#include "hardware_init.h"
#include "helper_function.h"
#include "sensor_processing.h"
#include "oled_display.h"
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
