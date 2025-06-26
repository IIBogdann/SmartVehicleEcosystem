#include "ServoMotor.h"

// Definirea constantelor

int currentServoAngle = CENTER;


void ServoMotor_init() {

  Serial.println("\nInițializare servo... ");
  servo.attach(SERVO_PIN);
  Serial.println("Setare poziție inițială la centru... ");
  currentServoAngle = CENTER;
  servo.write(currentServoAngle);
  delay(1000);
}

void ServoMotor(int position) {
  if (position == currentServoAngle) {
    return; // Nu e nevoie de mișcare
  }

  Serial.print("Servo write la ");
  Serial.println(position);

  servo.write(position);   // Mutare instantanee
  currentServoAngle = position;
  delay(50);               // mic timp pentru stabilizare
}
