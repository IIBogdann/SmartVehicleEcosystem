#include "RFIDManager.h"

// Inițializare variabile globale
String lastCardID = "";
bool cardDetected = false;
unsigned long lastCardReadTime = 0;
char rfidBuffer[RFID_BUFFER_SIZE];
int rfidBufferIndex = 0;

/**
 * Inițializează modulul RFID
 */
void RFIDManager_init() {
  Serial.println("\nInițializare Modul RFID YRM1003...");
  
  // Configurare UART2 pentru comunicația cu modulul RFID
  Serial2.begin(RFID_BAUD_RATE, SERIAL_8N1, RFID_RX_PIN, RFID_TX_PIN);
  delay(500); // Așteaptă stabilizarea conexiunii
  
  // Reset buffer
  memset(rfidBuffer, 0, RFID_BUFFER_SIZE);
  rfidBufferIndex = 0;
  
  Serial.println("Modul RFID YRM1003 inițializat");
  Serial.println("- RX pin (primire date): " + String(RFID_RX_PIN));
  Serial.println("- TX pin (trimitere comenzi): " + String(RFID_TX_PIN));
}

/**
 * Actualizează starea modulului RFID și procesează datele primite
 * Această funcție trebuie apelată frecvent în loop() pentru a detecta cardurile
 */
void RFIDManager_update() {
  // Verifică dacă sunt date disponibile de la modulul RFID
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    
    // Adaugă caracterul la buffer dacă mai este spațiu și nu e caracter de terminare
    if (rfidBufferIndex < RFID_BUFFER_SIZE - 1 && c != '\r' && c != '\n') {
      rfidBuffer[rfidBufferIndex] = c;
      rfidBufferIndex++;
    } 
    // Procesează datele când primim terminatorul de linie
    else if (c == '\n' || c == '\r') {
      if (rfidBufferIndex > 0) {
        rfidBuffer[rfidBufferIndex] = '\0'; // Termină string-ul
        String data = String(rfidBuffer);
        processRFIDData(data);
        
        // Reset buffer pentru următoarea citire
        memset(rfidBuffer, 0, RFID_BUFFER_SIZE);
        rfidBufferIndex = 0;
      }
    }
  }
  
  // Verifică dacă cardul a fost scos (dacă nu s-a mai citit nimic în ultimele 1000ms)
  if (cardDetected && (millis() - lastCardReadTime > 1000)) {
    cardDetected = false;
    Serial.println("Card RFID îndepărtat");
  }
}

/**
 * Returnează ID-ul ultimului card citit
 */
String getRFIDCardID() {
  return lastCardID;
}

/**
 * Verifică dacă există un card prezent în câmpul cititorului
 */
bool isCardPresent() {
  return cardDetected;
}

/**
 * Trimite o comandă către cititorul RFID
 */
void sendRFIDCommand(const char* command) {
  Serial2.println(command);
  Serial.println("Comandă trimisă către RFID: " + String(command));
}

/**
 * Procesează datele primite de la cititorul RFID
 * Formatul exact al datelor depinde de modelul YRM1003, 
 * va trebui ajustat în funcție de datele reale primite
 */
void processRFIDData(String data) {
  // Curățăm datele primite de spații și caractere speciale
  data.trim();
  
  // Verificăm dacă avem date valide (putem ajusta această logică)
  if (data.length() > 8) {
    Serial.println("Date RFID primite: " + data);
    
    // În acest exemplu, presupunem că datele primite conțin direct ID-ul cardului
    // Va trebui ajustat în funcție de formatul real al datelor
    lastCardID = data;
    cardDetected = true;
    lastCardReadTime = millis();
    
    Serial.println("Card RFID detectat, ID: " + lastCardID);
  }
}
