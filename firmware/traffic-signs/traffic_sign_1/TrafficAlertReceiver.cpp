#include "TrafficAlertReceiver.h"
#include "BleManager.h"  // Necesar pentru notificarea BLE
#include <esp_mac.h>  // Pentru macro-urile MAC2STR și MACSTR

TrafficAlertReceiver* TrafficAlertReceiver::_instance = nullptr;

TrafficAlertReceiver::TrafficAlertReceiver(DisplayManager* displayManager)
  : _displayManager(displayManager), _isInitialized(false) {
  // Setăm instanța statică pentru a fi accesată din callback-uri
  _instance = this;
}

// Callback pentru mesaje broadcast primite de la un peer necunoscut
void receive_broadcast(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
  // Verificăm dacă este mesaj broadcast (adresa de destinație este FF:FF:FF:FF:FF:FF)
  if (memcmp(info->des_addr, ESP_NOW.BROADCAST_ADDR, 6) == 0) {
    Serial.printf("Mesaj broadcast primit de la: " MACSTR " (len=%d)\n", MAC2STR(info->src_addr), len);

    // Încercăm să afișăm mesajul ca text (poate e text)
    Serial.print("Conținut mesaj (text): ");
    for (int i = 0; i < len && i < 32; i++) {
      if (data[i] >= 32 && data[i] <= 126) { // Caractere ASCII imprimabile
        Serial.print((char)data[i]);
      } else {
        Serial.print(".");
      }
    }
    Serial.println();

    if (TrafficAlertReceiver::_instance) {
      TrafficAlertReceiver::_instance->processMessage(data, len);
    }
  } else {
    // Ignorăm mesajele unicast
    Serial.printf("Mesaj unicast primit de la " MACSTR " - ignorat\n", MAC2STR(info->src_addr));
  }
}

void TrafficAlertReceiver::init() {
  // Configurare WiFi în modul stație și setare canal
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);

  // Așteptăm pornirea WiFi
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("Inițializare TrafficAlertReceiver - ESP-NOW Slave");
  Serial.println("  Mode: STA");
  Serial.println("  Adresa MAC: " + WiFi.macAddress());
  Serial.printf("  Canal: %d\n", ESPNOW_WIFI_CHANNEL);

  if (!ESP_NOW.begin()) {
    Serial.println("Eroare la inițializarea ESP-NOW!");
    Serial.println("Repornire în 5 secunde...");
    delay(5000);
    ESP.restart();
    return;
  }

  // Înregistrăm callback-ul pentru recepția mesajelor broadcast de la dispozitive necunoscute
  ESP_NOW.onNewPeer(receive_broadcast, NULL);

  _isInitialized = true;
  Serial.println("TrafficAlertReceiver inițializat, în așteptare pentru mesaje broadcast...");
}

void TrafficAlertReceiver::processMessage(const uint8_t* data, int len) {
  if (!_displayManager) {
    Serial.println("Eroare: Display manager neinițializat!");
    return;
  }

  // Dacă deja suntem în stare de accident, nu mai actualizăm ecranul
  // Resetarea se va face doar din aplicația Android
  if (_accidentActive) {
    Serial.println("Mesaj de accident ignorat - semnul este deja în stare de accident");
    return;
  }

  // Metoda 1: Încercăm să procesam ca TrafficMessage dacă avem dimensiunea potrivită
  if (len >= sizeof(TrafficMessage)) {
    TrafficMessage message;
    memcpy(&message, data, sizeof(TrafficMessage));

    Serial.printf("Interpretare ca TrafficMessage - Tip: %d, Locație: %s\n", message.eventType, message.location);

    // Ne concentrăm doar pe mesajele de accident (eventType = 1)
    if (message.eventType == 1) { // ACCIDENT
      activateAccidentState();
      return;
    }
  }

  // Metoda 2: Încercăm să interpretăm în alte moduri
  // Verificăm dacă conține cuvântul "accident" în orice format
  bool isAccidentMessage = false;

  // 1. Cautăm cuvântul "accident" în mesaj (poate e text)
  if (len >= 8) { // Lungimea minimă pentru "accident"
    for (int i = 0; i <= len - 8; i++) {
      if ((data[i] == 'a' || data[i] == 'A') &&
          (data[i+1] == 'c' || data[i+1] == 'C') &&
          (data[i+2] == 'c' || data[i+2] == 'C') &&
          (data[i+3] == 'i' || data[i+3] == 'I') &&
          (data[i+4] == 'd' || data[i+4] == 'D') &&
          (data[i+5] == 'e' || data[i+5] == 'E') &&
          (data[i+6] == 'n' || data[i+6] == 'N') &&
          (data[i+7] == 't' || data[i+7] == 'T')) {
        isAccidentMessage = true;
        break;
      }
    }
  }

  // 2. Cautăm byte-ul 0x01 în primii bytes (în cazul în care este codul pentru accident)
  if (!isAccidentMessage && len >= 1) {
    if (data[0] == 0x01) {
      isAccidentMessage = true;
      Serial.println("Detectat cod posibil pentru accident (0x01)");
    }
  }

  // Dacă am detectat un mesaj de accident în orice format
  if (isAccidentMessage) {
    activateAccidentState();
    return;
  }

  Serial.println("Mesaj ignorat: format necunoscut");
}

// Activare stare accident - afișare semn și notificare BLE
void TrafficAlertReceiver::activateAccidentState() {
  _accidentActive = true;

  Serial.println("ACCIDENT detectat! Actualizez display-ul...");
  if (_displayManager) {
    _displayManager->showAccidentAlert();

    // Trimitem notificare către aplicația Android
    extern BleManager bleManager;
    bleManager.sendStatusUpdate("SignID=1;Event=ACCIDENT");

    Serial.println("Notificare trimisă către aplicația Android");
    Serial.println("Semn blocat în starea ACCIDENT. Resetare posibilă doar din aplicația Android.");
  } else {
    Serial.println("Eroare: Display manager neinițializat pentru afișarea semnului de accident!");
  }
}

// Resetare stare accident - folosită de handler-ul BLE pentru comanda din aplicație
void TrafficAlertReceiver::resetAccidentState() {
  if (_accidentActive) {
    _accidentActive = false;
    Serial.println("Resetare stare accident - revenire la semn normal");

    _displayManager->showTrafficSign("STOP"); // Sau alt semn implicit

    Serial.println("Semn resetat cu succes");
  } else {
    Serial.println("Resetare ignorată - nu suntem în stare de accident");
  }
}
