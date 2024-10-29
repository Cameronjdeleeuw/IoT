#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Hi";
const char* password = "veryhardpassword";
const char* server = "https://camerondeleeuw.com/results.txt"; // HTTPS URL

WiFiClientSecure client;  // Secure client for HTTPS
const int ledPin = D5;

void setup() {
    pinMode(ledPin, OUTPUT);
    Serial.begin(9600);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    client.setInsecure();  // Skips SSL certificate validation
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(client, server);  // Use secure client with HTTPS server
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("LED Status: " + payload);
            
            if (payload == "on") {
                digitalWrite(ledPin, HIGH);
            } else if (payload == "off") {
                digitalWrite(ledPin, LOW);
            }
        } else {
            Serial.println("Error in HTTP request");
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}
