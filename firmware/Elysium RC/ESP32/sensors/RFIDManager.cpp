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

  static String lastEpcPrinted = "";
  if(epc != lastEpcPrinted){
    Serial.print("EPC: "); Serial.print(epc);
    Serial.print("  RSSI: "); Serial.print(rssiToDbm(rssi),1);
    Serial.println(" dBm");
    lastEpcPrinted = epc;
  }
}

/* =============================================================
 *  RFID_writeEpc – blocant, scrie EPC-ul furnizat (max 12 B ASCII)
 *  Returnează true dacă status = 0x00 sau 0x0E (no tag response)
 * =============================================================*/
bool RFID_writeEpc(const uint8_t* data, uint8_t len){
  if(len == 0 || len > 12) return false;          // max 12 ASCII

  static const uint8_t CMD_INV_STOP[]  = {0xBB,0x00,0x28,0x00,0x00,0x28,0x7E};
  static const uint8_t CMD_INV_START[] = {0xBB,0x00,0x27,0x00,0x03,0x22,0xFF,0xFF,0x4A,0x7E};
  static const uint8_t CMD_SINGLE[]    = {0xBB,0x00,0x22,0x00,0x00,0x22,0x7E};

  auto sumCS = [](const uint8_t* f, uint16_t l)->uint8_t{ uint8_t s=0; for(uint16_t i=1;i<l-2;++i) s+=f[i]; return s; };

  // 1. Stop inventar continuu
  Serial2.write(CMD_INV_STOP, sizeof(CMD_INV_STOP));
  Serial2.flush();
  delay(10);

  // 2. Pauză scurtă pentru a ne asigura că tagul rămâne alimentat
  delay(20);
  while(Serial2.available()) Serial2.read();      // golește eventualele cadre vechi

  // 3. Construim frame 0x49 cu helper
  uint8_t frame[64]; uint8_t i = 0;
  uint16_t DL = (len + 1) >> 1;                   // word count (16-bit)
  uint16_t PL = 4 + 1 + 2 + 2 + DL*2;             // payload length

  frame[i++] = 0xBB; frame[i++] = 0x00; frame[i++] = 0x49;
  frame[i++] = PL >> 8; frame[i++] = PL;
  for(int k=0;k<4;++k) frame[i++] = 0x00;         // AccessPwd = 0x00000000
  frame[i++] = 0x01;                              // MemBank EPC
  frame[i++] = 0x00; frame[i++] = 0x02;           // WordPtr = 2
  frame[i++] = DL >> 8; frame[i++] = DL;          // WordCnt
  for(uint8_t k=0;k<len;++k) frame[i++] = data[k];
  if(len & 1) frame[i++] = 0x00;                  // pad dacă len e impar
  frame[i++] = 0x00;                              // checksum placeholder
  frame[i++] = 0x7E;                              // tail
  uint8_t frameLen = i;                           // total length including 7E
  frame[frameLen - 2] = sumCS(frame, frameLen);   // compute checksum same as ref

  // Debug: print frame we send
  Serial.print("[RFID] TX frame (" ); Serial.print(i); Serial.println("):");
  for(uint8_t k=0;k<i;++k){ Serial.print(frame[k], HEX); Serial.print(" "); } Serial.println();

  Serial2.write(frame, i);
  Serial2.flush();

  // 4. Așteaptă ACK max 250 ms
  uint8_t resp[64]; uint8_t r = 0; uint32_t t0 = millis();
  uint8_t status = 0xFF;
  while(millis() - t0 < 300 && r < sizeof(resp)){
    if(Serial2.available()){
      uint8_t b = Serial2.read();
      if(r < sizeof(resp)) resp[r++] = b;
      // detect end of a frame
      if(b == 0x7E){
        // scan this frame for ACK 0x49
        for(uint8_t p = 0; p + 7 < r; ++p){
          if(resp[p] == 0xBB && resp[p+1] == 0x01 && resp[p+2] == 0x49){
            status = resp[p+5];
          }
        }
        // dacă am găsit ACK iesim
        if(status != 0xFF) break;
        // altfel continuăm să citim alte cadre
      }
    }
  }
  Serial.print("[RFID] Write ACK status=0x");
  Serial.println(status, HEX);

  if(status == 0xFF){
    Serial.print("Received frame: ");
    for(uint8_t p = 0; p < r; ++p){
      Serial.print(resp[p], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }

  // 5. Restart inventar continuu
  Serial2.write(CMD_INV_START, sizeof(CMD_INV_START));
  Serial2.flush();

  // evită false "removed"
  lastCardReadTime = millis();
  cardDetected     = true;

	Serial.print("\n\nIESIRE DIN MOD SCRIERE TAG...\n\n"); 

  return (status == 0x00 || status == 0x0E);
}

