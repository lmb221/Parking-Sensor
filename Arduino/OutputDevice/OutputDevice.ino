// YWROBOT 
// Compatible with the Arduino IDE 1.0
// Library version: 1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string.h>
#include <ArduinoJson.h>
#include <esp_eap_client.h>
#include <esp_wifi.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 20x4 display

using namespace std;

int pirPin = 19;
int pirStat = 0;
int counter_var = 0;
const char WIFI_SSID[] = "My Son Alexander";
const char WIFI_PASSWORD[] = "P455word!";
// const char WIFI_SSID[] = "lehigh-guest";
// String POST = "http://192.168.4.84:5000/post";
String GET = "http://192.168.4.84:5000/get";
// String POST = "http://172.31.64.100:5000/post";
// String GET = "http://172.31.64.100:5000/get";

// Use 3 elements since you are sending 3 sensor values.
int16_t tfDist[3] = {0, 0, 0};
int16_t tDist[3] = {0, 0, 0};

void processData(String payload) {
  // Arrays to store keys based on their values
  String s_open[10], s_closed[10], h_open[10], h_closed[10];
  int s_open_count = 0, s_closed_count = 0, h_open_count = 0, h_closed_count = 0;

  // Parse the JSON payload
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Iterate through the JSON object
  for (JsonPair kv : doc.as<JsonObject>()) {
    String key = kv.key().c_str();
    String value = kv.value().as<String>();

    // Skip non-spot keys
    if (!key.startsWith("spot")) continue;

    // Categorize keys based on their values
    if (value.endsWith("S")) {
      if (value.startsWith("1")) {
        s_open[s_open_count++] = key;
      } else {
        s_closed[s_closed_count++] = key;
      }
    } else if (value.endsWith("H")) {
      if (value.startsWith("1")) {
        h_open[h_open_count++] = key;
      } else {
        h_closed[h_closed_count++] = key;
      }
    }
  }

  // Sort the _open arrays
  std::sort(s_open, s_open + s_open_count);
  std::sort(h_open, h_open + h_open_count);

  // Display the first value in s_open and h_open on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Std:    | Hand:");

  lcd.setCursor(0, 1);
  if (s_open_count > 0) {
    lcd.print(s_open[0]);
  } else {
    lcd.print("None");
  }

  lcd.setCursor(10, 1); // Move to the right side of the LCD
  if (h_open_count > 0) {
    lcd.print(h_open[0]);
  } else {
    lcd.print("None");
  }
}

String receive() {
  HTTPClient http;
  http.begin(GET);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  String payload;

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.printf("[HTTP] GET code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  // Process the received data
  processData(payload);

  return payload;
}

void setup() {
  Serial.begin(115200); 
  pinMode(pirPin, INPUT);
  // Set WiFi to station mode and disconnect from any previous AP.
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  // Lehigh Wifi
  WiFi.begin(WIFI_SSID);
  //normal wifi 
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  delay(100);

  Serial.println("Setup done");

  lcd.init();  // initialize the LCD
  lcd.backlight();
  // Example initial display message:
  lcd.setCursor(0, 0);
  lcd.print("Never fear,");
}

void loop() {
  // Receive data from the server and process it
  receive();
  // Wait before scanning again
  delay(5000);
}
