#include "RFIDManager.h"

// --- Comenzi YRM1003 pre-calculate -------------------------------
static const uint8_t CMD_SET_PWR_30DBM[] = {
  0xBB,0x00,0xB6,0x00,0x02, 0x0B,0xB8, 0x7B,0x7E
};
static const uint8_t CMD_MULTI_START[]  = {
  0xBB,0x00,0x27,0x00,0x03, 0x22,0xFF,0xFF, 0x4A,0x7E
};

// Inițializare variabile globale
String lastCardID = "";
bool cardDetected = false;
unsigned long lastCardReadTime = 0;

// Forward declaration for binary frame parser
void parseYRMFrame(uint8_t* frame, uint8_t len);

/**
 * Inițializează modulul RFID
 */
void RFIDManager_init() {
  Serial.println("\n[RFID] Init YRM1003...");
  Serial2.begin(RFID_BAUD_RATE, SERIAL_8N1, RFID_RX_PIN, RFID_TX_PIN);
  delay(200);

  // 1. Set RF power 30 dBm
  Serial2.write(CMD_SET_PWR_30DBM, sizeof(CMD_SET_PWR_30DBM));
  Serial2.flush();
  delay(50);
  uint8_t resp[16]; int n = Serial2.readBytes(resp, sizeof(resp));
  if(n>=8 && resp[2]==0xB6 && resp[5]==0x00) Serial.println("[RFID] RF power 30 dBm OK");
  else                                         Serial.println("[RFID] ⚠️  Set power failed");

  // 2. Start continuous inventory
  Serial2.write(CMD_MULTI_START, sizeof(CMD_MULTI_START));
  Serial2.flush();

  // init vars
  lastCardID = "";
  cardDetected = false;
}


/**
 * Actualizează starea modulului RFID și procesează datele primite
 * Această funcție trebuie apelată frecvent în loop() pentru a detecta cardurile
 */
void RFIDManager_update() {
  static uint8_t frame[RFID_BUFFER_SIZE];
  static uint8_t idx = 0;

  while(Serial2.available()){
    uint8_t b = Serial2.read();
    frame[idx++] = b;
    if(idx >= RFID_BUFFER_SIZE) idx = 0;       // safety
    if(b == 0x7E && idx >= 8){
      parseYRMFrame(frame, idx);
      idx = 0;
    }
  }

  // card removed timeout (1 s fără citire)
  if(cardDetected && (millis() - lastCardReadTime > 1000)){
    cardDetected = false;
    Serial.println("[RFID] Card removed");
  }
}

/**
 * Returnează ID-ul ultimului tag citit
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
/* ------------------------------------------------------------------
 *  YRM1003 – helper functions
 * ------------------------------------------------------------------*/
float rssiToDbm(uint8_t r){ return -((255 - r)/2.0f); }

void parseYRMFrame(uint8_t* f, uint8_t len){
  if(len < 8) return;
  if(f[0]!=0xBB || f[len-1]!=0x7E) return;   // header / tail

  /* checksum */
  uint8_t cs = 0; for(uint8_t i=1;i<len-2;++i) cs += f[i];
  if(cs != f[len-2]) return;

  // Acceptăm doar mesajul "Tag Found" 0x02 0x22
  if(f[1]!=0x02 || f[2]!=0x22) return;

  uint16_t payloadLen = (f[3]<<8)|f[4];
  if(payloadLen + 7 != len) return;          // size mismatch

  uint8_t  rssi     = f[5];
    // EPC length: payload - RSSI(1) - PC(2) - CRC(2)
  uint8_t epcLen = payloadLen - 5;
  uint8_t epcStart = 8; // after PC

  String epc = "";
  for(uint8_t i = 0; i < epcLen; ++i){
    if(f[epcStart+i] < 0x10) epc += '0';
    epc += String(f[epcStart+i], HEX);
  }
  epc.toUpperCase();

  lastCardID       = epc;
  cardDetected     = true;
  lastCardReadTime = millis();

  Serial.print("EPC: "); Serial.print(epc);
  Serial.print("  RSSI: "); Serial.print(rssiToDbm(rssi),1);
  Serial.println(" dBm");
}

