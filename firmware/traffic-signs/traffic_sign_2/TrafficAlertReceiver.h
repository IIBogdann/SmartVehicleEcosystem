#ifndef TRAFFIC_ALERT_RECEIVER_H
#define TRAFFIC_ALERT_RECEIVER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "DisplayManager.h"

// Tipuri de evenimente ce pot fi primite de la Elysium RC
enum ElysiumEventType {
  EVENT_NORMAL = 0,
  EVENT_ACCIDENT = 1,
  EVENT_OBSTACLE = 2,
  EVENT_EMERGENCY = 3
};

// Structura mesajului primit
typedef struct {
  uint8_t eventType;     // Tipul evenimentului (din enum ElysiumEventType)
  uint8_t severity;      // Severitatea (1-10)
  char location[32];     // Locația evenimentului
} ElysiumMessage;

class TrafficAlertReceiver {
public:
  TrafficAlertReceiver(DisplayManager* displayManager);
  void init();
  void setElysiumMac(const uint8_t* mac);
  void processMessage(const ElysiumMessage& message);

private:
  DisplayManager* _displayManager;
  uint8_t _elysiumMacAddress[6];
  bool _isInitialized;

  // Callback-uri statice pentru ESP-Now
  static void onDataReceived(const esp_now_recv_info* info, const uint8_t* data, int data_len);
  
  // Pointer static pentru a accesa instanța din callback-uri
  static TrafficAlertReceiver* _instance;
};

#endif // TRAFFIC_ALERT_RECEIVER_H
