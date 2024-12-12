#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi variables
const char* ssid = "Hi";  // WiFi name
const char* password = "veryhardpassword";  // WiFi password

// MQTT variables
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* potTopic = "testtopic/temp/outTopic/updatePotRead1883"; // Topic to publish potentiometer value
const char* buttonTopic = "testtopic/temp/outTopic/updateButton1883"; // Topic to publish and subscribe for button state

WiFiClient espClient; // Non-secure client
PubSubClient client(espClient);

// Potentiometer and button pins
#define POT_PIN A0
#define BUTTON_PIN D0
#define LED_PIN D1
#define CONNECTION_LED D2 // New LED for connection

bool buttonState = false; // To store the button state
unsigned long lastPotPublish = 0; // Timer for potentiometer publish
unsigned long ledOnTime = 0; // Timer for LED on state
unsigned long buttonPressTime = 0; // Timer for button press
const unsigned long potPublishInterval = 15000; // 15 seconds interval
const unsigned long ledOffDelay = 5000; // 5 seconds delay to turn off LED

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(CONNECTION_LED, !digitalRead(CONNECTION_LED)); // Blink connection LED
    delay(500);
    Serial.print(".");
  }

  digitalWrite(CONNECTION_LED, LOW); // Turn connection LED off initially
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, buttonTopic) == 0) {
    if (length == 1 && payload[0] == '1') {
      digitalWrite(LED_PIN, HIGH); // Turn on LED
      ledOnTime = millis(); // Start timer for LED on state
      Serial.println("LED turned ON");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    digitalWrite(CONNECTION_LED, !digitalRead(CONNECTION_LED)); // Blink connection LED
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      digitalWrite(CONNECTION_LED, HIGH); // Turn connection LED solid when connected
      client.subscribe(buttonTopic); // Subscribe to button topic
      Serial.println("Subscribed to button topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Non-SSL port
  client.setCallback(callback);

  pinMode(BUTTON_PIN, INPUT); // Button with external pull-down resistor // Button with pull-up resistor
  pinMode(LED_PIN, OUTPUT);
  pinMode(CONNECTION_LED, OUTPUT); // Connection LED
  digitalWrite(LED_PIN, LOW); // Ensure LED is off initially
  digitalWrite(CONNECTION_LED, LOW); // Ensure connection LED is off initially
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish potentiometer value every 15 seconds
  unsigned long now = millis();
  if (now - lastPotPublish >= potPublishInterval) {
    lastPotPublish = now;

    // Read the analog value from the potentiometer
    int potValue = analogRead(POT_PIN);

    // Convert potentiometer value to string and publish
    char potValueStr[10];
    snprintf(potValueStr, sizeof(potValueStr), "%d", potValue);
    client.publish(potTopic, potValueStr);
    Serial.print("Published potentiometer value: ");
    Serial.println(potValue);
  }

  // Handle button press
  bool currentButtonState = digitalRead(BUTTON_PIN) == HIGH;
  if (currentButtonState && !buttonState) {
    buttonState = true;
    buttonPressTime = millis();
    client.publish(buttonTopic, "1"); // Publish button pressed
    Serial.println("Button pressed, published 1");
  }

  // Turn off LED after 5 seconds and publish 0
  if (digitalRead(LED_PIN) == HIGH && millis() - ledOnTime >= ledOffDelay) {
    digitalWrite(LED_PIN, LOW); // Turn off LED
    client.publish(buttonTopic, "0"); // Publish LED off
    Serial.println("LED turned OFF after 5 seconds, published 0");
  }

  // Turn off button state after 5 seconds
  if (buttonState && millis() - buttonPressTime >= ledOffDelay) {
    buttonState = false;
    Serial.println("Button state reset after 5 seconds");
  }

  delay(100); // Small delay for stability
}
