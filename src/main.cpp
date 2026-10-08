#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <AccelStepper.h>
#include "secrets.h"

// Ορισμός Pins για το ULN2003 Driver
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

// AccelStepper σε λειτουργία 4-wire half-step (Τύπος 8)
// Προσοχή στη σειρά ακολουθίας πηνίων: IN1, IN3, IN2, IN4
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

// MQTT Topics
const char* TOPIC_COMMAND = "stepper/command";
const char* TOPIC_STATUS  = "stepper/status";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void connectToWiFi() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    message.trim();
    Serial.printf("Received on [%s]: %s\n", topic, message.c_str());

    // Αποκωδικοποίηση εντολών
    if (message == "STOP") {
        stepper.stop();
        Serial.println("Action: STOP");
    } else {
        // Αναμένει αριθμό σχετικών βημάτων (π.χ. "512", "-512", "2048")
        long steps = message.toInt();
        if (steps != 0 || message == "0") {
            stepper.move(steps);
            Serial.printf("Action: Moving %ld steps\n", steps);
        } else {
            Serial.println("Action: Unknown command format");
        }
    }
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Connecting to EMQX MQTT Broker...");
        String clientId = "ESP32_Stepper_" + String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
            Serial.println(" Connected!");
            client.subscribe(TOPIC_COMMAND);
            client.publish(TOPIC_STATUS, "online");
        } else {
            Serial.printf(" Failed, rc=%d. Retrying in 5 seconds...\n", client.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Ρυθμίσεις κινητήρα 28BYJ-48
    stepper.setMaxSpeed(800.0);
    stepper.setAcceleration(400.0);

    // Ρυθμίσεις δικτύου
    connectToWiFi();
    espClient.setInsecure(); // Παράκαμψη αυστηρού ελέγχου CA για απλοποίηση
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(mqttCallback);
}

void loop() {
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();

    // Εκτέλεση κίνησης βημάτων
    stepper.run();

    // Απενεργοποίηση πηνίων όταν το μοτέρ είναι σταματημένο (για να μην καίει ρεύμα/ζεσταίνεται)
    if (stepper.distanceToGo() == 0) {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
    }
}