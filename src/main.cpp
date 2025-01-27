#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#define LIMIT_SWITCH_PIN 23

#define BOARD_ID 2

uint8_t broadcastAddress[] = {0x08, 0xd1, 0xf9, 0x72, 0xaf, 0x1c};

typedef struct struct_message {
    int id;
    int check_value;
    int readingId;
} struct_message;

esp_now_peer_info_t peerInfo;


struct_message myData;

unsigned long previousMillis = 0;   
const long interval = 10000;        

unsigned int readingId = 0;


constexpr char WIFI_SSID[] = "RNXG_WEB";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
 Serial.println(myData.check_value);
  
}
 
void setup() {
 
  Serial.begin(115200);
  pinMode(LIMIT_SWITCH_PIN,INPUT_PULLUP);
 

  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); 
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

 
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
    

  if(digitalRead(LIMIT_SWITCH_PIN)==LOW){

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    myData.id = BOARD_ID;
    myData.check_value = 50;

    myData.readingId = readingId++;
     
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  } 
  }
}