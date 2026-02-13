#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

/* WIFI & MQTT CONFIGURATION */
const char* ssid = "Random Guest";
const char* password = "beourguest";
const char* mqtt_server = "10.112.10.10";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long lastPrintTime = 0;
unsigned long lastMsg = 0;

bool on = false;
bool curtainOn = false;

void connectWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "FabricController-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), "mqtt", "password")) {
      Serial.println("connected");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); 

  connectWiFi();
  client.setServer(mqtt_server, 1883);
}

bool lastSentButton = false;
int lastSentApproach = 0;
bool lastSentCurtain = false;

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();

  int touch1 = touchRead(T2); //white
  int touch2 = touchRead(T6);  //grey
  int touch3 = touchRead(T9); //purple

  touch1 = unsigned((touch1 - 54000) / 100);
  touch1 = constrain(touch1, 0, 255);

  touch2 = unsigned((touch2 -  40000) / 100);
  touch2 = constrain(touch2, 0, 255);

   if(touch2 > 120) {
    curtainOn = true;
  }
  else {
    curtainOn = false;
  }

  if(touch3 > 65000) {
    on = true;
  }
  else {
    on = false;
  }

  if (millis() - lastMsg > 150) { 
    lastMsg = millis();
    if (touch1 != lastSentApproach) {
      client.publish("fabric_approach/touch/change", (const uint8_t*)&touch1, sizeof(unsigned int));
      lastSentApproach = touch1;
    }

    if (curtainOn != lastSentCurtain) {
      client.publish("fabric_curtain/touch/change", (const uint8_t*)&curtainOn, sizeof(bool));
      lastSentCurtain = curtainOn;
    }

    if (on != lastSentButton) {
      client.publish("fabric_button/touch/change", (const uint8_t*)&on, sizeof(bool));
      lastSentButton = on;
    }
  }

  if (millis() - lastPrintTime > 500) {
    lastPrintTime = millis();
    Serial.print("Button:"); Serial.print(touch3);
    Serial.print(" | ON:"); Serial.print(on);      
    Serial.print(" | Curtain:"); Serial.print(touch2);
    Serial.print(" | Curtain ON:"); Serial.print(curtainOn);
    Serial.print(" | Approach:"); Serial.println(touch1);
  }

  delay(10); 
}
