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

[Baza hardware](../../docs/HARDWARE_BASELINE.md) documentează opțiunile de flash citite, versiunile mediului de referință și hashurile imaginilor instalate.

Dependențe observate în sursă:

- ESP32 Arduino core;
- ESP32Servo;
- ESP-NOW/WiFi din ESP32 core;
- MechaQMC5883 și SoftwareSerial pentru sketchul Arduino.

Protocoalele comune sunt descrise în `../../docs/PROTOCOLS.md`.
