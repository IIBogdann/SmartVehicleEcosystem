/**
 * Adaptive Traffic System - E-Paper Display
 * 
 * Componentă a proiectului SmartVehicleEcosystem
 * Autor: Bulgariu (Mihăilă) Elena-Iuliana & Mihăilă Bogdan-Iulian
 * 
 * Hardware: MH ET Live 2.13 inch e-paper display cu XIAO ESP32C3
 * 
 * Actualizări:
 * - Adăugare server BLE pentru comunicare cu aplicația Android
 * - Implementare ESP-Now pentru sincronizare între semne de trafic
 */

// Traffic Sign 1 - Firmware for Adaptive Traffic System
// Based on ESP-NOW Broadcast Slave example

#include <Arduino.h>
#include <SPI.h>
#include "Config.h"

// Declarații înainte de folosire
extern volatile bool propaga_mesaj_urgenta;
extern struct traffic_message mesaj_urgenta_de_propagat; // struct forward declaration will come later

// Variabilele pentru propagarea mesajelor de urgență vor fi definite mai jos, după definirea structurii traffic_message

#include "DisplayManager.h"
#include "BleManager.h"
#include "ESP32_NOW.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_mac.h>  // Pentru macrourile MAC2STR și MACSTR
#include <vector>

// Dezactivăm modulul TrafficAlertReceiver deoarece funcționalitatea sa 
// a fost integrată în implementarea ESP32_NOW
#define TRAFFIC_ALERT_RECEIVER_DISABLED 1

// Declarăm extern variabilele care vor fi folosite în clase
extern DisplayManager epaperDisplay;
extern BleManager bleManager;

/* Definiții pentru ESP-NOW */
#define ESPNOW_WIFI_CHANNEL 6  // Folosim canalul 6 pentru compatibilitate cu exemplul master

// Tipuri de evenimente ce pot fi primite de la Elysium RC
enum ElysiumEventType {
  EVENT_NORMAL = 0,
  EVENT_ACCIDENT = 1,
  EVENT_OBSTACLE = 2,
  EVENT_EMERGENCY = 3
};

// Structura mesajelor primite de la vehicule (preluate din TrafficAlertReceiver)
typedef struct __attribute__((packed)) ElysiumMessage {
  uint8_t eventType;     // Tipul evenimentului (din enum ElysiumEventType)
  uint8_t severity;      // Severitatea (1-10)
  char location[32];     // Locația evenimentului
} ElysiumMessage;

// Structura datelor care vor fi transmise prin ESP-Now între semne
typedef struct __attribute__((packed)) traffic_message {
  uint8_t targetId;      // 0 = broadcast, altfel ID semn
  char    signType[20];  // "STOP", "ACCIDENT" etc.
  uint8_t priority;      // 0 = normal, 1 = urgent
} traffic_message;

// Variabile globale dependente de traffic_message
volatile bool propaga_mesaj_urgenta = false;
traffic_message mesaj_urgenta_de_propagat;

/* Clasă pentru gestionarea peer-ilor ESP-NOW */
class ESP_NOW_Peer_Class : public ESP_NOW_Peer {
public:
  // Constructor
  ESP_NOW_Peer_Class(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(mac_addr, channel, iface, lmk) {}

  // Destructor
  ~ESP_NOW_Peer_Class() {}

  // Funcție pentru înregistrarea peer-ului master
  bool add_peer() {
    if (!add()) {
      log_e("Eroare la înregistrarea peer-ului broadcast");
      return false;
    }
    return true;
  }

  // Funcție publică de trimitere a unui mesaj (wrapper peste metoda protected send)
  bool send_message(const uint8_t *data, size_t len) {
    return send(data, (int)len) == len;
  }

  // Funcție pentru procesarea mesajelor primite de la master
  void onReceive(const uint8_t *data, size_t len, bool broadcast) {
    Serial.printf("Mesaj primit de la master " MACSTR " (%s)\n", MAC2STR(addr()), broadcast ? "broadcast" : "unicast");
    
    // Afișăm conținutul mesajului pentru debugging
    Serial.print("Conținut mesaj (text): '");
    for (int i = 0; i < len && i < 32; i++) {
      Serial.print((char)data[i]);
    }
    Serial.println("'");

    Serial.print("Conținut mesaj (hex): ");
    for (int i = 0; i < len && i < 16; i++) {
      Serial.printf("%02X ", data[i]);
    }
    Serial.println();
    
    // Analizăm tipul de mesaj primit și îl procesăm corespunzător
    if (len == sizeof(traffic_message)) {
      // Este un mesaj de la un alt semn de circulație
      processTrafficSignMessage(data, len, broadcast);
    } 
    else if (len == sizeof(ElysiumMessage)) {
      // Este un mesaj de la un vehicul (de ex. Elysium RC)
      processVehicleMessage(data, len, broadcast);
    }
    else {
      // Mesaj necunoscut sau text simplu - încercăm să îl procesăm ca text
      // Creăm un buffer terminat cu NULL pentru a gestiona corect șirul de caractere
      char textBuffer[33]; // 32 caractere + NULL terminator
      memset(textBuffer, 0, sizeof(textBuffer));
      int copyLen = min(len, sizeof(textBuffer)-1);
      memcpy(textBuffer, data, copyLen);
      
      Serial.printf("  Mesaj text primit: %s\n", textBuffer);
      
      // Pentru testare, vom afișa mesaj pe semn când primim mesaje text
      // Extragem numărul mesajului pentru afișare
      char* numberPos = strstr(textBuffer, "#");
      if (numberPos) {
        epaperDisplay.showTrafficSign(numberPos); // Afișăm doar partea cu numărul
      } else {
        epaperDisplay.showTrafficSign("TEST");
      }
      
      // Notificăm aplicația Android
      String status = "SignID=" + String(SIGN_ID) + ";Event=TEXT_MESSAGE;Content=";
      status += String(textBuffer);
      bleManager.sendStatusUpdate(status);
    }
  }
  
  // Procesare mesaje de la alte semne de circulație
  void processTrafficSignMessage(const uint8_t *data, size_t len, bool broadcast) {
    traffic_message *message = (traffic_message*) data;
  
    // Verificăm dacă mesajul este pentru acest semn sau broadcast
    if (message->targetId == 0 || message->targetId == SIGN_ID) {
      Serial.printf("Mesaj primit de la alt semn pentru semnul %d\n", SIGN_ID);
      Serial.printf("Tip de semn: %s, Prioritate: %d\n", message->signType, message->priority);
      
      // Ne ocupăm de mesaj în funcție de prioritate
      if (message->priority > 0) {
        // Mesaj prioritar (accident, urgență, obstacol)
        Serial.println("ATENȚIE: Mesaj prioritar primit!");
        
        // Verificăm dacă mesajul conține un eveniment de tip accident
        if (strstr(message->signType, "ACCIDENT") != NULL) {
          epaperDisplay.showTrafficSign("ACCIDENT");
          Serial.println("Afișez ACCIDENT pe display");
        } 
        else if (strstr(message->signType, "OBSTACLE") != NULL || strstr(message->signType, "OBSTACOL") != NULL) {
          epaperDisplay.showTrafficSign("OBSTACOL");
          Serial.println("Afișez OBSTACOL pe display");
        }
        else if (strstr(message->signType, "EMERGENCY") != NULL || strstr(message->signType, "URGENTA") != NULL) {
          epaperDisplay.showTrafficSign("URGENTA");
          Serial.println("Afișez URGENTA pe display");
        }
        else {
          // Alt tip de mesaj prioritar, afișăm direct conținutul
          epaperDisplay.showTrafficSign(message->signType);
        }
        
        // Notificăm aplicația Android despre eveniment prioritar
        String status = "SignID=" + String(SIGN_ID) + ";Event=" + String(message->signType);
        status += ";Priority=" + String(message->priority) + ";Source=OtherSign";
        bleManager.sendStatusUpdate(status);
      } else {
        // Mesaj normal (schimbare de semn)
        Serial.printf("Actualizez semnul cu: %s\n", message->signType);
        epaperDisplay.showTrafficSign(message->signType);
        
        // Notificăm aplicația Android despre schimbarea normală
        String status = "SignID=" + String(SIGN_ID) + ";Event=SignChange;"
                     + "Sign=" + String(message->signType) + ";Ack=OK";
        bleManager.sendStatusUpdate(status);
      }
      
    } else {
      // Mesaj pentru alt semn - îl ignorăm
      Serial.printf("Mesaj destinat semnului %d - ignorat.\n", message->targetId);
    }
  }
  
  // Procesare mesaje de la vehicule (Elysium sau alte vehicule compatibile)
  void processVehicleMessage(const uint8_t *data, size_t len, bool broadcast) {
    ElysiumMessage *message = (ElysiumMessage*) data;
    
    Serial.print("Mesaj de la vehicul. Tip eveniment: ");
    String eventType;
    String eventDisplay;
    bool isEmergency = false; // Flag pentru evenimentele care necesită propagare
    
    // În funcție de tipul evenimentului, actualizăm afișajul
    switch (message->eventType) {
      case EVENT_ACCIDENT:
        eventType = "ACCIDENT";
        eventDisplay = "ACCIDENT";
        Serial.println("Accident");
        Serial.printf("Accident raportat în locația: %s. Severitate: %d\n", 
                      message->location, message->severity);
        isEmergency = true;
        break;
        
      case EVENT_OBSTACLE:
        eventType = "OBSTACLE";
        eventDisplay = "OBSTACOL";
        Serial.println("Obstacol");
        Serial.printf("Obstacol raportat în locația: %s. Severitate: %d\n", 
                      message->location, message->severity);
        isEmergency = true;
        break;
        
      case EVENT_EMERGENCY:
        eventType = "EMERGENCY";
        eventDisplay = "URGENTA";
        Serial.println("Urgență");
        Serial.printf("Urgență raportată în locația: %s. Severitate: %d\n", 
                      message->location, message->severity);
        isEmergency = true;
        break;
        
      case EVENT_NORMAL:
      default:
        eventType = "NORMAL";
        eventDisplay = "STOP"; // Semnul default pentru acest semn
        Serial.println("Normal/Default");
        break;
    }
    
    // Afișăm semnul corespunzător
    epaperDisplay.showTrafficSign(eventDisplay.c_str());
    
    // Notificăm aplicația Android cu detalii despre vehicul și eveniment
    String status = "SignID=" + String(SIGN_ID) + ";Event=" + eventType;
    status += ";Location=" + String(message->location);
    status += ";Severity=" + String(message->severity);
    status += ";Time=" + String(millis());
    bleManager.sendStatusUpdate(status);
    
    // Dacă este un eveniment de urgență (accident, obstacol, etc.),
    // îl vom semnala pentru propagare
    if (isEmergency && !broadcast) {
      // Creăm un mesaj de trafic care va fi transmis din exteriorul clasei
      static traffic_message emergencyMessage;
      emergencyMessage.targetId = 0; // 0 = broadcast
      strncpy(emergencyMessage.signType, eventType.c_str(), sizeof(emergencyMessage.signType)-1);
      emergencyMessage.signType[sizeof(emergencyMessage.signType)-1] = '\0'; // Asigurăm NULL terminator
      emergencyMessage.priority = 1; // 1 = urgent
      
      // Setăm un flag global pentru a indica necesitatea propagării
      propaga_mesaj_urgenta = true;
      memcpy(&mesaj_urgenta_de_propagat, &emergencyMessage, sizeof(traffic_message));
      
      Serial.printf("Am marcat evenimentul %s pentru propagare către alte semne\n", eventType.c_str());
    }
  }
};

/* Variabile globale pentru ESP-NOW */
// Lista cu toți master-ii. Va fi populată când un nou master este înregistrat
std::vector<ESP_NOW_Peer_Class> masters;

// Variabile pentru controlul secvenței de afișare
bool welcomeShown = false;
unsigned long welcomeStartTime = 0;
const unsigned long WELCOME_DURATION = 5000; // 5 secunde

// Manager BLE pentru comunicare cu aplicația Android
BleManager bleManager(&epaperDisplay);

/* Callback pentru înregistrarea unui nou master */
void register_new_master(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
  // Adăugăm debug pentru toate mesajele primite
  Serial.println("DEBUG: Callback onNewPeer a fost apelat!");
  Serial.printf("DEBUG: Mesaj primit de la " MACSTR " pentru " MACSTR ", len=%d\n", 
                MAC2STR(info->src_addr), MAC2STR(info->des_addr), len);
  
  // Afișăm conținutul mesajului pentru debugging
  Serial.print("DEBUG: Conținut mesaj: '");
  for (int i = 0; i < min(len, 32); i++) {
    Serial.print((char)data[i]);
  }
  Serial.println("'");
  
  if (memcmp(info->des_addr, ESP_NOW.BROADCAST_ADDR, 6) == 0) {
    Serial.printf("Master necunoscut " MACSTR " a trimis un mesaj broadcast\n", MAC2STR(info->src_addr));
    Serial.println("Înregistrez peer-ul ca master");

    // Verificăm dacă dispozitivul nu există deja în lista noastră
    for (auto &master : masters) {
      if (memcmp(info->src_addr, master.addr(), ESP_NOW_ETH_ALEN) == 0) {
        Serial.print("Master-ul există deja: ");
        Serial.printf(MACSTR "\n", MAC2STR(info->src_addr));
        return;
      }
    }

    ESP_NOW_Peer_Class new_master(info->src_addr, ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

    masters.push_back(new_master);
    if (!masters.back().add_peer()) {
      Serial.println("Eroare la înregistrarea noului master");
      return;
    }
    
    Serial.println("Master nou înregistrat cu succes");
  } else {
    // Semnul va primi doar mesaje broadcast
    Serial.printf("Mesaj unicast primit de la " MACSTR ", ignorat\n", MAC2STR(info->src_addr));
  }
}
uint8_t elysiumMacAddress[] = ELYSIUM_MAC;

void setup() {
  // Inițializare serial pentru debugging
  Serial.begin(115200);
  Serial.println("Adaptive Traffic System - Pornire");
  
  // 1. Eliberare memorie Bluetooth Classic (doar BLE necesar)
  Serial.println("1. Eliberare memorie Bluetooth Classic");
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  
  // 2. Inițializare BLE Manager
  Serial.println("2. Inițializare BLE Manager");
  bleManager.init();
  Serial.println("   BLE inițializat cu succes");
  
  // 3. Pornire WiFi în modul STA cu canalul 6 pentru ESP-NOW
  Serial.println("3. Pornire WiFi în modul STA");
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  // Forțăm canalul WiFi să fie 6 pentru compatibilitate cu exemplul master
  esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  while (!WiFi.STA.started()) {
    delay(100);
  }
  
  Serial.println("Adaptive Traffic System - ESP-NOW Slave");
  Serial.println("Parametri Wi-Fi:");
  Serial.println("  Mod: STA");
  Serial.println("  Adresa MAC: " + WiFi.macAddress());
  Serial.printf("  Canal: %d\n", ESPNOW_WIFI_CHANNEL);
  
  // Inițializare protocol ESP-NOW pentru comunicare cu orice dispozitiv
  Serial.println("4. Inițializare ESP-NOW");
  if (!ESP_NOW.begin()) {
    Serial.println("Eroare la inițializarea ESP-NOW!");
    Serial.println("Repornire în 5 secunde...");
    delay(5000);
    ESP.restart();
  }
  
  // Afișăm informații de debugging despre starea ESP-NOW
  Serial.printf("DEBUG: Total peer count: %d\n", ESP_NOW.getTotalPeerCount());
  Serial.printf("DEBUG: Adresa MAC locală: %s\n", WiFi.macAddress().c_str());
  
  // Înregistrăm callback-ul pentru auto-înregistrarea dispozitivelor
  ESP_NOW.onNewPeer(register_new_master, NULL);
  Serial.println("DEBUG: Callback onNewPeer configurat");
  
  Serial.println("ESP-NOW configurat pentru a primi mesaje de la orice dispozitiv");
  
  // Adăugăm dispozitivul Elysium ca peer pentru comunicare targetată
  ESP_NOW_Peer_Class elysiumPeer(elysiumMacAddress, ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);
  masters.push_back(elysiumPeer);
  if (!masters.back().add_peer()) {
    Serial.println("Eroare la înregistrarea peer-ului Elysium!");
  } else {
    Serial.println("Peer Elysium înregistrat cu succes.");
  }
  
  Serial.println("ESP-NOW configurat pentru a primi mesaje de la orice dispozitiv");
  
  // 5. Inițializare display și alte componente
  Serial.println("5. Inițializare display");
  epaperDisplay.init();
  Serial.println("   Display inițializat");
  
  // Afișarea mesajului de bun venit
  Serial.println("6. Afișare mesaj de bun venit");
  epaperDisplay.welcomeMessage();
  
  // Inițializarea cronometrului pentru tranziție
  welcomeStartTime = millis();
  welcomeShown = true;
  
  Serial.println("Inițializare completă. Sistem pregătit pentru comenzi BLE și mesaje ESP-NOW.");
  Serial.println("Aștept mesaje broadcast de la dispozitive master...");
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
  
  // BLE este gestionat automat de biblioteca BleManager
  
  // Propagăm mesajele de urgență dacă este necesar
  if (propaga_mesaj_urgenta) {
    Serial.println("Propagare mesaj de urgență către toate semnele înregistrate");
    
    // Trimitem mesajul către toate semnele înregistrate
    bool cel_putin_unul_trimis = false;
    for (auto &master : masters) {
      Serial.printf("Trimit mesajul de urgență '%s' către un alt semn\n", mesaj_urgenta_de_propagat.signType);
      if (master.send_message((uint8_t*)&mesaj_urgenta_de_propagat, sizeof(traffic_message))) {
        cel_putin_unul_trimis = true;
        Serial.println("Mesaj trimis cu succes!");
      } else {
        Serial.println("Eroare la trimiterea mesajului!");
      }
    }
    
    if (!cel_putin_unul_trimis) {
      Serial.println("Atenție: Niciun semn nu a putut fi notificat!");
    }
    
    // Resetăm flag-ul
    propaga_mesaj_urgenta = false;
  }
  
  // Afișăm periodic informații despre starea ESP-NOW
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 10000) {  // La fiecare 10 secunde
    lastDebugTime = millis();
    Serial.printf("DEBUG: Stare ESP-NOW - Total peers: %d\n", ESP_NOW.getTotalPeerCount());
    Serial.printf("DEBUG: Număr masters înregistrați: %d\n", masters.size());
    Serial.println("DEBUG: Aștept în continuare mesaje broadcast...");
    Serial.println("DEBUG: Adresa MAC locală: " + WiFi.macAddress());
    Serial.printf("DEBUG: Canal WiFi: %d\n", WiFi.channel());
    
    // Verificăm dacă canalul WiFi coincide cu cel configurat
    if (WiFi.channel() != ESPNOW_WIFI_CHANNEL) {
      Serial.printf("AVERTISMENT: Canalul WiFi curent (%d) nu coincide cu ESPNOW_WIFI_CHANNEL (%d)\n", 
                    WiFi.channel(), ESPNOW_WIFI_CHANNEL);
    }
  }
  
  delay(100);
}
