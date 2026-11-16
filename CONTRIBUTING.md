# Fluxul de colaborare

## Principiul de bază

Fiecare lucrează pe calculatorul său, într-o clonă proprie a repository-ului public de pe GitHub. Folderul repository-ului nu se partajează pentru editare prin rețeaua locală.

Pentru protocolul complet dintre cele două persoane și cele două instanțe ChatGPT/Codex, vezi `docs/COLLABORATION.md`.

## O singură configurare inițială

1. Repository-ul este public pe GitHub. Dump-urile hardware și configurațiile personale se păstrează local.
2. Bogdan o adaugă pe Iuliana drept collaborator.
3. Ramura stabilă se numește `main`.
4. Se activează protecția pentru `main`:
   - integrare numai prin Pull Request;
   - minimum o aprobare pentru schimbările comune;
   - conversațiile din review trebuie rezolvate;
   - ștergerea și force-push-ul sunt dezactivate.
5. Fiecare clonează repository-ul în propriul folder local.

## Începerea unei sarcini

Actualizează mai întâi ramura stabilă:

```powershell
git switch main
git pull --ff-only
```

Verifică apoi starea comună:

```powershell
.\scripts\project-status.ps1
```

Înainte de a scrie cod, creează sau revendică un Issue, asignează persoana responsabilă și notează numele ramurii în Issue.

Creează apoi o ramură cu nume descriptiv:

```powershell
git switch -c vehicle/fix-autonomous-resume
```

Exemple:

- `vehicle/fix-rfid-ack`
- `signs/align-espnow-channel`
- `mobile-vehicle/add-connection-state`
- `mobile-signs/require-spp-for-tag-write`
- `shared/version-radio-protocol`
- `docs/update-hardware-map`

Nu folosi ramuri permanente cu numele persoanei și nu combina mai multe obiective fără legătură.

## În timpul lucrului

- Fă commituri mici, în puncte în care proiectul este coerent.
- Nu comite rezultate de build sau configurații locale.
- Dacă apare lucru neplanificat, deschide o sarcină separată în loc să îl amesteci.
- Pentru o schimbare de protocol, coordonează înainte cu celălalt colaborator.
- Cere asistentului să citească `AGENTS.md` și `PROJECT_MEMORY.md` înainte să modifice codul.

Exemple de mesaje de commit:

```text
Fix autonomous resume after STOP timeout
Require active SPP connection before RFID write
Align sign 2 ESP-NOW payload with vehicle
Document verified ESP32 pin mapping
```

## Publicarea și Pull Request-ul

```powershell
git push -u origin vehicle/fix-autonomous-resume
```

Deschide un PR catre `main`. Spune pe scurt ce ai schimbat, de ce si ce ai verificat. Daca mai trebuie test pe placa sau se schimba un protocol, spune asta. Pune o captura daca ajuta la review.

## Când este obligatorie verificarea celuilalt

Este obligatorie pentru:

- orice fișier din `docs/PROTOCOLS.md`;
- UUID-uri BLE, canal sau payload ESP-NOW;
- comenzile SPP și formatele de telemetrie;
- fluxul RFID dintre aplicația semnelor și mașinuță;
- mutări de directoare și schimbări ale structurii proiectului;
- schimbări care ating ambele domenii.

Pentru o modificare izolată într-o singură componentă, aprobarea poate fi rapidă, dar build-ul relevant trebuie raportat.

## Evitarea conflictelor

- Anunțați în ce ramură și componentă lucrați.
- Evitați să modificați simultan același fișier.
- Sincronizați ramura cu `main` înainte de integrare.
- Persoana care a scris schimbarea rezolvă conflictul, deoarece îi înțelege intenția.
- Nu acceptați automat „ours” sau „theirs” pentru fișiere de protocol.

## Folosirea ChatGPT/Codex

La începutul fiecărei sesiuni, mesajul ar trebui să includă:

```text
Citește AGENTS.md și PROJECT_MEMORY.md.
Lucrez pe ramura <nume>.
Scopul este <un singur rezultat>.
Nu modifica alte componente fără să explici de ce.
Rulează verificările relevante și actualizează memoria numai dacă se schimbă o decizie stabilă.
```

Înainte de a accepta modificările asistentului, verificați `git diff` și rezultatele build-ului.

## Publicarea versiunilor

APK-urile și firmware-ul compilat nu se comit în repository. Pentru o versiune testată:

1. creați un tag, de exemplu `v0.2.0`;
2. creați un GitHub Release;
3. atașați acolo APK-urile și firmware-ul;
4. notați hardware-ul și commitul pe care au fost testate.
