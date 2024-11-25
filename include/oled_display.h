#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

void displayWelcomeLogo();
void displayParrotGif();
void displayWaveAnimation();
void displayBME680Readings(float temperature, float humidity, float pressure, float gas, float altitude);
void displayMQ2Readings(float lpg, float co, float smoke);
void displaySoundSensorReading(float soundLevel);

#endif