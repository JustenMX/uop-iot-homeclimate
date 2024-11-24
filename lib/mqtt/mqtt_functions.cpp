#include "mqtt_functions.h"

// MQTT Connection Setup
void setupMQTT(PubSubClient &client)
{
    client.setServer(MQTT_BROKER, MQTT_PORT);
}

// Reconnect to MQTT Broker
void reconnectMQTT(PubSubClient &client)
{
    while (!client.connected())
    {
        Serial.print("Connecting to MQTT broker...");
        if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD))
        {
            Serial.println("Connected!");
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.println(client.state());
            Serial.println("Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

// Publish Sensor Readings to MQTT
void publishMQTTReadings(
    PubSubClient &client,
    float temperature,
    float humidity,
    float pressure,
    float gas,
    float altitude,
    float lpg,
    float co,
    float smoke,
    float sound)
{

    // Ensure MQTT connection
    if (!client.connected())
    {
        reconnectMQTT(client);
    }
    client.loop();

    // Debug sensor readings
    Serial.println("Preparing to publish MQTT readings...");
    Serial.print("Temperature: ");
    Serial.println(temperature);

    Serial.print("Humidity: ");
    Serial.println(humidity);

    Serial.print("Pressure: ");
    Serial.println(pressure);

    Serial.print("Gas Resistance: ");
    Serial.println(gas);

    Serial.print("Altitude: ");
    Serial.println(altitude);

    Serial.print("LPG Levels: ");
    Serial.println(lpg);

    Serial.print("CO Levels: ");
    Serial.println(co);

    Serial.print("Smoke Levels: ");
    Serial.println(smoke);

    Serial.print("Sound Levels: ");
    Serial.println(sound);

    Serial.println("All sensor readings logged.");

    // Publish sensor readings to MQTT topics
    client.publish(TOPIC_TEMPERATURE, String(temperature).c_str(), true);
    client.publish(TOPIC_HUMIDITY, String(humidity).c_str(), true);
    client.publish(TOPIC_PRESSURE, String(pressure).c_str(), true);
    client.publish(TOPIC_GAS, String(gas).c_str(), true);
    client.publish(TOPIC_ALTITUDE, String(altitude).c_str(), true);
    client.publish(TOPIC_LPG, String(lpg).c_str(), true);
    client.publish(TOPIC_CO, String(co).c_str(), true);
    client.publish(TOPIC_SMOKE, String(smoke).c_str(), true);
    client.publish(TOPIC_SOUND, String(sound).c_str(), true);

    Serial.println("MQTT readings successfully published!");
}