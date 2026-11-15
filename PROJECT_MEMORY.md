# Memoria locală a proiectului

Ultima actualizare: 12 noiembrie 2026

## Scop

Acest fișier păstrează contextul tehnic stabil de care au nevoie Bogdan, Iuliana și asistenții lor. Nu înlocuiește codul, istoricul Git sau documentația protocoalelor.

## Echipa

- Bogdan: responsabilitate principală pentru mașinuță și aplicația mașinuței.
- Iuliana: responsabilitate principală pentru semne și aplicația semnelor.
- Oricare poate contribui în orice componentă.
- Modificările interfețelor comune se verifică de amândoi.

## Istoric și sursă de adevăr

- Versiunea finală găsită în proiectul de licență este commitul `255eb346d1274252a85744bd054145d6cfc5c5ca`, din 3 iulie 2025.
- Ultimul commit adaugă modul autonom și oprirea la obstacole față/spate la 30 cm, cu reluare după trei secunde.
- Ramura veche `master` de la momentul reorganizării era mai puțin avansată funcțional, în ciuda datei sale ulterioare.
- Folderul `ANEXE` nu este sursă canonică: conține în principal curățări de comentarii, dar și modificări semantice, placeholder-e invalide și surse care probabil nu compilează.
- Aplicația Android a mașinuței fusese introdusă ca gitlink fără `.gitmodules`. În reorganizare a fost importată direct din commitul confirmat `aa017c153520ad6310e258ffeb054cfa49a7dc53`.

## Arhitectura curentă

### Mașinuța

- ESP32: control motor și servo, patru senzori ultrasonici, buzzer, Bluetooth Classic SPP, RFID YRM1003, mod autonom și alertă de accident prin ESP-NOW.
- Arduino: transmite contorul, magnetometrul și tensiunea către ESP32 prin UART text.
- Aplicația mașinuței: control manual și afișarea telemetriei primite prin Bluetooth.

### Semnele

- Două dispozitive ESP32-C3 cu e-paper.
- Control din aplicația semnelor prin BLE.
- Comunicare între dispozitive și alerte de la mașinuță prin ESP-NOW.
- Semnul 1 are implementarea ESP-NOW mai avansată; semnul 2 păstrează o arhitectură mai veche și trebuie aliniat.

### Scrierea tagurilor RFID

- Există un singur cititor/scriitor fizic și este montat pe mașinuță.
- Aplicația semnelor este interfața operatorului pentru alegerea semnului și inițierea scrierii.
- Aplicația semnelor se conectează temporar la mașinuță prin Bluetooth Classic SPP și îi cere să scrie tagul.
- Aceasta este o dependență hardware intenționată, nu transferul responsabilității aplicației semnelor către Bogdan.
- Pe viitor, un al doilea writer poate înlocui transportul SPP fără a schimba fluxul operatorului, dacă interfața de scriere este păstrată abstractă.

## Comportament implementat

- Comenzi manuale vehicul: înainte, înapoi, neutru, stânga, dreapta și centrare.
- Mod autonom pornit/oprit prin `AUTO_START` și `AUTO_STOP`.
- Autonomia actuală repetă o secvență fixă de deplasări și viraje; nu înregistrează și nu reproduce trasee.
- RFID `STOP`: oprire aproximativ cinci secunde și latch/cooldown de zece secunde.
- RFID `YIELD`: reducerea vitezei cât timp tagul este prezent.
- Semne disponibile în UI: `STOP`, `YIELD`, `SPEED_LIMIT_30`, `SPEED_LIMIT_50`.
- Alerta `ACCIDENT` este transmisă prin ESP-NOW dacă un ultrasonic rămâne invalid timp de cinci secunde.

## Probleme cunoscute și priorități

### Prioritate critică

1. Verificarea completă a compilării după reorganizare, pe mediile reale Arduino/ESP32.
2. Alinierea ESP-NOW între mașinuță, semnul 1 și semnul 2. În cod apar canalele 6, 1 și 0.
3. Verificarea și corectarea semnului 2; arhitectura sa diferă de semnul 1, iar `DisplayManager.cpp` conține o metodă care pare nedeclarată în header.
4. Confirmarea fizică a scrierii RFID. Implementarea poate interpreta lipsa unui ACK valid drept succes.

### Mașinuță

- Oprirea RFID `STOP` în timpul anumitor stări autonome poate opri motorul fără să îl repornească la expirare.
- Temporizatorul de eliberare a obstacolului nu se resetează complet dacă obstacolul reapare în fereastra de trei secunde.
- O citire ultrasonică invalidă poate fi tratată ca spațiu liber de logica autonomă.
- Detectarea accidentului are cooldown de numai 100 ms și poate transmite aproximativ zece alerte pe secundă.
- Telemetria `$SEN` este trimisă mai des decât sunt actualizați individual senzorii.

### Aplicații Android

- Aplicația semnelor poate activa fluxul de scriere chiar dacă nu există o conexiune SPP validă către mașinuță.
- Permisiunea `POST_NOTIFICATIONS` este declarată, dar fluxul runtime pentru Android 13+ trebuie verificat.
- Conexiunea BLE este marcată conectată înainte ca serviciile GATT să fie complet pregătite.
- Serviciul `BleBackgroundService` există, dar nu este integrat complet în manifest și în ciclul aplicației.
- Testele automate reale lipsesc; vechile fișiere erau doar șabloane Android Studio.

## Decizii de proiect

### 2026-11-12 — repository comun curat

- Se folosește GitHub privat, nu un folder de lucru partajat în rețeaua locală.
- Fiecare persoană are clonă proprie.
- Se folosesc ramuri scurte per sarcină și Pull Request pentru integrare.
- Cele două aplicații Android rămân proiecte separate.
- Aplicația semnelor păstrează momentan UI-ul de scriere RFID, folosind writerul mașinuței prin SPP.
- Build-urile, cache-urile, backupurile și directoarele goale nu se păstrează în Git.
- Protocoalele comune sunt documentate în `docs/PROTOCOLS.md`.

### 2026-11-15 — coordonare între două instanțe ChatGPT locale

- GitHub Issues reprezintă sursa live pentru cine lucrează la ce și care este starea sarcinii.
- Pull Request-urile reprezintă evidența modificărilor, verificărilor și review-ului.
- `PROJECT_MEMORY.md` păstrează numai decizii și fapte stabile, nu progres temporar.
- Nu se folosește un fișier comun de tip `WORK_STATUS.md`, deoarece ar deveni un punct de conflict la lucru simultan.
- Fiecare asistent local trebuie să verifice Issues, Pull Request-uri, ramura și starea Git înainte să modifice proiectul.
- Scriptul `scripts/project-status.ps1` oferă o vedere locală rapidă asupra stării Git și GitHub.

### 2026-11-15 — validarea reorganizării

- `mobile/vehicle-app`: `test assembleDebug` a terminat cu succes și a produs APK-ul debug.
- `mobile/traffic-signs-app`: `test assembleDebug` a terminat cu succes și a produs APK-ul debug.
- Ambele proiecte Android nu conțin încă teste reale; taskurile de unit test raportează `NO-SOURCE`.
- Aplicația semnelor compilează cu avertismente pentru API-uri Android/Bluetooth depreciate; acestea trebuie tratate într-o sarcină separată, nu în commitul de reorganizare.
- Căile relative ale include-urilor firmware se rezolvă după mutare, iar directoarele sketchurilor păstrează numele fișierelor `.ino` principale.
- Firmware-ul nu a fost compilat încă, deoarece Arduino CLI/PlatformIO și versiunile plăcilor/bibliotecilor nu sunt încă instalate și fixate pe acest PC.

## Următoarea actualizare a memoriei

### Prioritatea următoarei sesiuni — pregătirea completă a PC-ului de dezvoltare

La următoarea sesiune nu se începe cu funcții noi. Se pregătește permanent acest calculator pentru lucru rapid și repetabil:

- inventarierea și instalarea toolchain-urilor Android, Arduino/ESP32 și PlatformIO necesare;
- configurarea persistentă și corectă a `JAVA_HOME`, Android SDK, `PATH` și cache-urilor Gradle;
- fixarea versiunilor de plăci, biblioteci și unelte folosite de proiect;
- comenzi/scripturi simple pentru build-ul fiecărei componente și pentru verificarea completă;
- reutilizarea cache-urilor pentru a reduce timpul și consumul de resurse/API;
- verificarea celor două aplicații Android și a celor trei ținte firmware;
- documentarea pașilor astfel încât asistenții să nu repete configurarea la fiecare sesiune;
- evaluarea unor verificări automate locale și GitHub Actions cu cost minim.

Obiectivul este ca după această sesiune build-urile uzuale să pornească prin comenzi scurte, fără configurări temporare repetate.

După prima sesiune cu hardware real, trebuie consemnate:

- plăcile exacte și versiunile de board package;
- pinii confirmați și conexiunile hardware;
- bibliotecile și versiunile care compilează;
- canalul ESP-NOW ales definitiv;
- adresele/configurarea dispozitivelor fără a publica date sensibile;
- rezultatele testelor pentru fiecare dintre cele patru componente.
