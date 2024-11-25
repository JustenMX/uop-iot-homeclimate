#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_NeoPixel.h>
#include <MQUnifiedsensor.h>
#include <WiFi.h>
#include <PubSubClient.h>

/*
 * =================================================
 * ███████████████ CONFIGURATION ███████████████████
 * =================================================
 */

// OLED CONFIGURATION
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// BME680 SENSOR CONFIGURATION
#define SDA_PIN 21
#define SCL_PIN 22
#define BME680_ADDRESS 0x77
#define SEALEVELPRESSURE_HPA (1013.25)

// MQ-2 SENSOR CONFIGURATION
#define MQ2_BOARD "ESP32"
#define MQ2_PIN 39
#define MQ2_TYPE "MQ-2"
#define MQ2_VOLTAGE_RESOLUTION 3.3
#define MQ2_ADC_RESOLUTION 12
#define MQ2_RATIO_CLEAN_AIR 9.83

// KY-038 SENSOR CONFIGURATION
#define KY038_PIN 34
#define LOUD_THRESHOLD 70.0 // Threshold for loudness in dB

// NEOPIXEL CONFIGURATION
#define NEOPIXEL_PIN 16
#define NUM_PIXELS 5
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// BUZZER CONFIGURATION
#define BUZZER_PIN 25

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

/*
 * =================================================
 * ███████████████ GLOBAL VARIABLE █████████████████
 * =================================================
 */

// BME680 Sensor Readings
float temperature; // Temperature reading (°C)
float humidity;    // Humidity reading (%)
float pressure;    // Barometric pressure reading (hPa)
float gas;         // Gas resistance reading (kΩ)
float altitude;    // Altitude reading (meters)

// MQ-2 Gas Sensor Readings
float lpg;   // LPG gas concentration (ppm)
float co;    // Carbon Monoxide concentration (ppm)
float smoke; // Smoke concentration (ppm)

// KY-038 Sound Sensor Reading
float sound; // Sound level (dB)

/*
 * =================================================
 * ███████████████ OBJECTS █████████████████████████
 * =================================================
 */

extern Adafruit_NeoPixel pixels;
extern Adafruit_SH1106G display;
extern Adafruit_BME680 bme;
extern MQUnifiedsensor MQ2;
extern WiFiClient espClient;
extern PubSubClient client;

/*
 * =================================================
 * ███████████████ FUNCTION DECLARATION ████████████
 * =================================================
 */

// Hardware Initialization
void initializeBuzzer();
void initializeNeoPixels();
void initializeOLED();
void initializeBME680();
void initializeMQ2();
void initializeSoundSensor();

#endif
