
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


// ******************* GLOBAL **************************************
Servo servo;
String mesaj;
BluetoothManager btManager;
const int RXD1 = 16;
const int TXD1 = 17;
// *******************************************************************




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
  // Verificăm dacă avem comenzi de la Bluetooth
  String command = btManager.receiveData();
  
  if (command.length() > 0) {
    Serial.print("Comandă primită: ");
    Serial.println(command);
    
    // Procesăm comanda
    if (command == "F") {
      // Înainte
      DCMotor(true, false);
    }
    else if (command == "B") {
      // Înapoi
      DCMotor(false, true);
    }
    else if (command == "S") {
      // Stop motor și centrare servo
      DCMotor(false, false);
      ServoMotor(CENTER);
    }
    else if (command == "L") {
      // Virare stânga
      ServoMotor(LEFT);
    }
    else if (command == "R") {
      // Virare dreapta
      ServoMotor(RIGHT);
    }
  }

  readSensorsSequentially();

  String sensorData = String(distanceFront) + "," + 
                     String(distanceBack) + "," + 
                     String(distanceLeft) + "," + 
                     String(distanceRight);

  btManager.sendData(sensorData);


  if (Serial1.available()) {
    mesaj = Serial1.readStringUntil('\n');
  }

  Serial.println(mesaj);
  
  // Actualizare stare modul RFID
  RFIDManager_update();
  
  // Verificare dacă a fost detectat un card nou
  if (isCardPresent() && lastCardID.length() > 0) {
    Serial.print("Card RFID detectat: ");
    Serial.println(lastCardID);
    // Trimite ID-ul cardului prin Bluetooth
    btManager.sendData("RFID:" + lastCardID);
  }
  
  delay(10);
}




