#ifndef SERIAL_MONITOR_H
#define SERIAL_MONITOR_H

void printBME680Readings(float temperature, float humidity, float pressure, float gas, float altitude);
void printMQ2Readings(float lpg, float co, float smoke);
void printSoundSensorReadings(float soundLevel);

#endif