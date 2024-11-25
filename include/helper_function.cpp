#include "helper_function.h"
#include "wifi_setup.h"

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
