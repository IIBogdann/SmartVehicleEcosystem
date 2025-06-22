#include "TrafficAlertReceiver.h"
#include <esp_wifi.h>  // Necesar pentru esp_wifi_set_protocol

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
  // Nu mai setăm WiFi.mode(WIFI_STA) pentru a nu interfera cu BLE
  // În schimb, configurăm WiFi pentru coexistență cu BLE
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_LR);
  
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
  peerInfo.channel = 1;                 // ACELAȘI canal pe care ai setat stația (1)
  peerInfo.encrypt = false;
  
  // Adăugare peer pentru a permite recepția
  esp_now_add_peer(&peerInfo);
}

void TrafficAlertReceiver::onDataReceived(const esp_now_recv_info_t* info, const uint8_t* data, int data_len) {
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
      // (pentru semnul 1, presupunem că este STOP)
      _displayManager->showTrafficSign("STOP");
      break;
  }
}
