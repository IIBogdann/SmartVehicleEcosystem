#include "UltrasonicSensors.h"

// Declară variabilele globale
volatile long distanceFront = -1;
volatile long distanceBack  = -1;
volatile long distanceLeft  = -1;
volatile long distanceRight = -1;

void UltrasonicSensors_init() {
  // Configurarea pinilor
  Serial.println("\nConfigurare senzori ultrasonici...");
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_BACK, OUTPUT);
  pinMode(ECHO_BACK, INPUT);
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
}

long readDistanceCM(int trigPin, int echoPin) {
  // Dezactivăm toți ceilalți pini TRIG
  if (trigPin != TRIG_FRONT) digitalWrite(TRIG_FRONT, LOW);
  if (trigPin != TRIG_BACK) digitalWrite(TRIG_BACK, LOW);
  if (trigPin != TRIG_LEFT) digitalWrite(TRIG_LEFT, LOW);
  if (trigPin != TRIG_RIGHT) digitalWrite(TRIG_RIGHT, LOW);
  
  delayMicroseconds(100);
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  return (duration > 0) ? duration * 0.034 / 2 : -1;
}

void readSensorsSequentially() {
  static unsigned long lastReadTime = 0;
  static int currentSensor = 0;
  
  if (millis() - lastReadTime > 50) {
    lastReadTime = millis();
    
    switch (currentSensor) {
      case 0:
        distanceFront = readDistanceCM(TRIG_FRONT, ECHO_FRONT);
        break;
      case 1:
        distanceLeft = readDistanceCM(TRIG_LEFT, ECHO_LEFT);
        break;
      case 2:
        distanceBack = readDistanceCM(TRIG_BACK, ECHO_BACK);
        break;
      case 3:
        distanceRight = readDistanceCM(TRIG_RIGHT, ECHO_RIGHT);
        break;
    }
    
    currentSensor = (currentSensor + 1) % 4;
  }
}

String getSensorDataString() {
  return String(distanceFront) + "," + 
         String(distanceBack) + "," + 
         String(distanceLeft) + "," + 
         String(distanceRight);
}
