#include <Wire.h>
#include <MechaQMC5883.h>
#include <SoftwareSerial.h>


SoftwareSerial Serial1(10, 11); 

#define outputA 2  // Pin INT0 pentru encoder
#define outputB 3  // Pin INT1 pentru encoder

int offset =20;// set the correction offset value

volatile int counter = 0; // Contor pentru encoder
MechaQMC5883 qmc;         // Busolă

void encoderA() {
  if (digitalRead(outputA) != digitalRead(outputB)) {
    counter++;
  } else {
    counter--;
  }
}

void encoderB() {
  if (digitalRead(outputA) == digitalRead(outputB)) {
    counter++;
  } else {
    counter--;
  }
}

void setup() {

  Serial.begin(115200);
 



  pinMode(outputA, INPUT_PULLUP);  // Rezistențe pull-up interne
  pinMode(outputB, INPUT_PULLUP);

  Serial1.begin(9600);   // Transmisie către ESP32
  Serial1.println("Test comunicare Arduino Uno cu ESP32");
  delay(5000);
  Wire.begin();         // Inițializare bus I2C pentru busolă
  qmc.init();           // Inițializare QMC5883

  // Configurarea întreruperilor pentru encoder
  attachInterrupt(digitalPinToInterrupt(outputA), encoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(outputB), encoderB, CHANGE);
}

void loop() {
  int x, y, z;
  qmc.read(&x, &y, &z); // Citirea valorilor busolei\


   int volt = analogRead(A0);// read the input
  double voltage = map(volt,0,1023, 0, 2500) + offset;// map 0-1023 to 0-2500 and add correction offset
  
  voltage /=100;// divide by 100 to get the decimal values

  // Transmite datele către ESP32
  Serial.print("Counter: ");
  Serial.print(counter);
  Serial.print(", X: ");
  Serial.print(x);
  Serial.print(", Y: ");
  Serial.print(y);
  Serial.print(", Z: ");
  Serial.print(z);
  Serial.print(" Voltage:");
  Serial.println(voltage);
  

  // Transmisie compactă pentru ESP32 (parsează $ARD, cnt,x,y,z,v)
  Serial1.print("$ARD,");
  Serial1.print(counter);
  Serial1.print(',');
  Serial1.print(x);
  Serial1.print(',');
  Serial1.print(y);
  Serial1.print(',');
  Serial1.print(z);
  Serial1.print(',');
  Serial1.println(voltage,2);




  delay(100); // Pauză pentru transmisie
}
