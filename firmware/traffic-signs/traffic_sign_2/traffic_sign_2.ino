#include <Arduino.h>
#include <SPI.h>
#include "Config.h"
#include "TrafficAlertReceiver.h"

// Codul a fost simplificat - eliminare comunicare între semne

// Flag care reține dacă semnul este blocat pe mesaj de ACCIDENT
bool accidentLatched = false;

#include "DisplayManager.h"
#include "BleManager.h"
#include "ESP32_NOW.h"
#include <WiFi.h>
#include <esp_wifi.h>


// Declarăm extern variabilele care vor fi folosite în clase
extern DisplayManager epaperDisplay;
extern BleManager bleManager;



// Variabile pentru controlul secvenței de afișare
bool welcomeShown = false;
unsigned long welcomeStartTime = 0;
const unsigned long WELCOME_DURATION = 5000; // 5 secunde

// Manager BLE pentru comunicare cu aplicația Android
BleManager bleManager(&epaperDisplay);

// TrafficAlertReceiver pentru recepționarea mesajelor de accident
TrafficAlertReceiver alertReceiver(&epaperDisplay);


void setup() {
  // Inițializare serial pentru debugging
  Serial.begin(115200);

  delay(2000);
  Serial.println("Adaptive Traffic System - Pornire");

  // 1. Eliberare memorie Bluetooth Classic (doar BLE necesar)
  Serial.println("1. Eliberare memorie Bluetooth Classic");
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);


  Serial.println("2. Inițializare BLE Manager");
  bleManager.init();
  Serial.println("   BLE inițializat cu succes");



  Serial.println("Inițializare TrafficAlertReceiver...");
  alertReceiver.init();


  epaperDisplay.init();
  Serial.println("Display inițializat");

  epaperDisplay.welcomeMessage();

  // Inițializarea cronometrului pentru tranziție
  welcomeStartTime = millis();
  welcomeShown = true;

  Serial.println("Inițializare completă. Sistem pregătit pentru comenzi BLE și mesaje ESP-NOW broadcast.");
  Serial.println("Aștept mesaje broadcast de la orice dispozitiv...");
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


  delay(100);
}
