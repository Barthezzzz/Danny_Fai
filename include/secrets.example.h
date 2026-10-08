#ifndef SECRETS_H
#define SECRETS_H

// Wi-Fi Credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// MQTT Broker Credentials (HiveMQ)
const char* MQTT_BROKER = "your-cluster-url.hivemq.cloud";
const int   MQTT_PORT = 8883; // Secure TLS port
const char* MQTT_USER = "YOUR_MQTT_USERNAME";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";

#endif