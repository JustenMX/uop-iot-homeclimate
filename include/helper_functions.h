#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "hardware_init.h"

// ENUM STATUS
enum Status
{
    SAFE,
    WARNING,
    DANGER
};

/*
 * =================================================
 * ███████████████ FUNCTION DECLARATION ████████████
 * =================================================
 */

void testBuzzer();
void testNeoPixels();
void checkSafetyAndAlert(float lpg, float co, float smoke);
void setNeoPixelStatus(Status status);
float convertRawSoundToDecibels(int rawValue);
void checkWiFi();

#endif