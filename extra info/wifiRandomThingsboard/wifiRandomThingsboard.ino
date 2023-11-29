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
int updateDelay = 100;        // Initial update delay.
int lastUpdate  = 0;            // Time of last update.
bool subscribed = false;        // Set to true if application is subscribed for the RPC messages.

float temperature = 27.5;
float pH = 5;
int RPM = 750;

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response set_pH(const RPC_Data &data) {
  Serial.print("set pH: ");

  // Process data
  updateDelay = data;

  // Serial.print("Set new delay: ");
  // Serial.println(updateDelay);
  int big = (int) updateDelay / 10;
  int small = (int) updateDelay % 10;

  Serial.print(big);
  Serial.print('.');
  Serial.print(small);

  Serial.println(' ');

  pH = big + (small * 0.1);

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response get_pH(const RPC_Data &data) {
  Serial.println("get pH");

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response set_Temp(const RPC_Data &data) {
  Serial.print("set Temp: ");

  // Process data
  updateDelay = data;

  // Serial.print("Set new delay: ");
  // Serial.println(updateDelay);
  int big = (int) updateDelay / 10;
  int small = (int) updateDelay % 10;

  Serial.print(big);
  Serial.print('.');
  Serial.print(small);

  Serial.println(' ');

  temperature = big + (small * 0.1);

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response get_Temp(const RPC_Data &data) {
  Serial.println("get Temp");

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response set_RPM(const RPC_Data &data) {
  Serial.print("set RPM: ");

  // Process data
  updateDelay = data;

  RPM = updateDelay;

  // Serial.print("Set new delay: ");
  Serial.print(updateDelay);

  Serial.println(' ');

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response get_RPM(const RPC_Data &data) {
  Serial.println("get RPM");

  return RPC_Response(NULL, updateDelay);
}


// RPC handlers
RPC_Callback callbacks[] = {
  { "getpHValue",         get_pH},
  { "setpHValue",         set_pH},
  { "getTempValue",         get_Temp},
  { "setTempValue",         set_Temp},
  { "getRPMValue",         get_RPM},
  { "setRPMValue",         set_RPM},
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
      // return;
    }
  }

  // Subscribe for RPC, if needed
  if (!subscribed) {
    // Serial.println("Subscribing for RPC...");

    // Perform a subscription. All consequent data processing will happen in
    // callbacks as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks))) {
      Serial.println("Failed to subscribe for RPC");
      // return;
    }

    // Serial.println("Subscribe done");
    subscribed = true;
  }

  if (now > lastUpdate + updateDelay) {
    // r = (float)random(1000)/1000.0;
    // loopCounter++;

    // Serial.print("Sending data...[");  
    // Serial.print(loopCounter);
    // Serial.print("]: ");
    // Serial.println(r);

    // temperature += 10.1;
    // pH += 0.1;
    // RPM += 10;
    
    // Uploads new telemetry to ThingsBoard using MQTT. 
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api 
    // for more details
    // tb.sendTelemetryInt("count", loopCounter);    
    // tb.sendTelemetryFloat("randomVal", r);
    tb.sendTelemetryFloat("temperature", temperature);
    tb.sendTelemetryFloat("pH", pH);
    tb.sendTelemetryInt("RPM", RPM);
    lastUpdate += updateDelay;
  }

  // Process messages
  tb.loop();

}
