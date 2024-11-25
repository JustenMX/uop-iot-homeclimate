#include "hardware_init.h"

// Hardware Initialization
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME680 bme;
MQUnifiedsensor MQ2(MQ2_BOARD, MQ2_VOLTAGE_RESOLUTION, MQ2_ADC_RESOLUTION, MQ2_PIN, MQ2_TYPE);
WiFiClient espClient;
PubSubClient client(espClient);

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