#include "TrafficSigns.h"
#include "../display/DisplayManager.h"

// Inițializare obiect global
TrafficSigns trafficSigns;

TrafficSigns::TrafficSigns() : display(epaperDisplay) {
}

void TrafficSigns::showStop() {
  Serial.println(F("Afișare semn STOP"));
  display.showStopSign();
}

void TrafficSigns::showYield() {
  Serial.println(F("Funcționalitate semn YIELD - implementare viitoare"));
  // Implementare viitoare
}

void TrafficSigns::showSpeedLimit(int limit) {
  Serial.println(F("Funcționalitate semn Speed Limit - implementare viitoare"));
  // Implementare viitoare
}

void TrafficSigns::testSignDisplay() {
  showStop();
}
