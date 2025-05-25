#include "DCMotor.h"

// Definirea constantelor
const int PIN_MOTOR_IN1 = 5;  // Control direcție
const int PIN_MOTOR_IN2 = 18;  // Control direcție
const int PIN_MOTOR_ENA = 21;  // ENABLE
const int PWM_CHANNEL = 2;     // Canal PWM
const int PWM_FREQ = 30000;    // Frecvență 30KHz
const int PWM_RESOLUTION = 8;  // Rezoluție 8 biți (0-255)
const int MOTOR_SPEED = 190;  // Viteza maximă (0-255)


// Variabile de stare
bool isMovingForward = false;
bool isMovingBackward = false;

void DCMotor_init() {
  Serial.println("\nInițializare Motor DC... ");
  ledcAttachChannel(PIN_MOTOR_ENA, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL);
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  delay(1000);
}

void DCMotor(bool forward, bool backward) {

  if (forward && !backward) {
    // Mișcare înainte
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    ledcWrite(PIN_MOTOR_ENA, MOTOR_SPEED);
    Serial.println("DCMotor: Înainte");
  }
  else if (!forward && backward) {
    // Mișcare înapoi
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
    ledcWrite(PIN_MOTOR_ENA, MOTOR_SPEED);
    Serial.println("DCMotor: Înapoi");
  }
  else {
    // Oprire
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    ledcWrite(PIN_MOTOR_ENA, 0);
    Serial.println("DCMotor: Oprit");
  }
  
  isMovingForward = forward;
  isMovingBackward = backward;
}
