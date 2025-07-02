
#include <ESP32Servo.h>
#include <Arduino.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Include module componente
// Core
#include "../core/BluetoothManager.h"
#include "../core/TaskManager.h"
// Actuators
#include "../motion-control/DCMotor.h"
#include "../motion-control/ServoMotor.h"
// Sensors
#include "../sensors/UltrasonicSensors.h"
#include "../sensors/RFIDManager.h"
// Feedback
#include "../feedback/BuzzerManager.h"
#include "../alerts/AccidentDetector.h"


// ******************* GLOBAL **************************************
Servo servo;
String mesaj;
BluetoothManager btManager;
const int RXD1 = 16;
const int TXD1 = 17;
// *******************************************************************

// ------- RFID sign handling ------------------
// STOP_EPC păstrat doar pentru referinţă, nu mai este folosit direct
const char* STOP_EPC = "E200470D88D068218CD6010E";
static bool stopMode = false;
static bool stopLatched = false;
static unsigned long stopEnd = 0;

// viteză normală implicită
const int NORMAL_MOTOR_SPEED = 190;

// YIELD handling
const int  YIELD_SPEED = 100;           // duty pentru cedare (0-255)  // <= va deveni MOTOR_SPEED când yieldMode activ
static bool yieldMode  = false;         // true cât timp tag YIELD este prezent
// ---------------------------------------------
const unsigned long STOP_COOLDOWN_MS = 10000; // 10s fara semn pentru reset
static unsigned long lastStopSeen = 0;

// --- RFID write mode state ---
volatile bool rfidWriteMode = false;
static String writePayload = "";
// --- date pentru scriere tag ---
static String lastSignCode = "";   // "S", "Y", "30", "50" etc.
static int    currentY = 0;
static bool   hasY = false;

// timp până la care suprimăm print-uri diagnostice
volatile unsigned long muteConsoleUntil = 0;

// ------------------- helper -------------------
char getSignCode(const String& epc){
  if(epc.length() < 2) return 0;
  String byteStr = epc.substring(0,2);
  int val = strtol(byteStr.c_str(), nullptr, 16);
  return (char)val;
}




void setup() {

  // ************************* SERIAL***************************************
  Serial.begin(115200);
  delay(500);
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
  delay(1000);
  //**************************************************************************



  //========================== START ==========================================
  Serial.println("\n*** Elysium RC is starting... ***");
  Serial.println("*** Initializare componente... ***");
  delay(1000);
  //===========================================================================


  btManager.begin();
  DCMotor_init();
  ServoMotor_init();
  UltrasonicSensors_init();
  Buzzer_init();
  AccidentDetector_init();
  RFIDManager_init();


  //################################# TEST AND DIAGNOSE #######################################

  //-------------------------- DC MOTOR -------------------------------------------
  Serial.println("\nDC Motor test... ");
  DCMotor(true, false);
  delay(1000);
  DCMotor(false, false); 
  delay(1000);
  DCMotor(false, true);
  delay(1000);
  DCMotor(false, false);
  delay(1000);

  
  //---------------------- SERVO -------------------------------------------------
  Serial.println("\nServomotor test..."); delay(1000);
  Serial.println("1. Test stânga maxim...");
  ServoMotor(LEFT); delay(1000);
  Serial.println("2. Test dreapta maxim"); delay(1000);
  ServoMotor(RIGHT);
  Serial.println("3. Revenire la centru"); delay(1000);
  ServoMotor(CENTER); delay(1000);

  Tasks_init();
  
  //---------------------- RFID -------------------------------------------------
  Serial.println("\nRFID Reader test..."); delay(1000);
  Serial.println("1. Așteptare card RFID...");
  

  Serial.println("\n\n*** Elysium RC is READY! *** \n\n");
  delay(1000);
  
}

void loop() {
  unsigned long now = millis();
  // ieșire din stopMode dacă a expirat
  // Cooldown STOP – latch se resetează doar după 10s fără semn
  if(stopLatched && !isCardPresent() && (now - lastStopSeen >= STOP_COOLDOWN_MS)){
    stopLatched = false;
    Serial.println("[STOP] cooldown ended – STOP poate declanșa din nou");
  }
  if(stopMode && now >= stopEnd){
    stopMode = false;
    Serial.println("[STOP] 5 secunde expirate – control reluat");
  }
  // Verificăm dacă avem comenzi de la Bluetooth (ignorat în stopMode)
  String command = btManager.receiveData();
  if(command.length()) Serial.printf("RX raw: %s\n", command.c_str());
  
  if (!stopMode && command.length() > 0) {
    Serial.print("Comandă primită: ");
    Serial.println(command);

  // --- WRITE_TAG (poate include \"type\") ---
  if(command.indexOf("WRITE_TAG") >= 0){
    // Dacă mesajul este JSON şi conţine "type":"..." actualizăm lastSignCode
    int pos = command.indexOf("\"type\":\"");
    if(pos >= 0){
      pos += 8; // trece de \"type":"
      int endq = command.indexOf('"', pos);
      if(endq > pos){
        String t = command.substring(pos, endq);
        t.toUpperCase();
        if(t == "STOP" || t == "S")          lastSignCode = "S";
        else if(t == "YIELD" || t == "Y")    lastSignCode = "Y";
        else if(t == "30" || t == "SPEED_LIMIT_30") lastSignCode = "30";
        else if(t == "50" || t == "SPEED_LIMIT_50") lastSignCode = "50";
      }
    }

    if(lastSignCode.length() == 0){
      Serial.println("[TAG] Cannot write: missing signCode");
      btManager.sendData("TAG_WRITE:NO_DATA");
    } else {
      char buf[16];
      if(hasY){
        snprintf(buf, sizeof(buf), "%s%d", lastSignCode.c_str(), currentY);
      } else {
        strncpy(buf, lastSignCode.c_str(), sizeof(buf));
        buf[sizeof(buf)-1] = '\0';
      }
      writePayload = String(buf);
      Serial.print("[TAG] Write mode ON, payload=");
      Serial.println(writePayload);
      rfidWriteMode = true;
      muteConsoleUntil = millis() + 5000; // 5 secunde fără spam în consolă
    }
  }

  // --- Comenzi semn ---
  if(command == "STOP"){
    lastSignCode = "S";
  } else if(command == "YIELD"){
    lastSignCode = "Y";
  } else if(command == "SPEED_LIMIT_30" || command == "30"){
    lastSignCode = "30";
  } else if(command == "SPEED_LIMIT_50" || command == "50"){
    lastSignCode = "50";
  }
  // === Control motor ===
    if (command == "F") {
      DCMotor(true, false);                 // înainte
    } else if (command == "B") {
      DCMotor(false, true);                // înapoi
    } else if (command == "N") {
      DCMotor(false, false);               // neutral motor, direcţia rămâne nemodificată

    // === Control direcţie ===
    } else if (command == "L") {
      ServoMotor(LEFT);                    // viraj stânga
    } else if (command == "R") {
      ServoMotor(RIGHT);                   // viraj dreapta
    } else if (command == "C") {
      ServoMotor(CENTER);                  // centru direcţie, motorul rămâne nemodificat

    // === Stop total ===
    } else if (command == "S") {
      DCMotor(false, false);
      ServoMotor(CENTER);
    }
  }

  readSensorsSequentially();
  // Actualizează logica de detectare accident
  AccidentDetector_update();

  String sensorData = String(distanceFront) + "," + 
                     String(distanceBack) + "," + 
                     String(distanceLeft) + "," + 
                     String(distanceRight);

  btManager.sendData(sensorData);


  // === Date de orientare de la Arduino UNO ===
  if(Serial1.available()){
    String line = Serial1.readStringUntil('\n');
    line.trim();
    // parsam X si Y
    int x=0,y=0;
    if(sscanf(line.c_str(), "Counter: %*d, X: %d, Y: %d", &x, &y) == 2){
      currentY = y;
      hasY = true;
    }
  }

  ///Serial.println(mesaj);
  
  // Actualizare stare modul RFID
  RFIDManager_update();
  
  // === Gestionare evenimente RFID (detectare / îndepărtare) ===
  static bool cardReported = false;

  bool present = isCardPresent();
  if(present && lastCardID.length() > 0 && !cardReported){
    // Semnalăm o singură dată când intră în câmp
    char signCode = getSignCode(lastCardID);
    if(signCode == 'S'){
      lastStopSeen = millis();
      if(!stopMode && !stopLatched){
        stopLatched = true;
        Serial.println("[STOP] Semn STOP detectat – oprire 5 secunde");
        DCMotor(false,false);
        ServoMotor(CENTER);
        stopMode = true;
        stopEnd  = millis() + 5000;
      }
    } else if(signCode == 'Y'){
      if(!yieldMode){
        yieldMode = true;
        Serial.println("[YIELD] Semn CEDEAZĂ detectat – reducere viteză");
        MOTOR_SPEED = YIELD_SPEED;
        ledcWrite(PIN_MOTOR_ENA, MOTOR_SPEED);
      }
    }
    if(!rfidWriteMode){
      Serial.print("Card RFID detectat: ");
      Serial.println(lastCardID);
      btManager.sendData("RFID:" + lastCardID);
    }
    cardReported = true;
  }
  else if(!present && cardReported){
    // Dacă părăseşte zona YIELD revenim la viteza normală
    if(yieldMode){
      yieldMode = false;
      Serial.println("[YIELD] Tag CEDEAZĂ îndepărtat – revenire viteză normală");
      MOTOR_SPEED = NORMAL_MOTOR_SPEED;
      ledcWrite(PIN_MOTOR_ENA, MOTOR_SPEED);
    }
    // Tag-ul a plecat din rază -> notificăm o singură dată
    if(!rfidWriteMode){
      Serial.print("Tag-ul: "); Serial.print(lastCardID); Serial.println(" a fost indepartat");
      btManager.sendData("RFID_REMOVED:" + lastCardID);
    }
    cardReported = false;
  }

  // === Execută scriere EPC dacă este în writeMode ===
  if(rfidWriteMode){
    uint8_t bytes[12] = {0};
    uint8_t Lstr = min((uint8_t)writePayload.length(), (uint8_t)12);
    memcpy(bytes, writePayload.c_str(), Lstr);      // rest rămân 0
    Serial.print("[TAG] Writing EPC: "); Serial.println(writePayload);
    bool ok = RFID_writeEpc(bytes, 12);            // scriem 6 words complete
    String notif = ok ? "TAG_WRITE:OK" : "TAG_WRITE:ERR";
    Serial.print("[BLE] sendData: "); Serial.println(notif);
    btManager.sendData(notif);
    rfidWriteMode = false;
  }
  
  delay(10);
}




