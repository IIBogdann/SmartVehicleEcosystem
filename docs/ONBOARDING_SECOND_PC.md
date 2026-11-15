# Configurarea celui de-al doilea calculator

Acest ghid se folosește numai după ce repository-ul reorganizat este publicat și ramura implicită este `main`.

## 1. Contul GitHub

Pe contul Iulianei:

1. verifică adresa de email;
2. activează autentificarea în doi pași;
3. salvează recovery codes într-un loc sigur;
4. acceptă invitația la `IIBogdann/SmartVehicleEcosystem`.

Conturile și parolele nu se partajează. Fiecare commit trebuie atribuit persoanei care l-a făcut.

## 2. Uneltele minime

Pe calculatorul Iulianei se instalează:

- Git for Windows, cu Git Credential Manager;
- GitHub CLI (`gh`);
- aplicația locală ChatGPT/Codex folosită pentru dezvoltare;
- Android Studio și Android SDK pentru aplicația semnelor;
- toolchain-ul Arduino/ESP32 stabilit în documentația proiectului.

Versiunile exacte ale mediului vor fi fixate după pregătirea completă a calculatorului principal.

## 3. Identitatea Git

În PowerShell:

```powershell
git config --global user.name "Numele complet al Iulianei"
git config --global user.email "email-verificat-in-github"
```

Verificare:

```powershell
git config --global user.name
git config --global user.email
```

## 4. Autentificarea GitHub

Varianta recomandată:

```powershell
gh auth login
```

Se aleg:

- `GitHub.com`;
- `HTTPS`;
- autentificare în browser;
- configurarea Git pentru folosirea acelorași credențiale.

Nu se introduc parole sau tokenuri în fișierele proiectului ori în conversațiile cu asistentul.

## 5. Clonarea

Într-un folder local, nu într-un share de rețea:

```powershell
git clone https://github.com/IIBogdann/SmartVehicleEcosystem.git
cd SmartVehicleEcosystem
git switch main
.\scripts\project-status.ps1
```

## 6. Prima sesiune cu asistentul local

Mesaj recomandat:

```text
Citește integral AGENTS.md, PROJECT_MEMORY.md, CONTRIBUTING.md și docs/COLLABORATION.md.
Eu sunt Iuliana și răspund în principal de firmware-ul semnelor și aplicația semnelor.
Verifică git status, Issues și Pull Request-urile înainte de orice modificare.
Nu începe lucrul fără un Issue asignat și o ramură separată.
```

Asistentul trebuie lăsat să indexeze repository-ul înainte de prima sarcină.

## 7. Începerea unei sarcini

1. deschide sau revendică un GitHub Issue;
2. asignează contul Iulianei;
3. notează în Issue numele ramurii;
4. actualizează `main`;
5. creează ramura sarcinii.

Exemplu:

```powershell
git switch main
git pull --ff-only
git switch -c signs/fix-display-manager
```

## 8. Încheierea unei sarcini

```powershell
git status
git diff
git add <fisierele-verificate>
git commit -m "Fix traffic sign display manager"
git push -u origin signs/fix-display-manager
```

Se deschide un Pull Request către `main`, se completează șablonul și se leagă Issue-ul cu `Closes #NUMAR`.

## Reguli esențiale

- Nu lucra direct pe `main`.
- Nu lucra pe aceeași ramură de pe două calculatoare.
- Nu începe o sarcină deja asignată altcuiva.
- Este permis să lucrezi în orice componentă, dar responsabilul principal al componentei face review.
- Schimbările de protocol se coordonează și actualizează `docs/PROTOCOLS.md`.
- Build-urile și cache-urile nu se comit.
- Memoria conversațională a celor două instanțe ChatGPT nu este comună; GitHub și fișierele proiectului sunt memoria comună.
