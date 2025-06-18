/**
 * Adaptive Traffic System - E-Paper Display
 * 
 * Componentă a proiectului SmartVehicleEcosystem
 * Autor: Bulgariu (Mihăilă) Elena-Iuliana & Mihăilă Bogdan-Iulian
 * 
 * Hardware: MH ET Live 2.13 inch e-paper display cu XIAO ESP32C3
 * 
 * Actualizare: Adăugare server BLE pentru comunicare cu aplicația Android
 */

// Include modulele necesare
#include "DisplayManager.h"
#include "BleManager.h"

// Variabile pentru controlul secvenței de afișare
bool welcomeShown = false;
unsigned long welcomeStartTime = 0;
const unsigned long WELCOME_DURATION = 5000; // 5 secunde

// Manager BLE pentru comunicare cu aplicația Android
BleManager bleManager(&epaperDisplay);

void setup() {
  // Inițializare display manager - include inițializarea hardware și Serial
  epaperDisplay.init();
  
  // Inițializare BLE Manager
  bleManager.init();
  Serial.println("Server BLE inițializat și gata pentru conexiuni");
  
  // Afișarea mesajului de bun venit
  epaperDisplay.welcomeMessage();
  
  // Inițializarea cronometrului pentru tranziție
  welcomeStartTime = millis();
  welcomeShown = true;
}

void loop() {
  // Dacă s-a afișat ecranul de bun venit și au trecut cele 5 secunde
  if (welcomeShown && (millis() - welcomeStartTime > WELCOME_DURATION)) {
    // Afișăm semnul STOP și punem display-ul în hibernare (inclus în metodă)
    epaperDisplay.showTrafficSign("STOP");
    
    // Resetăm flag-ul pentru a nu mai intra în această condiție
    welcomeShown = false;
    
    // Trimitem un status update prin BLE
    bleManager.sendStatusUpdate("Ready to receive commands");
  }
  
  // Delay mic pentru a nu supraîncărca procesorul
  delay(100);
}
