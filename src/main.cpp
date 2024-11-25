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
 * ██████████████████████████ CONFIGURATION █████████████████████████████
 * ==================================================================================
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
 * ==================================================================================
 * █████████████████████████ GLOBAL VARIABLES ██████████████████████████████████████
 * ==================================================================================
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
 * ==================================================================================
 * ██████████████████████████ OBJECTS & ENUM █████████████████████████████
 * ==================================================================================
 */

// ENUM STATUS
enum Status
{
  SAFE,
  WARNING,
  DANGER
};

// SENSOR OBJECT INSTANTIATION
Adafruit_BME680 bme;
MQUnifiedsensor MQ2(MQ2_BOARD, MQ2_VOLTAGE_RESOLUTION, MQ2_ADC_RESOLUTION, MQ2_PIN, MQ2_TYPE);
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*
 * ==================================================================================
 * ██████████████████████████ FUNCTION PROTOTYPES █████████████████████████████
 * ==================================================================================
 */

// Sensor Initialization
void initializeBuzzer();
void initializeNeoPixels();
void initializeOLED();
void initializeBME680();
void initializeMQ2();
void initializeSoundSensor();

// Sensor Processing
void processSoundSensor();
void processBME680();
void processMQ2();

// Helper Functions
void testBuzzer();
void testNeoPixels();
void checkSafetyAndAlert(float lpg, float co, float smoke);
void setNeoPixelStatus(Status status);
float convertRawSoundToDecibels(int rawValue);
void checkWiFi();

// OLED Display Functions
void displayWelcomeLogo();
void displayParrotGif();
void displayWaveAnimation();
void displayBME680Readings(float temperature, float humidity, float pressure, float gas, float altitude);
void displayMQ2Readings(float lpg, float co, float smoke);
void displaySoundSensorReading(float soundLevel);

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
 * ██████████████████████████████████████████████████████████████████████████████████
 * ██████████████████████ ALL FUNCTIONS ARE LISTED BELOW ████████████████████████████
 * ██████████████████████████████████████████████████████████████████████████████████
 *
 * ╔═══════════════════════════════════════════════════════════════════════════════╗
 * ║ 1. Sensor Initialization:                                                     ║
 * ║    - Prepares all sensors (e.g., BME680, MQ-2, KY-038) for use.               ║
 * ║    - Handles configurations, calibrations, and error checking.                ║
 * ║                                                                               ║
 * ║ 2. Sensor Processing:                                                         ║
 * ║    - Processes raw data from sensors (e.g., BME680, MQ-2, KY-038).            ║
 * ║    - Includes data acquisition, computation, and triggering of alerts.        ║
 * ║                                                                               ║
 * ║ 3. Helper Functions:                                                          ║
 * ║    - Peripheral initialization and testing (e.g., buzzer, NeoPixels).         ║
 * ║    - Status management using NeoPixels and buzzer alerts.                     ║
 * ║    - General utility operations like sound-to-decibel conversion.             ║
 * ║                                                                               ║
 * ║ 4. OLED Display Functions:                                                    ║
 * ║    - Handles displaying processed sensor data on the OLED screen.             ║
 * ║    - Examples include environmental readings (temperature, humidity),         ║
 * ║      gas concentrations, and sound levels.                                    ║
 * ║                                                                               ║
 * ║ 5. Serial Monitor Print Functions:                                            ║
 * ║    - Outputs sensor data and debug information to the Serial Monitor.         ║
 * ║    - Includes detailed logs for monitoring system performance.                ║
 * ╚═══════════════════════════════════════════════════════════════════════════════╝
 *
 * ██████████████████████████████████████████████████████████████████████████████████
 */

/*
 * ==================================================================================
 * ███████████████████████████ SENSOR INITIALIZATION ████████████████████████████████
 * ==================================================================================
 *
 * This section is responsible for initializing all sensors used in the project.
 * Each sensor's setup function ensures proper configuration, calibration,
 * and error handling before data acquisition begins.
 *
 * ╔═══════════════════════════════════════════════════════════════════════════════╗
 * ║ - Sets up the KY-038 Sound Sensor by defining the input pin mode.             ║
 * ║ - Configures the BME680 Environmental Sensor, including oversampling settings ║
 * ║   and gas heater configuration.                                               ║
 * ║ - Calibrates the MQ-2 Gas Sensor, ensuring accurate readings for LPG, CO,     ║
 * ║   and Smoke concentrations.                                                   ║
 * ║ - Includes error checks for initialization failures and calibration issues.   ║
 * ╚═══════════════════════════════════════════════════════════════════════════════╝
 *
 * Functions:
 * - `initializeBuzzer()`: Configures the buzzer pin as output and sets it to LOW.
 * - `initializeNeoPixels()`: Configures and tests the NeoPixel LED array.
 * - `initializeOLED()`: Sets up the OLED display for use.
 * - `initializeSoundSensor()`: Prepares the KY-038 sound sensor for use.
 * - `initializeBME680()`: Configures the BME680 for environmental data collection.
 * - `initializeMQ2()`: Calibrates and prepares the MQ-2 for gas monitoring.
 *
 * ==================================================================================
 * ███████████████████████████ SENSOR INITIALIZATION ████████████████████████████████
 * ==================================================================================
 */

/*
 * ==================================================
 * FUNCTION: INITIALIZE BUZZER
 * ==================================================
 * Description:
 *   Configures the buzzer pin as output and sets it to LOW.
 */

void initializeBuzzer()
{
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Buzzer initialized!");
}

/*
 * ==================================================
 * FUNCTION: INITIALIZE NEOPIXELS
 * ==================================================
 * Description:
 *   Configures and initializes the NeoPixel LED array. Performs a test sequence
 *   to verify that the NeoPixels are functioning correctly.
 */

void initializeNeoPixels()
{
  pixels.begin();
  testNeoPixels();
  Serial.println("NeoPixels initialized!");
}

/*
 * ==================================================
 * FUNCTION: INITIALIZE OLED
 * ==================================================
 * Description:
 *   Initializes the SH1106 OLED display. Clears the screen for fresh use and
 *   halts the program if the initialization fails.
 */

void initializeOLED()
{
  if (!display.begin(0x3C))
  {
    Serial.println("SH1106 initialization failed!");
    while (true)
      ;
  }
  display.clearDisplay();
  Serial.println("OLED initialized!");
}

/*
 * ==================================================
 * FUNCTION: INITIALIZE SOUND SENSOR
 * ==================================================
 * Description:
 *   Configures the KY-038 sound sensor pin as input. This sensor requires no
 *   advanced calibration, making setup straightforward.
 */

void initializeSoundSensor()
{
  pinMode(KY038_PIN, INPUT);
  Serial.println("Sound sensor initialized!");
}

/*
 * ==================================================
 * FUNCTION: INITIALIZE BME680 SENSOR
 * ==================================================
 * Description:
 *   Configures the BME680 sensor with oversampling, filter size, and gas heater
 *   settings. Checks for initialization success and sets up sensor parameters.
 */

void initializeBME680()
{
  if (!bme.begin(BME680_ADDRESS))
  {
    Serial.println("BME680 initialization failed!");
    while (true)
      ;
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  Serial.println("BME680 initialized!");
}

/*
 * ==================================================
 * FUNCTION: INITIALIZE MQ-2 SENSOR
 * ==================================================
 * Description:
 *   Calibrates the MQ-2 gas sensor by calculating the clean air R0 value. This
 *   ensures accurate readings for LPG, CO, and Smoke levels.
 */

void initializeMQ2()
{
  MQ2.setRegressionMethod(1);
  MQ2.init();

  Serial.println("Calibrating MQ-2...");
  float calcR0 = 0;
  for (int i = 0; i < 10; i++)
  {
    MQ2.update();
    calcR0 += MQ2.calibrate(MQ2_RATIO_CLEAN_AIR);
    delay(100);
  }
  MQ2.setR0(calcR0 / 10);

  if (isinf(calcR0) || calcR0 == 0)
  {
    Serial.println("MQ-2 calibration failed!");
    while (true)
      ;
  }

  Serial.println("MQ-2 initialized and calibrated!");
}

/*
 * ==================================================================================
 * █████████████████████████████ SENSOR PROCESSING ██████████████████████████████████
 * ==================================================================================
 *
 * This section handles all sensor-related data processing. Each function in this
 * section is responsible for:
 *
 * ╔═══════════════════════════════════════════════════════════════════════════════╗
 * ║ - Acquiring data from specific sensors, such as the KY-038 Sound Sensor,      ║
 * ║   BME680 Environmental Sensor, and MQ-2 Gas Sensor.                           ║
 * ║ - Applying calibration and threshold logic to interpret raw sensor data.      ║
 * ║ - Triggering appropriate alerts or displaying processed data on the OLED.     ║
 * ╚═══════════════════════════════════════════════════════════════════════════════╝
 *
 * Functions:
 * - `processSoundSensor()`: Handles KY-038 sound sensor data and alerts.
 * - `processBME680()`: Reads and processes environmental data from the BME680.
 * - `processMQ2()`: Monitors gas levels and manages safety alerts via MQ-2.
 *
 * ==================================================================================
 * █████████████████████████████ SENSOR PROCESSING ██████████████████████████████████
 * ==================================================================================
 */

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

/*
 * ==================================================================================
 * █████████████████████████████ HELPER FUNCTIONS ███████████████████████████████████
 * ==================================================================================
 *
 * This section is dedicated to all the helper functions used throughout
 * the codebase. These functions perform operations such as:
 *
 * ╔═══════════════════════════════════════════════════════════════════════════════╗
 * ║ - Peripheral initialization and testing.                                      ║
 * ║ - Alert management via NeoPixels and buzzer.                                  ║
 * ║ - Data conversion and utility operations.                                     ║
 * ╚═══════════════════════════════════════════════════════════════════════════════╝
 *
 * ==================================================================================
 * █████████████████████████████ HELPER FUNCTIONS ███████████████████████████████████
 * ==================================================================================
 */

/*
 * ==================================================
 * FUNCTION: TEST BUZZER
 * ==================================================
 * Description:
 *   This function activates the buzzer three times to test its functionality.
 *   It is called during setup to ensure the buzzer is operational.
 */

void testBuzzer()
{
  Serial.println("Testing buzzer...");
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
    delay(500);
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
    delay(500);
  }
  Serial.println("Buzzer test completed.");
}

/*
 * ==================================================
 * FUNCTION: TEST NEOPIXELS
 * ==================================================
 * Description:
 *   This function cycles through red, green, and blue colors on the NeoPixel
 *   LEDs to test their functionality. Each color is displayed for 500ms.
 */

void testNeoPixels()
{
  Serial.println("Testing NeoPixels...");
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Red
    pixels.show();
    delay(500);
    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green
    pixels.show();
    delay(500);
    pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // Blue
    pixels.show();
    delay(500);
  }
  pixels.clear();
  pixels.show();
  Serial.println("NeoPixel test completed.");
}

/*
 * ==================================================
 * FUNCTION: CHECK SAFETY AND ALERT
 * ==================================================
 * Description:
 *   Evaluates sensor readings for gas levels (LPG, CO, smoke) and triggers
 *   an alert if unsafe levels are detected. Alerts include activating the
 *   buzzer and setting the NeoPixel LEDs to corresponding danger levels.
 */

void checkSafetyAndAlert(float lpg, float co, float smoke)
{
  if (lpg > 1000 || co > 50 || smoke > 200)
  {
    Serial.println("ALERT: Unsafe gas levels detected!");
    digitalWrite(BUZZER_PIN, HIGH);
    setNeoPixelStatus(DANGER);
    delay(2000);
    digitalWrite(BUZZER_PIN, LOW);
  }
  else if (lpg > 500 || co > 20 || smoke > 100)
  {
    Serial.println("Warning: Elevated gas levels detected!");
    setNeoPixelStatus(WARNING);
  }
  else
  {
    Serial.println("Gas levels are within safe limits.");
    setNeoPixelStatus(SAFE);
  }
}

/*
 * ==================================================
 * FUNCTION: SET NEOPIXEL STATUS
 * ==================================================
 * Description:
 *   Updates the NeoPixel LEDs based on the provided status (SAFE, WARNING, DANGER).
 *   Each status is associated with a specific color and flashing pattern.
 */

void setNeoPixelStatus(Status status)
{
  switch (status)
  {
  case SAFE:
    for (int i = 0; i < 3; i++)
    { // Blue flashing
      pixels.fill(pixels.Color(0, 0, 255));
      pixels.show();
      delay(500);
      pixels.clear();
      pixels.show();
      delay(500);
    }
    break;
  case WARNING:
    for (int i = 0; i < 3; i++)
    { // Green flashing
      pixels.fill(pixels.Color(0, 255, 0));
      pixels.show();
      delay(500);
      pixels.clear();
      pixels.show();
      delay(500);
    }
    break;
  case DANGER:
    for (int i = 0; i < 3; i++)
    { // Red flashing
      pixels.fill(pixels.Color(255, 0, 0));
      pixels.show();
      delay(500);
      pixels.clear();
      pixels.show();
      delay(500);
    }
    return;
  }
  pixels.show();
}

/*
 * ==================================================
 * FUNCTION: CONVERT RAW SOUND TO DECIBELS
 * ==================================================
 * Description:
 *   Converts the raw analog input value from the KY-038 sound sensor to decibels (dB).
 *   The conversion is based on a mapped range of 30-100 dB.
 */

float convertRawSoundToDecibels(int rawValue)
{
  return map(rawValue, 0, 1023, 30, 100); // Map analog value to decibels (30-100 dB range)
}

/*
 * ==================================================
 * FUNCTION: CHECK WIFI
 * ==================================================
 * Description:
 *   Checks wifi status
 */

void checkWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToWiFi();
  }
}

/*
 * ==================================================================================
 * ███████████████████████████ OLED DISPLAY FUNCTIONS ██████████████████████████████
 * ==================================================================================
 *
 * This section is dedicated to functions handling the OLED display.
 * These functions are responsible for rendering sensor readings, messages,
 * and other visual data onto the screen in an organized manner.
 *
 * ╔═══════════════════════════════════════════════════════════════════════════════╗
 * ║ - Rendering sensor readings (temperature, humidity, gas levels, etc.).        ║
 * ║ - Displaying warnings and alerts.                                             ║
 * ║ - Showing a welcome logo or introductory screen.                              ║
 * ╚═══════════════════════════════════════════════════════════════════════════════╝
 *
 * ==================================================================================
 * ███████████████████████████ OLED DISPLAY FUNCTIONS ██████████████████████████████
 * ==================================================================================
 */

/*
 * ==================================================
 * FUNCTION: DISPLAY WELCOME LOGO
 * ==================================================
 * Description:
 *   Displays a custom bitmap logo on the OLED screen during initialization.
 *   The logo is shown for 5 seconds.
 */

void displayWelcomeLogo()
{
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_logo, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(5000);
}

/*
 * ==================================================
 * FUNCTION: DISPLAY PARROT GIF
 * ==================================================
 * Description:
 *   Displays a custom bitmap GIF on the OLED screen during initialization.
 */

void displayParrotGif()
{
  // parrot 1
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot1, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 2
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot2, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 3
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot3, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 4
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot4, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 5
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot5, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 6
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot6, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 7
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot7, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 8
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot8, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 9
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot9, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
  // parrot 10
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap_parrot10, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(500);
}

/*
 * ==================================================
 * FUNCTION: DISPLAY WAVE ANIMATION
 * ==================================================
 * Description:
 *   Displays the wave animation on the OLED screen during sensor reading intervals
 */

void displayWaveAnimation()
{
  for (int t = 0; t < 100; t++)
  { // Time steps for animation
    display.clearDisplay();
    for (int x = 0; x < 128; x++)
    {
      int y = 32 + 16 * sin(2 * 3.14 * x / 64 + t / 10.0); // Sine wave
      display.drawPixel(x, y, 1);
    }
    display.display();
    delay(50);
  }
}

/*
 * ==================================================
 * FUNCTION: DISPLAY BME680 READINGS
 * ==================================================
 * Description:
 *   Displays sensor readings from the BME680 on the OLED display. The readings
 *   include temperature, humidity, pressure, gas resistance, and altitude,
 *   formatted for clarity.
 */

void displayBME680Readings(float temperature, float humidity, float pressure, float gas, float altitude)
{

  // Display BME-680 Sensor
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(1);
  display.setCursor(0, 10);
  display.print("BME680");
  display.setTextSize(2);
  display.setTextColor(1);
  display.setCursor(0, 45);
  display.print("SENSOR");

  display.display();
  delay(5000);

  // Display Temperature
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.print("Temperature:");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.printf("%.1f C", temperature);
  display.display();

  // Display Humidity
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 35);
  display.print("Relative Humidity:");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.printf("%.1f %%", humidity);

  display.display();
  delay(5000);

  // Display Pressure
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.print("Barometric Pressure:");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.printf("%.1f hPa", pressure);
  display.display();

  // Display Gas Resistance
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 35);
  display.print("Gas Resistance:");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.printf("%.1f kOhms", gas);

  display.display();
  delay(5000);

  // Display Altitude
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.print("Altitude:");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.printf("%.1f m", altitude);

  display.display();
  delay(5000);
}

/*
 * ==================================================
 * FUNCTION: DISPLAY MQ-2 READINGS
 * ==================================================
 * Description:
 *   Displays sensor readings from the MQ-2 sensor on the OLED display. The
 *   readings include LPG, CO, and smoke levels, shown with clear labeling.
 */

void displayMQ2Readings(float lpg, float co, float smoke)
{

  // Display MQ-2 Sensor
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(1);
  display.setCursor(0, 10);
  display.print("MQ-2");
  display.setTextSize(2);
  display.setTextColor(1);
  display.setCursor(0, 45);
  display.print("SENSOR");

  display.display();
  delay(5000);

  // LPG Reading
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.print("LPG:");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.printf("%.1f ppm", lpg);

  // CO Reading
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("CO:");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.printf("%.1f ppm", co);

  display.display();
  delay(5000);

  // Smoke Reading
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.print("Smoke:");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.printf("%.1f ppm", smoke);

  display.display();
  delay(5000);
}

/*
 * ==================================================
 *  FUNCTION: DISPLAY KY-038 READING
 * ==================================================
 * Description:
 *   Displays the sound level from the KY-038 sensor on the OLED display. The
 *   reading includes the sound level in decibels (dB) and indicates whether
 *   the environment is LOUD or Normal.
 */

void displaySoundSensorReading(float soundLevel)
{
  float soundDecibels = convertRawSoundToDecibels(soundLevel);

  // Display KY-038 Sensor
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(1);
  display.setCursor(0, 10);
  display.print("KY-038");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print("SENSOR");

  display.display();
  delay(5000);

  // Sound Reading
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Sound Level:");
  display.setCursor(0, 15);
  display.printf("Level: %.1f dB", soundDecibels);
  display.setTextSize(2);
  display.setCursor(0, 35);
  display.print(soundDecibels > LOUD_THRESHOLD ? "LOUD" : "Normal");

  display.display();
  delay(5000);
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
