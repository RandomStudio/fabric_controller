#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

unsigned long lastPrintTime = 0;
unsigned long lastMsg = 0;

bool on = false;
bool curtainOn = false;

void setup() {
  Serial.begin(115200);
  delay(2000); 
}

enum CurtainState {
  TOUCH,
  APPROACH,
  NONE
};

const char* getCurtainName(CurtainState state) {
  switch(state) {
    case TOUCH: return "TOUCH";
    case APPROACH: return "APPROACH";
    case NONE: return "NONE";
  }
  return "UNKNOWN";
}

CurtainState currentState = NONE;
CurtainState lastAnnouncedState = NONE;

void loop() { 
  int touch2 = touchRead(T6);

  touch2 = unsigned((touch2 -  40000) / 100);
  touch2 = constrain(touch2, 0, 255);

  if(touch2 > 120) {
    currentState = TOUCH;
  } else {
    currentState = NONE;
  }


  if (currentState != lastAnnouncedState) {
    Serial.print("CURTAIN="); Serial.println(getCurtainName(currentState));
    lastAnnouncedState = currentState;
  }
  Serial.print(" VALUE="); Serial.println(touch2);

  delay(10); 
}
