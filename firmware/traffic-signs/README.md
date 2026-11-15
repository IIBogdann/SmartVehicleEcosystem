# Firmware semne de circulație

Responsabilitate principală: Iuliana.

## Puncte de intrare

- semnul 1: `traffic_sign_1/traffic_sign_1.ino`
- semnul 2: `traffic_sign_2/traffic_sign_2.ino`

Fiecare semn conține managerul display-ului e-paper, serverul BLE și logica de recepție a alertelor.

## Stare cunoscută

- Semnul 1 are implementarea ESP-NOW mai avansată și folosește explicit canalul 6.
- Semnul 2 păstrează o implementare mai veche și trebuie aliniat cu semnul 1 și mașinuța.
- Fișierele `Config.h` conțin încă valoarea 1 pentru canal; nu considera această valoare contract stabil.
- Compilarea versiunii curente trebuie verificată înainte de modificări funcționale mari.

Dependențe observate în sursă:

- ESP32 Arduino core;
- GxEPD2;
- Adafruit GFX;
- BLE și ESP-NOW din ESP32 core.

UUID-urile, comenzile și structura ESP-NOW sunt documentate în `../../docs/PROTOCOLS.md`.
