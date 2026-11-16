# Firmware semne de circulație

Responsabilitate principală: Iuliana.

## Puncte de intrare

- semnul 1: `traffic_sign_1/traffic_sign_1.ino`
- semnul 2: `traffic_sign_2/traffic_sign_2.ino`

Fiecare semn conține managerul display-ului e-paper, serverul BLE și logica de recepție a alertelor.

## Stare cunoscută

- Ambele surse au fost recuperate din aceeași variantă istorică. Numele BLE și identificatorii diferă în `Config.h`.
- Receptorul comun ascultă pe canalul 6. Schemele citite sunt `no_ota` pentru semnul 1 și `noota_ffat` pentru semnul 2.
- [Baza hardware](../../docs/HARDWARE_BASELINE.md) include proveniența, hashurile imaginilor de pe plăci și comenzile de compilare.

Dependențe observate în sursă:

- ESP32 Arduino core;
- GxEPD2;
- Adafruit GFX;
- BLE și ESP-NOW din ESP32 core.

UUID-urile, comenzile și structura ESP-NOW sunt documentate în `../../docs/PROTOCOLS.md`.
