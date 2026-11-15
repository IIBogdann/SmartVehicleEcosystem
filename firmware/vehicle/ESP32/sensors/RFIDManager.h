#ifndef RFID_MANAGER_H
#define RFID_MANAGER_H

#include <Arduino.h>

// Definițiile pinilor pentru modulul RFID YRM1003
#define RFID_RX_PIN 23  // ESP32 primește date de la modulul RFID (conectat la TXD al YRM1003)
#define RFID_TX_PIN 19  // ESP32 trimite date către modulul RFID (conectat la RXD al YRM1003)

// Constants
#define RFID_BUFFER_SIZE 128
#define RFID_BAUD_RATE 115200

// Variabile pentru stocarea datelor
extern String lastCardID;    // ID-ul ultimului card citit
extern bool cardDetected;    // Indicator pentru detectarea unui card
extern unsigned long lastCardReadTime;  // Timestamp pentru ultima citire

// Funcții
void RFIDManager_init();
void RFIDManager_update();
String getRFIDCardID();
bool isCardPresent();
void sendRFIDCommand(const char* command);
bool RFID_writeEpc(const uint8_t* data, uint8_t len);

#endif // RFID_MANAGER_H
