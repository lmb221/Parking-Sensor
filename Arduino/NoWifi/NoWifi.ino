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
const char WIFI_SSID[] = "My Son Alexander";
const char WIFI_PASSWORD[] = "P455word!";
// const char WIFI_SSID[] = "lehigh";
// const char WIFI_USERNAME[] = "lmb221";
// const char WIFI_PASSWORD[] = "SylviusLiamathieu626777";
String HOST_NAME = "http://192.168.137.64:5000/submit";

// Use 3 elements since you are sending 3 sensor values.
int16_t tfDist[3] = {0, -2, -1};

void send_data() {
  // Construct JSON to send. (e.g., {"id": 0, "spot0": 1, "spot1": 1, "spot2": 1})
  String queryString = "{\"id\": 0";
  for (int i = 0; i < 3; i++) {
    queryString += ", \"spot";
    queryString += i;
    queryString += "\": ";
    // Example conversion: if tfDist[i] equals 1 then send 0; otherwise, send 1.
    queryString += (tfDist[i] == 1) ? "0" : "1";
  }
  queryString += "}";

  Serial.println("Printing: " + queryString);

  HTTPClient http;
  http.begin(HOST_NAME);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(queryString);

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.printf("[HTTP] POST code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

String receive() {
  HTTPClient http;
  http.begin(HOST_NAME);
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

  // Use ArduinoJson to parse the JSON payload.
  // Adjust the document size based on your expected JSON.
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "JSON parse error";
  }

  // Assuming the JSON object has keys "id", "spot0", "spot1", "spot2"
  String id   = doc["id"]   | "N/A";
  String s1   = doc["spot0"] | "N/A";
  String s2   = doc["spot1"] | "N/A";
  String s3   = doc["spot2"] | "N/A";

  String s = "id: " + id + " s1: " + s1 + " s2: " + s2 + " s3: " + s3;
  return s;
}

void setup() {
  Serial.begin(115200); 
  pinMode(pirPin, INPUT);
  // Set WiFi to station mode and disconnect from any previous AP.
  // WiFi.disconnect();
  // WiFi.mode(WIFI_STA);
  // Lehigh Wifi
  // esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
  // ESP_LOGI(TAG, "Setting WiFi configuration SSID %s... ", wifi_config.sta.ssid);
  // ESP_ERROR_CHECK(esp_eap_client_set_mode(EAP_METHOD_PEAP));
  // ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
  // ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
  // ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD)));
  // ESP_ERROR_CHECK(esp_eap_client_enable());
  // //normal wifi 
  // WiFi.begin(WIFI_SSID);


  // Serial.println("Connecting");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("");
  // Serial.print("Connected to WiFi network with IP Address: ");
  // Serial.println(WiFi.localIP());
  // delay(100);

  // Serial.println("Setup done");

  lcd.init();  // initialize the LCD
  lcd.backlight();
  // Example initial display message:
  lcd.setCursor(0, 0);
  lcd.print("Never fear,");
  lcd.setCursor(0, 1);
  lcd.print("Captain Piss");
  lcd.setCursor(0, 2);
  lcd.print("is here!");
}

void loop() {
  // // send_data();
  // // String s = receive();
  
  // // Update sensor data for the next iteration.
  // tfDist[0] += 1;
  // tfDist[1] += 1;  // Changed from tfDist[3] to tfDist[1] to stay within bounds.
  // tfDist[2] += 1;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  // lcd.print(s);

  // Uncomment if you want to display based on PIR sensor status.
  pirStat = digitalRead(pirPin);
  if (pirStat == HIGH) { 
    lcd.print("Motion Detected");
    // delay(500);
  } else {
    lcd.print("No motion");
  }
  
  // // Wait before scanning again.
  delay(100);
}
