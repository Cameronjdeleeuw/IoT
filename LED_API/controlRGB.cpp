#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Hi";  // Your network SSID
const char* password = "veryhardpassword";  // Your network password
const char* serverURL = "https://camerondeleeuw.com/getRGB.txt";  // URL to getRGB.txt

// Define the pins for each color channel of the RGB LED
const int redPin = D6;
const int greenPin = D7;
const int bluePin = D8;

WiFiClientSecure client;

// Function to parse individual RGB values from the payload
int parseValue(const String &data, char color);

void setup() {
    Serial.begin(9600);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    client.setInsecure();  // Skip certificate validation for HTTPS
}

void loop() {
    HTTPClient http;
    http.begin(client, serverURL);

    int httpCode = http.GET();
    if (httpCode > 0) {
        // Check for successful response
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Response: " + payload);

            // Parse RGB values from the response
            int redValue = parseValue(payload, 'R');
            int greenValue = parseValue(payload, 'G');
            int blueValue = parseValue(payload, 'B');

            // Write to RGB pins
            analogWrite(redPin, redValue);
            analogWrite(greenPin, greenValue);
            analogWrite(bluePin, blueValue);

            // Log values for debugging
            Serial.println("Red: " + String(redValue) + ", Green: " + String(greenValue) + ", Blue: " + String(blueValue));
        }
    } else {
        Serial.println("Error in HTTP request");
    }
    http.end();

    delay(10000);  // Check every 10 seconds
}

// Function to parse individual RGB values from the payload
int parseValue(const String &data, char color) {
    int start = data.indexOf(color + String("=")) + 2;  // Find "R=", "G=", or "B="
    int end = data.indexOf(color == 'B' ? '\n' : 'G' + 1 == color ? 'B' : 'G', start);  // Find end of value
    String value = data.substring(start, end);  // Extract value as string
    return value.toInt();  // Convert to integer
}
