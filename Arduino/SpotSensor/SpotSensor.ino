#include <Wire.h>         // I2C library
#include <TFLI2C.h>       // LiDAR library
#include "BlinkM_funcs.h" // LED library
#include <vector>
#include <WiFi.h>
#include <HTTPClient.h>


#define uS_IN_S 1000000     // microseconds in a second
#define TIME_SLEEP  5       // Seconds to sleep for
#define PARKED_DISTANCE 20  // Distance to be considered a "parked car" in centimeters
#define LED_BRIGHTNESS 0x10 // LED brightness


#define NUM_SPOTS 3


/* WiFi */
const char WIFI_SSID[] = "My Son Alexander";
const char WIFI_PASSWORD[] = "P455word!";
String HOST_NAME = "http://192.168.4.96:5000/submit";


TFLI2C tflI2C; // LiDAR object


/* LiDAR parameters */
int16_t tfDist[NUM_SPOTS]; // Holds LiDAR distances in centimeters
int16_t tfAddr[] = {0x10, 0x11, 0x12}; // Set LiDAR I2C address // default TFL_DEF_ADR


byte blinkm_addr[] = {0x0a, 0x0b, 0x09};


// LED script to show red. This allows LED operation even when microcontroller is powered down.
blinkm_script_line red_script[] = {
  { 1, {'c', LED_BRIGHTNESS, 0x00, 0x00}}
};




// LED script to show green. This allows LED operation even when microcontroller is powered down.
blinkm_script_line green_script[] = {
  { 1, {'c', 0x00, LED_BRIGHTNESS, 0x00}}
};


byte script_id = 0;   // Writing to script 0 on blinkM EEPROM
byte script_len = 1;  // Number of lines in script
byte script_rep = 20; // Number of repeats for script


// Update LED's based on sensor data
void update_leds() {
    for (int i = 0; i < NUM_SPOTS; i++) {
        byte rc = BlinkM_getAddress(blinkm_addr[i]);
        if (rc == -1) {
            Serial.println("\r\nno response");
        } else if (rc != blinkm_addr[i]) {
            Serial.println("\r\naddr mismatch");
        }

        if (tfDist[i] > PARKED_DISTANCE) {
            Serial.println("writing GREEN...\n");
            BlinkM_fadeToRGB(blinkm_addr[i], 0x00, 0x10, 0x00);
            BlinkM_writeScript(blinkm_addr[i], script_id, script_len, script_rep, green_script); // Light green
        } else {
            Serial.println("writing RED...\n");
            BlinkM_fadeToRGB(blinkm_addr[i], 0xff, 0x00, 0x00);
            BlinkM_writeScript(blinkm_addr[i], script_id, script_len, script_rep, red_script); // Light red
        }
    }
}


// Poll LiDAR sensors
void poll_sensors(){
  for(int i = 0; i < NUM_SPOTS; i++){
    if ( tflI2C.getData(tfDist[i], tfAddr[i])){
      Serial.print("Dist: ");
      Serial.println(tfDist[i]);
      Serial.print("\n");
    }else{
      Serial.print("Error: ");
      tflI2C.printStatus();
      Serial.print("\n");
    }
  }
}


// Send parking lot state data to web backend
void send_data(){


  // Construct and send JSON
  String queryString = "{\"id\": 0";
  String data_send[NUM_SPOTS];
  String names[3] = {"spot2", "spot0", "spot1"};
  for(int i=0; i < NUM_SPOTS; i++){
    queryString += ", \"";
    queryString += names[i];
    queryString += "\": ";
    queryString += (tfDist[i] > PARKED_DISTANCE) ? 0 : 1;
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


void setup() {


  Serial.begin(115200);
  Wire.begin();
  BlinkM_begin(); // init BlinkM funcs
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


  for(int i = 0; i < NUM_SPOTS; i++){
    BlinkM_stopScript(blinkm_addr[i]);
  }


  // Set timer wake up event
  esp_sleep_enable_timer_wakeup(TIME_SLEEP * uS_IN_S);
 
  // Poll sensors
  poll_sensors();


  // Update LEDs according to sensor values
  update_leds();


  // Transmit data to web
  send_data();


  esp_deep_sleep_start(); // Put MC to sleep


}


void loop() {


}

