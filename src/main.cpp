#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

/* WIFI & MQTT CONFIGURATION */
const char* ssid = "Random Guest";
const char* password = "beourguest";
const char* mqtt_server = "10.112.10.10";


WiFiClient wifiClient;
PubSubClient client(wifiClient);

/* HARDWARE CONFIGURATION */
const int sensorPin = A2;         // Fabric raw signal
const int pixelPin = 13;          // Ring signal pin D13
const int numPixels = 24;         // Number of LEDs in the ring

int restingValue = 0;
int sensorThreshold = 500;        // Adjust this value if light doesn't reach 100%
unsigned long lastPrintTime = 0;   // Timer for serial print
unsigned long lastMsg = 0;         // Timer for MQTT send

Adafruit_NeoPixel ring(numPixels, pixelPin, NEO_GRB + NEO_KHZ800);

void connectWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Wait for connection with timeout (optional, simple while here)
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "FabricController-";
    clientId += String(random(0xffff), HEX);
    
    // Updated credentials: User="mqtt", Password="password"
    if (client.connect(clientId.c_str(), "mqtt", "password")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishData(int touchValue, int touchValue2, int rawValue, int intensityValue) {
  // Send each value as raw bytes (little-endian on ESP32)
  // client.publish("fabric/test/test", (const uint8_t*)&touchValue, sizeof(int));
  //client.publish("fabric/touch/change", (const uint8_t*)&touchValue, sizeof(int));
  client.publish("fabric/touch2/change", (const uint8_t*)&touchValue2, sizeof(int));
  client.publish("fabric/raw/change", (const uint8_t*)&rawValue, sizeof(int));
  client.publish("fabric/intensity/change", (const uint8_t*)&intensityValue, sizeof(int));
}

void setup() {
  Serial.begin(115200);
  delay(2000); 

  // WIFI & MQTT SETUP
  connectWiFi();
  client.setServer(mqtt_server, 1883);

  // NEOPIXEL SETUP
  ring.begin();
  ring.show();                    
  ring.setBrightness(100);        

  // BASELINE CALIBRATION
  Serial.println(">>> CALIBRATING... Do not touch the fabric.");
  long sum = 0;
  for(int i = 0; i < 100; i++) {
    sum += analogRead(sensorPin);
    delay(10);
  }
  restingValue = sum / 100;
  
  Serial.print(">>> Baseline acquired: "); Serial.println(restingValue);
}

void loop() {
  // CONNECTION MANAGEMENT
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // SENSOR READING
  int currentRaw = analogRead(sensorPin);
  int touch = touchRead(T8);
  int touch2 = touchRead(T9);

  // LED LOGIC
  int intensity = map(currentRaw, restingValue, restingValue - sensorThreshold, 0, 100);
  intensity = constrain(intensity, 0, 100);
  if (intensity < 10) intensity = 0; 

  // LED RING RESPONSE
  /* int ledBrightness = map(intensity, 0, 100, 0, 255);

  if (touch == 1){
    for(int i = 0; i < numPixels; i++) {
        ring.setPixelColor(i, ring.Color(
          (255 * ledBrightness) / 255, 
          (180 * ledBrightness) / 255, 
          (50  * ledBrightness) / 255
        ));
    }
    ring.show();
  } else {
      // Turn off if not touched? (optional, leaving as it was)
      // In original code there was no else to turn off, so LEDs remained "frozen"
      // If you want them to turn off when not touching, manage the else or move ring.show out.
      // LEAVING ORIGINAL LOGIC FOR NOW.
  }
  */

  // SEND MQTT DATA (e.g. every 200ms)
   /* 
     Note: sending at every loop (15ms) is TOO MUCH for MQTT. 
     Reduced to 5Hz (200ms) to prevent brownouts/network flooding.
   */
  if (millis() - lastMsg > 200) { 
    lastMsg = millis();
    publishData(touch, touch2, currentRaw, intensity);
  }

  // SERIAL DEBUG (Every 500ms for safety)
  if (millis() - lastPrintTime > 500) {
    lastPrintTime = millis();
    Serial.print("raw:"); Serial.print(currentRaw);
    //Serial.print(" | int:"); Serial.print(intensity); 
    //Serial.print(" | touch: "); Serial.println(touch);
    Serial.print(" | touch2: "); Serial.println(touch2);
  }

  delay(15); 
}