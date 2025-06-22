#include "AccidentDetector.h"

#include "../sensors/UltrasonicSensors.h"
#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>
#include <string.h>

#define ESPNOW_WIFI_CHANNEL 6
// Declanșează accident dacă un senzor raportează -1 timp de ≥5 secunde
#define ACC_INVALID_MS 5000
#define ACC_COOLDOWN_MS 100

// Struct identic cu cel folosit de semnele de trafic
typedef struct __attribute__((packed)) traffic_message {
  uint8_t targetId;      // 0 = broadcast
  char    signType[20];  // "ACCIDENT" etc.
  uint8_t priority;      // 1 = urgent
} traffic_message;

class AccidentBroadcastPeer : public ESP_NOW_Peer {
public:
  AccidentBroadcastPeer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) :
      ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      return false;
    }
    return true;
  }

  bool send_message(const uint8_t *data, size_t len) {
    return send(data, len);
  }
};

static AccidentBroadcastPeer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

// Stare internă
static unsigned long invalidSince[4] = {0};
static unsigned long lastAccidentMillis = 0;

static void sendAccident() {
  traffic_message msg{};
  msg.targetId = 0;
  strncpy(msg.signType, "ACCIDENT", sizeof(msg.signType));
  msg.priority = 1;

  broadcast_peer.send_message(reinterpret_cast<uint8_t*>(&msg), sizeof(msg));
  Serial.println("[AccidentDetector] ACCIDENT transmis prin ESP-NOW!");
}

void AccidentDetector_init() {
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(50);
  }

  if (!broadcast_peer.begin()) {
    Serial.println("[AccidentDetector] Eroare init ESP-NOW, se repornește în 5s");
    delay(5000);
    ESP.restart();
  }
}

void AccidentDetector_update() {
  unsigned long now = millis();
  long dists[4] = { distanceFront, distanceBack, distanceLeft, distanceRight };

  for (int i = 0; i < 4; ++i) {
    if (dists[i] == -1) {
      if (invalidSince[i] == 0) invalidSince[i] = now; // start perioadă invalidă
    } else {
      invalidSince[i] = 0; // senzor valid → reset
    }
  }

  bool trigger = false;
  for (int i = 0; i < 4; ++i) {
    if (invalidSince[i] != 0 && (now - invalidSince[i] >= ACC_INVALID_MS)) {
      trigger = true;
      break;
    }
  }

  if (trigger && (now - lastAccidentMillis > ACC_COOLDOWN_MS)) {
    sendAccident();
    lastAccidentMillis = now;

  }
}
