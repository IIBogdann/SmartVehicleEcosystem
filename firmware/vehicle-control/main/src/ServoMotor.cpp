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

  Serial.print("Mișcare de la ");
  Serial.print(currentServoAngle);
  Serial.print(" la ");
  Serial.println(position);
  
  // Determinăm direcția de mișcare
  int step = (position > currentServoAngle) ? 1 : -1;
  
  // Mișcăm gradual servo-ul
  while (currentServoAngle != position) {
    currentServoAngle += step;
    servo.write(currentServoAngle);
    delay(15);
  }
}
