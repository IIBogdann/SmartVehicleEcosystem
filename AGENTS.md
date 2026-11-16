# Reguli pentru asistenții proiectului

Acest fișier se aplică întregului repository. Orice asistent trebuie să îl citească integral înainte de a modifica proiectul.

## Surse obligatorii de context

Înainte de lucru:

1. citește `PROJECT_MEMORY.md`;
2. citește `CONTRIBUTING.md`;
3. pentru orice comunicare între componente, citește `docs/PROTOCOLS.md`;
4. verifică ramura și `git status`;
5. inspectează schimbările locale existente și nu le suprascrie.

Codul este sursa de adevăr pentru comportamentul implementat. Documentele de licență și copiile istorice pot descrie funcții planificate, dar nu dovedesc că acestea există.

## Baza proiectului

- Baza mașinuței provine din commitul `255eb346d1274252a85744bd054145d6cfc5c5ca`. Sursa semnelor a fost recuperată separat. Citește [baza hardware](docs/HARDWARE_BASELINE.md) pentru proveniență, opțiuni de compilare și limitele verificării față de flash.
- Aplicația mașinuței importată în monorepo: commit `aa017c153520ad6310e258ffeb054cfa49a7dc53` din fostul repository imbricat.
- Nu recupera automat cod din `ANEXE` sau din directoarele istorice de pe D:. Acele copii conțin curățări de comentarii, fișiere corupte și build-uri copiate.
- Dacă recuperarea istorică este necesară, compară explicit cu Git și documentează de unde provine fiecare fișier.

## Harta repository-ului

- `firmware/vehicle/` — ESP32 și Arduino pentru mașinuță.
- `firmware/traffic-signs/` — semnul 1 și semnul 2.
- `mobile/vehicle-app/` — aplicația Android a mașinuței.
- `mobile/traffic-signs-app/` — aplicația Android a semnelor și fluxul de scriere RFID.
- `docs/PROTOCOLS.md` — interfețele comune.
- `PROJECT_MEMORY.md` — decizii, stare și riscuri cunoscute.

Nu crea directoare goale „pentru viitor”. Un director se adaugă când există primul fișier real care îi justifică existența.

## Responsabilități și colaborare

- Bogdan răspunde în principal de `firmware/vehicle/` și `mobile/vehicle-app/`.
- Iuliana răspunde în principal de `firmware/traffic-signs/` și `mobile/traffic-signs-app/`.
- Responsabilitatea nu interzice contribuțiile în cealaltă zonă.
- O schimbare care atinge protocoale, RFID, BLE, ESP-NOW, structura repository-ului sau ambele aplicații este comună și trebuie semnalată clar în Pull Request.
- Nu presupune că doi utilizatori lucrează în același director. Fiecare lucrează în clona sa, pe propriul calculator.

## Conștientizare între cei doi asistenți locali

GitHub este memoria comună live. Nu folosi un fișier versionat pentru „cine lucrează acum”, deoarece două actualizări simultane ale lui ar produce conflicte.

La începutul fiecărei sesiuni de lucru, asistentul trebuie:

1. să citească acest fișier și `PROJECT_MEMORY.md`;
2. să ruleze `git status --short --branch` și să identifice schimbările locale existente;
3. dacă directorul este curat și începe o sarcină nouă, să actualizeze `main` cu `git pull --ff-only`;
4. să consulte Issues și Pull Request-urile deschise prin `scripts/project-status.ps1` sau GitHub CLI;
5. să confirme Issue-ul, persoana responsabilă și ramura înainte de modificări;
6. să evite fișierele pe care celălalt colaborator le modifică într-o ramură activă, dacă nu există coordonare explicită.

La finalul unei sesiuni, asistentul trebuie să lase în Issue sau Pull Request:

- ce a fost terminat;
- ce fișiere/componente au fost schimbate;
- ce verificări au trecut sau au eșuat;
- ce rămâne de făcut;
- dacă există o dependență sau o decizie necesară din partea celuilalt colaborator.

`PROJECT_MEMORY.md` se actualizează numai cu decizii și fapte stabile care au ajuns sau urmează să ajungă în `main`, nu cu progres temporar de pe o ramură.

## Reguli Git

- Nu lucra direct pe ramura stabilă.
- Creează o ramură scurtă pentru o singură problemă.
- Prefixe recomandate: `vehicle/`, `signs/`, `mobile-vehicle/`, `mobile-signs/`, `shared/`, `fix/`, `docs/`, `chore/`.
- Nu amesteca reformatarea masivă cu o schimbare funcțională.
- Nu rescrie istoricul public și nu folosi `push --force` fără acord explicit.
- Nu șterge sau restaura schimbările altui utilizator.
- Înainte de integrare, sincronizează ramura stabilă și rezolvă conflictele în propria ramură.
- Mesajele de commit trebuie să spună ce rezultat oferă schimbarea, nu doar „update” sau „fix”.
- O sarcină activă trebuie să aibă un Issue GitHub asignat și o ramură al cărei nume descrie sarcina.
- Nu lucra pe ramura altei persoane și nu împinge commituri în ea fără coordonare explicită.

## Reguli de implementare

- Păstrează schimbările mici și delimitate.
- Nu modifica un protocol doar într-o singură componentă.
- Dacă se schimbă un mesaj, UUID, canal, structură binară sau format serial:
  1. actualizează emițătorul;
  2. actualizează receptorul;
  3. actualizează testele relevante;
  4. actualizează `docs/PROTOCOLS.md`;
  5. notează decizia în `PROJECT_MEMORY.md`.
- Comentariile trebuie să explice motivul, limitarea hardware sau contractul; nu comenta fiecare linie și nu lăsa instrucțiuni de tip prompt/TODO adresate unui AI.
- Nu adăuga valori hardware presupuse. Pentru pini, adrese MAC, canale radio și baud rate, verifică sursa și documentația hardware.
- Nu declara o funcție „testată” doar pentru că proiectul compilează.

## Build-uri și verificări

Pentru fiecare schimbare, rulează cea mai apropiată verificare disponibilă:

- aplicație Android: Gradle `test` și/sau `assembleDebug` în proiectul afectat;
- firmware: compilare pentru placa reală sau, dacă mediul nu este instalat, cel puțin o verificare statică și raportarea exactă a limitării;
- protocol comun: verificare în ambele capete ale comunicației;
- documentație/structură: verifică linkurile, căile și `git status`.

În Pull Request trebuie scris ce s-a verificat și ce nu a putut fi verificat.

## Fișiere interzise în Git

Nu comite:

- `build/`, `.gradle/`, `.pio/`, `.idea/`, `.vscode/` sau cache-uri;
- `local.properties`, parole, tokenuri, chei sau configurații personale;
- APK/AAB, `.bin`, `.elf`, `.map` și alte rezultate de compilare;
- shortcuturi, copii „final”, „final2”, „backup” sau directoare duplicate.

Artefactele de livrare se publică în GitHub Releases, nu în arborele sursă.

## Memoria proiectului

Actualizează `PROJECT_MEMORY.md` când:

- se ia o decizie de arhitectură;
- se schimbă un protocol sau o responsabilitate;
- se confirmă ori se rezolvă o problemă cunoscută;
- se adaugă o componentă hardware;
- se schimbă pașii de build sau testare.

Nu transforma memoria într-un jurnal al fiecărui commit. Păstrează doar informația necesară următorului colaborator sau asistent.

## Încheierea unei sarcini

Înainte de predare:

1. verifică `git diff` și fișierele neintenționate;
2. rulează verificările relevante;
3. actualizează documentele dacă s-a schimbat comportamentul;
4. enumeră clar fișierele și comportamentele schimbate;
5. raportează riscurile și testele care necesită hardware real.
