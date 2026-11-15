# Firmware mașinuță

Responsabilitate principală: Bogdan.

## Puncte de intrare

- ESP32: `ESP32/ElysiumRC/ElysiumRC.ino`
- Arduino: `Arduino/ArduinoController/ArduinoController.ino`

## Module ESP32

- `core/` — taskuri, Bluetooth și logica autonomă;
- `motion-control/` — motor DC și servodirecție;
- `sensors/` — ultrasonice și RFID YRM1003;
- `feedback/` — buzzer;
- `alerts/` — detectarea și transmiterea accidentului prin ESP-NOW.

Fișierul `ESP32/ElysiumRC/Modules.cpp` include implementările modulelor în sketchul Arduino. Căile relative interne au fost păstrate în reorganizare.

## Verificare

Versiunile exacte ale plăcii și bibliotecilor nu sunt încă fixate în repository. La prima compilare reușită pe hardware, acestea trebuie adăugate aici și în `PROJECT_MEMORY.md`.

Dependențe observate în sursă:

- ESP32 Arduino core;
- ESP32Servo;
- ESP-NOW/WiFi din ESP32 core;
- MechaQMC5883 și SoftwareSerial pentru sketchul Arduino.

Protocoalele comune sunt descrise în `../../docs/PROTOCOLS.md`.
