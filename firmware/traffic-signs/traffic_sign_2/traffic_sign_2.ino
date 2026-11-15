/**
 * Adaptive Traffic System - Traffic Sign 2
 * 
 * Componentă a proiectului SmartVehicleEcosystem
 * Autor: Bulgariu (Mihăilă) Elena-Iuliana
 * 
 * Hardware: MH ET Live 2.13 inch e-paper display cu XIAO ESP32C3
 * 
 * Actualizări:
 * - Adăugare server BLE pentru comunicare cu aplicația Android
 * - Implementare ESP-Now pentru sincronizare între semne
 */

// Include modulele necesare
#include "DisplayManager.h"
#include "BleManager.h"
#include "TrafficAlertReceiver.h"

// Variabile pentru controlul secvenței de afișare
bool welcomeShown = false;
unsigned long welcomeStartTime = 0;
const unsigned long WELCOME_DURATION = 5000; // 5 secunde
bool syncDisplayShown = false;
const unsigned long SYNC_CHECK_INTERVAL = 1000; // 1 secundă pentru verificarea sincronizării
unsigned long lastSyncCheckTime = 0;
bool syncConfirmed = false;

// Managers pentru diferite componente
// epaperDisplay este definită în DisplayManager.cpp
extern DisplayManager epaperDisplay;

// Manager BLE pentru comunicare cu aplicația Android
BleManager bleManager(&epaperDisplay);

// Manager pentru alertele de trafic de la Elysium RC
TrafficAlertReceiver trafficAlert(&epaperDisplay);

// Adresa MAC a lui Elysium RC - va trebui actualizată cu adresa reală
uint8_t elysiumMacAddress[] = {0x24, 0x6F, 0x28, 0x00, 0x00, 0x00};

void setup() {
  // Inițializare display manager - include inițializarea hardware
  epaperDisplay.init();
  
  // Inițializare BLE Manager
  bleManager.init();
  
  // Inițializare TrafficAlert Receiver
  trafficAlert.init();
  
  // Configurare adresă MAC pentru Elysium RC
  trafficAlert.setElysiumMac(elysiumMacAddress);
  
  // Afișarea mesajului de bun venit
  epaperDisplay.welcomeMessage();
  
  // Inițializarea cronometrului pentru tranziție
  welcomeStartTime = millis();
  welcomeShown = true;
}

void loop() {
  // Proces de actualizare a afișajului după timpul de bun venit
  if (welcomeShown && (millis() - welcomeStartTime > WELCOME_DURATION)) {
    // Afișăm semnul YIELD (CEDEAZĂ TRECEREA)
    epaperDisplay.showTrafficSign("YIELD");
    welcomeShown = false;
    
    // Trimitem un status update prin BLE
    bleManager.sendStatusUpdate("Ready to receive commands");
  }
  
  // Delay mic pentru a nu supraîncărca procesorul
  delay(50);
}
