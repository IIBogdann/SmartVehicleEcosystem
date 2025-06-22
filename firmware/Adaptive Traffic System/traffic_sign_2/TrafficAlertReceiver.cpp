#include "TrafficAlertReceiver.h"

// Inițializare pointer static
TrafficAlertReceiver* TrafficAlertReceiver::_instance = nullptr;

TrafficAlertReceiver::TrafficAlertReceiver(DisplayManager* displayManager) 
  : _displayManager(displayManager), _isInitialized(false) {
  // Setăm instanța statică pentru a fi accesată din callback-uri
  _instance = this;
  
  // Inițializăm adresa MAC a lui Elysium cu zero până când este setată explicit
  memset(_elysiumMacAddress, 0, 6);
}

void TrafficAlertReceiver::init() {
  // Configurare WiFi în modul Station
  WiFi.mode(WIFI_STA);
  
  // Inițializare ESP-Now
  if (esp_now_init() != ESP_OK) {
    // În caz de eroare, se reîncearcă după restart
    ESP.restart();
    return;
  }
  
  // Înregistrăm callback-ul pentru recepție
  esp_now_register_recv_cb(onDataReceived);
  
  _isInitialized = true;
}

void TrafficAlertReceiver::setElysiumMac(const uint8_t* mac) {
  memcpy(_elysiumMacAddress, mac, 6);
  
  // Creare informație peer pentru Elysium RC
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, _elysiumMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  // Adăugare peer pentru a permite recepția
  esp_now_add_peer(&peerInfo);
}

void TrafficAlertReceiver::onDataReceived(const esp_now_recv_info* info, const uint8_t* data, int data_len) {
  // Verifică dacă avem o instanță validă
  if (!_instance) return;
  
  // Verifică dacă mesajul este de la Elysium
  if (memcmp(info->src_addr, _instance->_elysiumMacAddress, 6) == 0) {
    // Verifică dacă avem suficiente date pentru mesajul nostru
    if (data_len >= sizeof(ElysiumMessage)) {
      // Conversia datelor primite la structura noastră
      ElysiumMessage message;
      memcpy(&message, data, sizeof(ElysiumMessage));
      
      // Procesarea mesajului
      _instance->processMessage(message);
    }
  }
}

void TrafficAlertReceiver::processMessage(const ElysiumMessage& message) {
  // Dacă nu avem un display manager, nu putem face nimic
  if (!_displayManager) return;
  
  // În funcție de tipul evenimentului, actualizăm afișajul
  switch (message.eventType) {
    case EVENT_ACCIDENT:
      // Afișează semn de accident
      _displayManager->showTrafficSign("ACCIDENT");
      break;
      
    case EVENT_OBSTACLE:
      // Afișează semn de obstacol
      _displayManager->showTrafficSign("OBSTACOL");
      break;
      
    case EVENT_EMERGENCY:
      // Afișează semn de urgență
      _displayManager->showTrafficSign("URGENTA");
      break;
      
    case EVENT_NORMAL:
    default:
      // Afișează semnul normal pentru acest semn de trafic
      // (pentru semnul 2, presupunem că este CEDEAZĂ)
      _displayManager->showTrafficSign("YIELD");
      break;
  }
}
