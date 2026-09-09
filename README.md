## Compilazione:
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

## Esecuzione:
```bash

./build/tappa01
./build/tappa02
./build/tappa03
./build/tappa04
./build/tappa05
./build/tappa06
./build/tappa07
./build/tappa08
```
Le tappe non richiedono argomenti aggiuntivi da riga di comando. 

## Comandi interfaccia utente:

### Mouse

| Comando | Azione |
|---|---|
| Clic con tasto sinistro | Dis/Abiltare rotazione la camera, scegliere livello |
| Clic con tasto destro | Selezionare coniglio o elemento mobile |
| Spostamento | Ruota la camera, cambia il colore per elementi selezionabili |

### Tastiera

| Comando | Azione |
|---|---|
| `H` | Help |
| `↑` / `↓` / `←` / `→` | spostare telecamera |
| `+` / `-` numpad | allontanare/avvicinare telecamera |
| `W` / `S` / `A` / `D` | spostare oggetto mobili selezionato(dal clic destro) |
| escape | ritornare al punto precedente / uscire dal programma |

se la camera non può oltrepassare oggetti, quando "tocca" gli oggetti genera una collisione (visibile dal sfondo colorato di rosso)

### note

La Tappa01 non ha comandi interattivi.
La finestra è ridimensionabile in tutte le tappe.