# Firmware mașinuță

Responsabilitate principală: Bogdan.

## Puncte de intrare

- ESP32: `ESP32/ElysiumRC/ElysiumRC.ino`
- Arduino: `Arduino/ArduinoController/ArduinoController.ino`

`ESP32/core/` contine taskurile, Bluetooth-ul si mersul autonom. In `ESP32/sensors/` sunt ultrasonicele si cititorul RFID YRM1003.

`ESP32/ElysiumRC/Modules.cpp` include implementarile din directoarele vecine pentru ca Arduino sa le compileze impreuna cu sketchul.

[Build si setari de flash](../../docs/HARDWARE_BASELINE.md) · [Protocoale](../../docs/PROTOCOLS.md)
