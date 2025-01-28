#include<Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>

// Replace with your network credentials (STATION)
const char* ssid = "RNXG_WEB";
const char* password = "123456789";

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message { 
  int id;
  int check_value;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;

// JSON document for board readings
ArduinoJson::StaticJsonDocument<200> doc;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// Callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len) { 
  // Copies the sender MAC address to a string

  Serial.println(incomingReadings.check_value);

  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  // Populate JSON document
  doc["id"] = incomingReadings.id;
  doc["check"] = incomingReadings.check_value;
  doc["readingId"] = incomingReadings.readingId;
  String jsonString;
  serializeJson(doc, jsonString);
  events.send(jsonString.c_str(), "new_readings", millis());

}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP-NOW Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {font-size: 1.2rem;}
    body {margin: 0;}
    .topnav {overflow: hidden; background-color:rgb(5, 67, 175); color: white; font-size: 1.7rem; padding: 10px;}
    .content {padding: 20px;}
    .card {background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); padding: 20px; border-radius: 8px;}
    .cards {display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); max-width: 1000px; margin: 0 auto;}
    .reading {font-size: 2rem; color: #333;}
    .packet {color: #555;}
  </style>
</head>
<body>
  <div class="topnav">
    <h3>PATH OF PRIMES SCOREBOARD</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <h4>Checkpoint 1</h4>
        <p>Points: <span class="reading" id="t1">--</span></p>
      </div>

      <div class="card">
        <h4>Checkpoint 2</h4>
        <p>POINTS: <span class="reading" id="t2">--</span></p>
      </div>

      <div class="card">
        <h4>Checkpoint 3</h4>
        <p>POINTS: <span class="reading" id="t3">--</span></p>
      </div>

      <div class="card">
        <h4>Checkpoint 4</h4>
        <p>POINTS: <span class="reading" id="t4">--</span></p>

      </div>
      <div class="card">
        <h4>Checkpoint 5</h4>
        <p>POINTS: <span class="reading" id="t5">--</span></p>
      </div>

    </div>
  </div>
  <script>
    if (!!window.EventSource) {
      var source = new EventSource('/events');

      source.addEventListener('open', function(e) {
        console.log("Events Connected");
      }, false);

      source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);

      source.addEventListener('new_readings', function(e) {
        console.log("new_readings", e.data);
        var obj = JSON.parse(e.data);
        var checkpointId = obj.id;
        if (checkpointId >= 1 && checkpointId <= 5) {
          
          document.getElementById("t" + checkpointId).innerHTML = obj.check.toFixed(2);
          
        }
      }, false);
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);

  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for receive callback
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient* client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start the server
  server.begin();
}

void loop() {
  // Ping clients every 5 seconds
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
  }
}