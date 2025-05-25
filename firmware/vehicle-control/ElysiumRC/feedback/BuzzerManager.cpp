#include "BuzzerManager.h"

void Buzzer_init() {
  Serial.println("\nConfigurare buzzer...");
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void tone(int pin, int frequency, int duration) {
  // Canal PWM 3 pentru buzzer
  ledcAttachChannel(pin, frequency, 8, 3);  // Folosim PIN, freq, resolution, channel
  ledcWrite(3, 128); // 50% duty cycle
  
  if (duration > 0) {
    delay(duration);
    noTone(pin);
  }
}

void noTone(int pin) {
  ledcWrite(3, 0);
}
