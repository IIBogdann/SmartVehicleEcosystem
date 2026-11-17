# Baza firmware citită de pe plăci

Referința consemnează imaginile de aplicație citite în noiembrie 2026.
Hashurile sunt SHA-256 ale binarelor aplicațiilor. Copiile de flash și
artefactele de build sunt păstrate local, în afara repository-ului public.

| Placă | Octeți | SHA-256 |
|---|---:|---|
| Semnul 1 | 1.608.672 | `F3D84F954ACBC88BA51F526821673603686A238F8F7E91212B3181FD263CC44D` |
| Semnul 2 | 1.608.672 | `67F98E2348D83C4182AC7B5EFBD8788ACD6BB8834AD08E2DF9F9FA96113045D9` |
| ESP32 mașinuță | 1.642.832 | `18698097530B4F31E8083EF9865A816408BC062319F2E2D2207FF5539C7A07DD` |
| Arduino Uno | 8.588 | `592683241BF966D179BF9813B05760F22F4AC9DB04FFEE38A206B455CE4FB7BF` |

Pentru ESP32, imaginile au fost extrase din partiția `app0` de la
`0x10000`, cu dimensiunea de 2 MiB. S-au verificat atât checksumul,
cât și SHA-256-ul încorporat. Cele trei antete indică
`arduino-lib-builder`, versiunea `d3946f9` și ESP-IDF
`v5.5-dev-3129-gd930a386da-dirty`. Data din antet descrie
bibliotecile de build și nu stabilește data sketchului.

Pentru Arduino Uno, două citiri complete de 32 KiB au avut același hash.
Dimensiunea aplicației a fost dedusă din adresa de început și lungimea
datelor inițializate din codul de pornire AVR. Hashul din tabel acoperă
intervalul `0x0000–0x218B`, fără datele rămase ulterior în flash.

## Proveniența surselor

- **Semnele** provin din copia de lucru istorică `SmartVehicleEcosystem 1`,
  regăsită și în clona istorică cu modificări necomise peste `be9a27f`.
  Au aceeași logică, iar configurațiile separă ID-urile și numele BLE.
  Imaginile citite conțin numele BLE distincte și aceleași mesaje
  specifice de pornire, recepție și resetare ca sursele importate.
  Tabelele de partiții generate cu opțiunile de mai jos coincid cu cele
  de pe plăci. Imaginile de aplicație recompilate diferă.
- **ESP32-ul mașinuței** păstrează sursa din `main`, provenită din
  `255eb34`. Mesajele pentru oprirea și reluarea autonomă la obstacole
  apar în acest commit și în imaginea instalată. Build-ul cu
  toolchain-ul actual diferă de imaginea citită.
- **Arduino Uno** păstrează sketchul din `main`, introdus în forma
  actuală la `2a346c1`. Cei 156 de octeți ai datelor inițializate
  coincid cu build-ul actual. Codul compilat și lungimea imaginii diferă.

Acestea sunt cele mai bine susținute surse recuperabile. Pentru niciuna
dintre cele patru plăci nu avem un build reprodus octet cu octet sau
confirmarea toolchain-ului original complet.

## Compilare de referință

Din radacina repo-ului, `.\scripts\build.ps1 firmware` compileaza toate placile.
Pentru o singura placa, alege `uno`, `vehicle-esp32`, `sign-1` sau `sign-2`.
Scriptul compileaza local, fara upload.

Comenzile Arduino CLI pe care le foloseste:

```powershell
arduino-cli compile --fqbn 'esp32:esp32:XIAO_ESP32C3:PartitionScheme=no_ota' firmware/traffic-signs/traffic_sign_1
arduino-cli compile --fqbn 'esp32:esp32:XIAO_ESP32C3:PartitionScheme=noota_ffat' firmware/traffic-signs/traffic_sign_2
arduino-cli compile --fqbn 'esp32:esp32:esp32:FlashMode=dio,FlashFreq=80,PartitionScheme=no_ota' firmware/vehicle/ESP32/ElysiumRC
arduino-cli compile --fqbn 'arduino:avr:uno' firmware/vehicle/Arduino/ArduinoController
```

Mediul de referință folosește Arduino CLI 1.6.0, Arduino-ESP32 3.3.12,
Arduino AVR Boards 1.8.6, ESP32Servo 3.2.1, GxEPD2 1.6.9,
Adafruit GFX 1.13.0, Adafruit BusIO 1.17.4 și MechaQMC5883L
la commitul `eec46b7d938e21458ad888857489cd146a92241f`.
Pe ESP32-ul mașinuței, tipul exact de placă din meniul IDE rămâne
neconfirmat. Opțiunile de flash și partiția sunt preluate din imaginea
și tabela citite. Tabelele semnelor folosesc SPIFFS, respectiv FATFS.

Testarea BLE, ESP-NOW, afișajului și mișcării pe hardware se face separat
înainte de validarea unei imagini nou compilate pentru utilizare.
