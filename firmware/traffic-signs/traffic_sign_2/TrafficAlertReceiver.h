#ifndef TrafficAlertReceiver_h
#define TrafficAlertReceiver_h

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "ESP32_NOW.h"  // Pentru a folosi implementarea ESP_NOW
#include "DisplayManager.h"
#include <vector>

// Definiție canal ESP-NOW
#define ESPNOW_WIFI_CHANNEL 6

// Structura mesajului primit
typedef struct {
  uint8_t eventType;     // Tipul evenimentului (1 = ACCIDENT)
  char location[32];     // Locația evenimentului
} TrafficMessage;

class TrafficAlertReceiver {
public:
  // Constructor care primește display manager-ul
  TrafficAlertReceiver(DisplayManager* displayManager);

  // Inițializare modul ESP-NOW și înregistrare callback-uri
  void init();

  // Procesare mesaje primite
  void processMessage(const uint8_t* data, int len);

  // Resetare stare accident (chemată din handler-ul BLE)
  void resetAccidentState();

  // Activare stare accident - afișare semn și notificare BLE
  void activateAccidentState();

  // Verificare dacă suntem în stare de accident
  bool isInAccidentState() { return _accidentActive; }

  // Pointer static pentru a accesa instanța din callback-uri
  static TrafficAlertReceiver* _instance;

private:
  DisplayManager* _displayManager;
  bool _isInitialized;
  bool _accidentActive = false; // Flag pentru a ține minte dacă semaforul este în stare de accident
};

#endif // TrafficAlertReceiver_h
