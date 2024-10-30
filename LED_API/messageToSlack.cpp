#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "Hi";
const char* password = "veryhardpassword";
const char* iftttUrl = "https://maker.ifttt.com/trigger/cam_button_pressed/json/with/key/bjS5Ffng4PTymuf9U6Uvmk";
const int buttonPin = D0;
bool buttonPressed = false;

void sendToIFTTT();

WiFiClientSecure client;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WifI...");
  }
  Serial.println("Connected to WiFi");

  client.setInsecure();  
}

void loop() {
  if (digitalRead(buttonPin) == HIGH && !buttonPressed) {
    buttonPressed = true;
    sendToIFTTT();
  } else if (digitalRead(buttonPin) == LOW) {
    buttonPressed = false;
  }
}

void sendToIFTTT() {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect("maker.ifttt.com", 443)) {
      String url = "/trigger/cam_button_pressed/with/key/bjS5Ffng4PTymuf9U6Uvmk";
      client.println("GET " + url + " HTTP/1.1");
      client.println("Host: maker.ifttt.com");
      client.println("Connection: close");
      client.println();

      while (client.connected() || client.available()) {
        if (client.available()) {
          String line = client.readStringUntil('\n');
          Serial.println(line);
        }
      }
      client.stop();
      Serial.println("IFTTT request sent");
    }
  } else {
    Serial.println("WiFi not connected");
  }
}
