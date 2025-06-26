
#include <ESP32Servo.h>
#include <Arduino.h>
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

// ------- RFID STOP tag EPC ------------------
const char* STOP_EPC = "E200470D88D068218CD6010E";
static bool stopMode = false;
static bool stopLatched = false;
static unsigned long stopEnd = 0;
// ---------------------------------------------
const unsigned long STOP_COOLDOWN_MS = 10000; // 10s fara semn pentru reset
static unsigned long lastStopSeen = 0;




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
  
  if (!stopMode && command.length() > 0) {
    Serial.print("Comandă primită: ");
    Serial.println(command);

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


  /*if (Serial1.available()) {
    mesaj = Serial1.readStringUntil('\n');
  }*/

  ///Serial.println(mesaj);
  
  // Actualizare stare modul RFID
  RFIDManager_update();
  
  // Verificare dacă a fost detectat un card nou
  if (isCardPresent() && lastCardID.length() > 0) {
    // STOP tag logic
    if(lastCardID.equalsIgnoreCase(String(STOP_EPC))){
      lastStopSeen = millis();
      if(!stopMode && !stopLatched){
        stopLatched = true;
        Serial.println("[STOP] Semn STOP detectat – oprire 5 secunde");
        DCMotor(false,false);
        ServoMotor(CENTER);
        stopMode = true;
        stopEnd  = millis() + 5000;
      }
    }

    Serial.print("Card RFID detectat: ");
    Serial.println(lastCardID);
    // Trimite ID-ul cardului prin Bluetooth
    btManager.sendData("RFID:" + lastCardID);
  }
  
  delay(10);
}




