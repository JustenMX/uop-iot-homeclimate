#ifndef MQTT_FUNCTIONS_H
#define MQTT_FUNCTIONS_H

#include <PubSubClient.h>
#include "mqtt_config.h"

// Function Declarations
void setupMQTT(PubSubClient &client);
void reconnectMQTT(PubSubClient &client);
void publishMQTTReadings(PubSubClient &client, float temperature, float humidity, float pressure, float gas, float altitude, float lpg, float co, float smoke, float sound);

#endif