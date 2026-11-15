# Protocoale și contracte între componente

Acest document descrie comportamentul existent la reorganizarea din 12 noiembrie 2026. Orice schimbare trebuie implementată în toate capetele afectate, revizuită de ambii colaboratori și notată aici.

## 1. Aplicația semnelor ↔ semne: BLE

UUID-uri:

- serviciu: `8f0e0d0c-0b0a-0908-0706-050403020100`
- comandă: `8f0e0d0c-0b0a-0908-0706-050403020101`
- stare/notificări: `8f0e0d0c-0b0a-0908-0706-050403020102`

Comenzi text cunoscute:

- `STOP`
- `YIELD`
- `SPEED_LIMIT_30`
- `SPEED_LIMIT_50`

Aplicația poate menține mai multe conexiuni, indexate după adresa dispozitivului. Codul actual trebuie îmbunătățit astfel încât starea „conectat” să fie publicată numai după descoperirea cu succes a serviciilor GATT.

## 2. Aplicații ↔ mașinuță: Bluetooth Classic SPP

UUID SPP standard:

`00001101-0000-1000-8000-00805F9B34FB`

### Control manual

- `F` — înainte
- `B` — înapoi
- `N` — motor neutru/oprit
- `L` — direcție stânga
- `R` — direcție dreapta
- `C` — direcție centrată

O comandă manuală scoate firmware-ul din modul autonom.

### Mod autonom

- `AUTO_START` — intrare în modul autonom
- `AUTO_STOP` — revenire în modul manual

### Alegerea tipului de semn

Firmware-ul acceptă `STOP`, `YIELD`, `SPEED_LIMIT_30`, `SPEED_LIMIT_50`, `30` și `50` pentru actualizarea tipului care urmează să fie scris.

### Cerere de scriere RFID

Formatul folosit de aplicația semnelor:

```json
{"action":"WRITE_TAG","type":"STOP"}
```

Valori acceptate pentru `type`:

- `STOP` sau `S`
- `YIELD` sau `Y`
- `SPEED_LIMIT_30` sau `30`
- `SPEED_LIMIT_50` sau `50`

Răspunsuri curente:

- `TAG_WRITE:OK`
- `TAG_WRITE:ERR`
- `TAG_WRITE:NO_DATA`

Observație: parserul firmware-ului caută actualmente textul `WRITE_TAG` și câmpul `"type":"..."`; nu este încă un parser JSON strict.

## 3. Telemetrie mașinuță → aplicație

### Ultrasonice

```text
$SEN,<față>,<spate>,<stânga>,<dreapta>
```

Valorile sunt distanțe în centimetri. `-1` indică o citire invalidă în implementarea curentă.

### Arduino → ESP32 → aplicație

```text
$ARD,<contor>,<x>,<y>,<z>,<tensiune>
```

Legătura Arduino–ESP32 folosește în codul curent text delimitat prin virgulă și 9600 baud. ESP32 retransmite linia prin SPP.

### RFID

```text
RFID:<epc>
RFID_REMOVED:<epc>
```

## 4. Conținut tag RFID

Firmware-ul scrie un payload text într-un EPC de 12 bytes, completat cu zero. Codurile curente încep cu:

- `S` pentru STOP;
- `Y` pentru YIELD;
- `30` pentru limită 30;
- `50` pentru limită 50.

Citirea comportamentală folosește în primul rând primul caracter. Orice extindere a formatului trebuie versionată înainte de folosirea în teren.

## 5. Mașinuță ↔ semne: ESP-NOW

Structura binară folosită de mașinuță și de implementarea avansată a semnului 1 este împachetată:

```cpp
struct traffic_message {
  uint8_t targetId;
  char signType[20];
  uint8_t priority;
};
```

Semnificație:

- `targetId = 0` — broadcast;
- `signType` — de exemplu `ACCIDENT`, `STOP` sau `OBSTACLE`;
- `priority = 1` — mesaj urgent.

Canalul folosit de mașinuță și explicit de semnul 1 este `6`. Unele fișiere de configurare ale semnelor conțin încă `1`, iar semnul 2 folosește o implementare mai veche. Până la aliniere, canalul și compatibilitatea semnului 2 nu trebuie considerate confirmate.

## Regula de schimbare

Un Pull Request care schimbă acest document trebuie să enumere:

1. emițătoarele schimbate;
2. receptoarele schimbate;
3. compatibilitatea cu versiunea anterioară;
4. testul realizat fără hardware;
5. testul realizat pe hardware real.
