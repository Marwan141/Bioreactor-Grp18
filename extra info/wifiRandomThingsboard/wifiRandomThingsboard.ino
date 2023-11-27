#include <WiFi.h>             // WiFi control for ESP32
#include <ThingsBoard.h>      // ThingsBoard SDK
#include "esp_wpa2.h"   //wpa2 library for connections to Enterprise networks

//#define EAP_IDENTITY "insert your user name@ucl.ac.uk here"                
//#define EAP_PASSWORD "insert your password here"
//const char* ssid = "eduroam";

// WiFi
//
#define WIFI_AP_NAME        "Raghavs-iPhone"
#define WIFI_PASSWORD       "Raghav123"


// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
//


#define TOKEN               "rcnig1c5ekp8xk2oq5zi"
#define THINGSBOARD_SERVER  "thingsboard.cloud"

WiFiClient espClient;           // Initialize ThingsBoard client
ThingsBoard tb(espClient);      // Initialize ThingsBoard instance
int status = WL_IDLE_STATUS;    // the Wifi radio's status

int quant = 1;                  // Main application loop delay
int updateDelay = 10000;        // Initial update delay.
int lastUpdate  = 0;            // Time of last update.
bool subscribed = false;        // Set to true if application is subscribed for the RPC messages.


// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processDelayChange(const RPC_Data &data) {
  Serial.println("Received the set delay RPC method");

  // Process data
  updateDelay = data;

  Serial.print("Set new delay: ");
  Serial.println(updateDelay);

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processGetDelay(const RPC_Data &data) {
  Serial.println("Received the get value method");

  return RPC_Response(NULL, updateDelay);
}

// RPC handlers
RPC_Callback callbacks[] = {
  { "getValue",         processGetDelay },
  { "setValue",         processDelayChange },
};


void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}


void setup() {
  Serial.begin(115200);
  // tft.init();
  // tft.setRotation(1);

  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  InitWiFi();
}


void loop() {
  float  r;                            // Random value
  static uint16_t loopCounter = 0;     // Display iterations
  long now = millis();

  delay(quant);

  // Reconnect to WiFi, if needed
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
    return;
  }

  // Reconnect to ThingsBoard, if needed
  if (!tb.connected()) {
    subscribed = false;

    // Connect to the ThingsBoard
    // Serial.print("Connecting to: ");
    // Serial.print(THINGSBOARD_SERVER);
    // Serial.print(" with token ");
    // Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }  

  // Subscribe for RPC, if needed
  if (!subscribed) {
    // Serial.println("Subscribing for RPC...");

    // Perform a subscription. All consequent data processing will happen in
    // callbacks as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks))) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }

    // Serial.println("Subscribe done");
    subscribed = true;
  }

  if (now > lastUpdate + updateDelay) {
    r = (float)random(1000)/1000.0;
    loopCounter++;

    // Serial.print("Sending data...[");  
    // Serial.print(loopCounter);
    // Serial.print("]: ");
    // Serial.println(r);
    
    // Uploads new telemetry to ThingsBoard using MQTT. 
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api 
    // for more details
    tb.sendTelemetryInt("count", loopCounter);    
    tb.sendTelemetryFloat("randomVal", r);
    
    lastUpdate += updateDelay;
  }

  // Process messages
  tb.loop();

}
