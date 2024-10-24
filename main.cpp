#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>

#define DHTPIN D4     // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11 // DHT 11

// Wi-Fi credentials
const char* ssid = "Hi";
const char* password = "veryhardpassword";

// PHP script URL (base URL without parameters)
const String serverBaseURL = "https://camerondeleeuw.com/updateSensorData.php";

// Pin for the button
const int buttonPin = D1;  // GPIO5 for button

// Variables for temperature, humidity, and time
float temperature = 0;
float humidity = 0;
String currentTime = "";

// Time zone selection
int selectedTimeZone = 4;  // Default to PT

// To track the button state
int buttonState = LOW;
int lastButtonState = LOW;

DHT_Unified dht(DHTPIN, DHTTYPE);

// Function prototypes
int check_switch();
void read_time();
void read_sensors();
void transmit(float temperature, float humidity, String timeReceived);
String urlEncode(String str);
void promptForTimeZone();
String adjustTimeForTimeZone(String utcTime, int timeZoneOffset);

void setup() {
  Serial.begin(9600);
  
  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT sensor initialized");

  // Set up the button pin
  pinMode(buttonPin, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

   // Display the MAC Address
  Serial.print("ESP MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Display the IP Address
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  // Prompt user to select time zone
  promptForTimeZone();

  delay(1000);  // Small delay to stabilize everything
}

void loop() {
  // Check if the button is pressed
  buttonState = check_switch();

  // Only execute when the button is pressed (HIGH)
  if (buttonState == HIGH && lastButtonState == LOW) {
    // Fetch current time from the API
    read_time();

    // Read temperature and humidity
    read_sensors();

    // Send the data to the server if both temperature, humidity, and time are available
    if (temperature != 0 && humidity != 0 && currentTime != "") {
      transmit(temperature, humidity, currentTime);
    }

    // Prevent multiple triggers for the same press
    lastButtonState = HIGH;
  }

  // Reset button state when it's released
  if (buttonState == LOW && lastButtonState == HIGH) {
    lastButtonState = LOW;
  }

  delay(50);  // Short delay for debouncing
}

// Small function to check button
int check_switch(){
  return digitalRead(buttonPin);
}

// Function to fetch the current time from the Time API and format it properly
void read_time() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Skip certificate verification

    HTTPClient http;
    http.begin(client, "https://timeapi.io/api/Time/current/ip?ipAddress=237.71.232.203");

    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response from Time API:");
        Serial.println(payload);

        // Parse JSON response using JsonDocument
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          const char* dateTime = doc["dateTime"];
          if (dateTime != nullptr) {
            currentTime = String(dateTime);
            
            // Format currentTime to match "YYYY-MM-DD HH:MM:SS"
            currentTime.replace("T", " ");  // Replace 'T' with a space
            currentTime = currentTime.substring(0, 19);  // Remove the fractional seconds

            // Adjust for the selected time zone
            currentTime = adjustTimeForTimeZone(currentTime, selectedTimeZone);
            
            Serial.print("Formatted and adjusted time: ");
            Serial.println(currentTime);
          }
        } else {
          Serial.print("Failed to parse JSON: ");
          Serial.println(error.c_str());
        }
      }
    } else {
      Serial.printf("Error in HTTP request: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Wi-Fi is not connected");
  }
}

// Fucntion to read the value from temp/humidity sensor
void read_sensors() {
  // Fetch temperature and humidity data
    sensors_event_t event;

    // Get temperature
    dht.temperature().getEvent(&event);
    if (!isnan(event.temperature)) {
      temperature = event.temperature;
      Serial.print(F("Temperature: "));
      Serial.print(temperature);
      Serial.println(F("°C"));
    } else {
      Serial.println(F("Error reading temperature!"));
    }

    // Get humidity
    dht.humidity().getEvent(&event);
    if (!isnan(event.relative_humidity)) {
      humidity = event.relative_humidity;
      Serial.print(F("Humidity: "));
      Serial.print(humidity);
      Serial.println(F("%"));
    } else {
      Serial.println(F("Error reading humidity!"));
    }
}

// Function to send temperature, humidity, and time_received as URL parameters to the PHP server
void transmit(float temperature, float humidity, String timeReceived) {
  if (WiFi.status() == WL_CONNECTED) {
    // Create a WiFiClientSecure object for HTTPS connections
    WiFiClientSecure client;
    client.setInsecure();  // Skip certificate verification

    HTTPClient http;
    
    // URL-encode the parameters
    String node_name = urlEncode("node_1");
    String tempStr = urlEncode(String(temperature));
    String humidityStr = urlEncode(String(humidity));
    String timeStr = urlEncode(timeReceived);

    // Construct the full URL with parameters
    String fullURL = serverBaseURL + "?node_name=" + node_name + "&temperature=" + tempStr + "&humidity=" + humidityStr + "&time_received=" + timeStr;

    // Print the full URL to Serial Monitor for debugging
    Serial.println("GET request URL:");
    Serial.println(fullURL);

    // Send the GET request
    http.begin(client, fullURL);
    int httpResponseCode = http.GET();  // Send data via GET request

    // Check the response from the server
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from server:");
      Serial.println(response);
    } else {
      Serial.print("Error sending GET request: ");
      Serial.println(httpResponseCode);
    }

    // Close the connection
    http.end();
  } else {
    Serial.println("Wi-Fi not connected");
  }
}

// Prompt the user to select a time zone
void promptForTimeZone() {
  Serial.println("--> Select Your Time Zone (The default time is PT): ");
  Serial.println("1 - Eastern Time (ET)");
  Serial.println("2 - Central Time (CT)");
  Serial.println("3 - Mountain Time (MT)");
  Serial.println("4 - Pacific Time (PT)");
  Serial.println("5 - Alaska Time (AKT)");
  Serial.println("6 - Hawaii-Aleutian Time (HAT)");
  Serial.println("7 - Atlantic Time (AT)");
  Serial.println("Enter a number (1-7): ");

  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {  // Wait for 10 seconds for user input
    if (Serial.available() > 0) {
      int input = Serial.parseInt();
      if (input >= 1 && input <= 7) {
        selectedTimeZone = input;
        Serial.print("Selected Time Zone: ");
        Serial.println(selectedTimeZone);
        return;
      }
    }
  }

  // If no input is provided within 10 seconds, default to PT
  Serial.println("No input provided. Defaulting to Pacific Time (PT).");
  selectedTimeZone = 4;
}

// Function to adjust the time based on the selected time zone
String adjustTimeForTimeZone(String utcTime, int timeZone) {
  // Time zone offsets relative to UTC
  int standardTimeZoneOffsets[] = {0, -5, -6, -7, -8};  // ET, CT, MT, PT, AKT (no DST)
  int dstTimeZoneOffsets[] = {0, -4, -5, -6, -7};       // ET, CT, MT, PT, AKT (with DST)

  // Parse the utcTime (format: "YYYY-MM-DD HH:MM:SS") to extract date and time components
  int year = utcTime.substring(0, 4).toInt();
  int month = utcTime.substring(5, 7).toInt();
  int day = utcTime.substring(8, 10).toInt();
  int hour = utcTime.substring(11, 13).toInt();
  int minute = utcTime.substring(14, 16).toInt();
  int second = utcTime.substring(17, 19).toInt();

  // Determine if it's DST (simple rule: DST is from the second Sunday in March to the first Sunday in November)
  bool isDST = false;
  if ((month > 3 && month < 11) || (month == 3 && day >= 14) || (month == 11 && day < 7)) {
    isDST = true;
  }

  // Adjust the hour based on the selected time zone and whether it's DST
  if (isDST) {
    hour += dstTimeZoneOffsets[timeZone];
  } else {
    hour += standardTimeZoneOffsets[timeZone];
  }

  // Handle wrapping around midnight
  if (hour >= 24) {
    hour -= 24;
    day += 1;  // Handle day wraparound, adjust for month/year if needed
  } else if (hour < 0) {
    hour += 24;
    day -= 1;  // Handle going back to the previous day
  }

  // Return the adjusted time as a string
  char adjustedTime[20];
  snprintf(adjustedTime, sizeof(adjustedTime), "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
  return String(adjustedTime);
}


// Helper function to URL encode strings
String urlEncode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '%';  // Encode space as %20
      encodedString += '2';
      encodedString += '0';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      code0 = ((c >> 4) & 0xf) + '0';
      if (((c >> 4) & 0xf) > 9) {
        code0 = ((c >> 4) & 0xf) - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();  // Yield to avoid WDT reset
  }
  return encodedString;
}
