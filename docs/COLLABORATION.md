# Coordonarea echipei și a asistenților locali

Acest document definește cum lucrează Bogdan, Iuliana și cele două instanțe ChatGPT/Codex fără să își suprascrie munca.

## Sursele de adevăr

| Informație | Sursa corectă |
|---|---|
| Cod stabil | ramura `main` |
| Cine lucrează acum la ce | GitHub Issues asignate |
| Modificări în curs | ramurile de sarcină |
| Ce urmează să intre în `main` | Pull Request-uri |
| Decizii și probleme tehnice stabile | `PROJECT_MEMORY.md` |
| Contracte între componente | `docs/PROTOCOLS.md` |
| Reguli pentru asistenți | `AGENTS.md` |

Nu se păstrează progresul live într-un fișier Markdown comun. GitHub Issues poate fi actualizat fără conflicte și păstrează autorul, data și discuția.

## Ciclul unei sarcini

### 1. Definire

Se creează un Issue folosind șablonul `Sarcină de proiect`. Issue-ul trebuie să precizeze:

- rezultatul dorit;
- domeniul afectat;
- persoana responsabilă;
- criteriile de finalizare;
- dacă schimbarea atinge un protocol comun;
- ce hardware este necesar pentru test.

### 2. Revendicare

Persoana se asignează Issue-ului și lasă comentariul:

```text
STARTED
Branch: signs/align-espnow-channel
Scope: firmware/traffic-signs/traffic_sign_2 și docs/PROTOCOLS.md
```

Un Issue are un singur responsabil principal. Celălalt poate ajuta prin review, comentarii sau o sarcină separată.

### 3. Sincronizare locală

Înainte de o ramură nouă:

```powershell
git switch main
git pull --ff-only
.\scripts\project-status.ps1
git switch -c signs/align-espnow-channel
```

Dacă `git status` nu este curat, asistentul trebuie să înțeleagă schimbările existente înainte de orice `switch`, `pull`, mutare sau ștergere.

### 4. Implementare

- O ramură rezolvă o singură sarcină.
- Commiturile sunt atribuite contului persoanei care lucrează pe calculatorul respectiv.
- Nu se partajează aceeași ramură între calculatoare fără acord explicit.
- Schimbările comune actualizează toate capetele protocolului și `docs/PROTOCOLS.md`.

### 5. Predare prin Pull Request

Pull Request-ul leagă Issue-ul prin `Closes #NUMAR` și conține:

- rezumatul rezultatului;
- componentele schimbate;
- build-urile și testele rulate;
- ce nu a putut fi testat;
- impactul asupra celuilalt domeniu;
- capturi pentru schimbări vizuale.

### 6. Încheiere

După integrare:

```powershell
git switch main
git pull --ff-only
git branch -d numele-ramurii
```

Ramura remote poate fi ștearsă automat de GitHub. Issue-ul se închide automat dacă Pull Request-ul folosește `Closes #NUMAR`.

## Protocolul asistentului local

Mesaj recomandat la începutul unei sesiuni:

```text
Citește AGENTS.md, PROJECT_MEMORY.md și Issue-ul asociat.
Verifică ramura, git status, Issues și Pull Request-urile active.
Lucrează numai în scopul Issue-ului curent.
Nu modifica protocoale sau componenta celuilalt fără să semnalezi impactul.
La final actualizează Issue-ul/PR-ul cu rezultatul și verificările.
```

Asistentul nu trebuie să presupună că memoria sa conversațională este comună cu celălalt asistent. Numai informația publicată în Git/GitHub este comună și verificabilă.

## Când este necesară coordonarea explicită

Review-ul celuilalt este obligatoriu pentru:

- BLE, SPP și ESP-NOW;
- formatul RFID;
- telemetrie și comenzi;
- mutări sau redenumiri de directoare;
- build-uri comune și automatizări;
- `AGENTS.md`, `PROJECT_MEMORY.md` și `docs/PROTOCOLS.md`;
- orice Pull Request care modifică ambele domenii.

## Regula pentru lucru în domeniul celuilalt

Este permis. Persoana care dorește să ajute:

1. verifică Issues active;
2. evită o sarcină deja revendicată;
3. creează sau revendică alt Issue;
4. folosește prefixul ramurii după componentă, nu după numele persoanei;
5. solicită review responsabilului principal al componentei.
