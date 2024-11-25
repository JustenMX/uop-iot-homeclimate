#include "oled_display.h"
#include "helper_functions.h"
#include "bitmap_logo.h"
#include "bitmap_parrot.h"

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
