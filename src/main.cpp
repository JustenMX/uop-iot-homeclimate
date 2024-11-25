#include "hardware_init.h"
#include "wifi_setup.h"
#include "ota_setup.h"
#include "bitmap_logo.h"
#include "bitmap_parrot.h"
//
#include "../lib/mqtt/mqtt_config.h"
#include "../lib/mqtt/mqtt_functions.h"

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
