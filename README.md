# Smart Vehicle Ecosystem

Smart Vehicle Ecosystem reunește o mașinuță controlată de ESP32 și Arduino, două semne de circulație cu ESP32-C3 și e-paper și două aplicații Android.

Repository-ul este organizat pentru lucru în paralel de pe calculatoare diferite. Fiecare persoană lucrează într-o clonă proprie și într-o ramură scurtă; schimbările ajung în ramura stabilă numai prin Pull Request.

## Componente

- `firmware/vehicle/` — firmware-ul mașinuței: mișcare, senzori, autonomie, RFID, Bluetooth și alerte ESP-NOW.
- `firmware/traffic-signs/` — firmware-ul celor două semne: e-paper, BLE și ESP-NOW.
- `mobile/vehicle-app/` — aplicația Android pentru controlul și monitorizarea mașinuței.
- `mobile/traffic-signs-app/` — aplicația Android pentru administrarea semnelor și scrierea tagurilor prin cititorul montat momentan pe mașinuță.
- `docs/PROTOCOLS.md` — contractele de comunicare dintre componente.
- `PROJECT_MEMORY.md` — starea tehnică, deciziile și problemele cunoscute.
- `CONTRIBUTING.md` — fluxul Git/GitHub folosit de echipă.
- `docs/COLLABORATION.md` — protocolul live dintre cei doi colaboratori și asistenții lor locali.
- `AGENTS.md` — reguli obligatorii pentru asistenții care lucrează în repository.

## Punct de plecare

Reorganizarea pornește din versiunea finală a proiectului de licență, commitul `255eb346d1274252a85744bd054145d6cfc5c5ca` din 3 iulie 2025. Aplicația mașinuței a fost importată din commitul său confirmat `aa017c153520ad6310e258ffeb054cfa49a7dc53`.

## Înainte de lucru

1. Citește `AGENTS.md`, `PROJECT_MEMORY.md` și `CONTRIBUTING.md`.
2. Actualizează ramura stabilă.
3. Creează o ramură pentru o singură sarcină.
4. Nu comite build-uri, APK-uri, fișiere `.bin`, cache-uri sau configurații locale.
5. Dacă schimbi un protocol, actualizează simultan toate componentele afectate și `docs/PROTOCOLS.md`.

## Responsabilități uzuale

- Bogdan: mașinuța și aplicația mașinuței.
- Iuliana: semnele și aplicația semnelor.

Acestea sunt responsabilități principale, nu restricții. Oricare poate lucra în orice zonă, însă schimbările comune trebuie anunțate și verificate de amândoi.

## Licență

Proiectul păstrează licența Apache 2.0 din [LICENSE](LICENSE).
