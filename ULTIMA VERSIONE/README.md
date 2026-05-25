# UNO Foca - Documentazione del codice

## Panoramica

Questo progetto implementa un gioco UNO in C usando la libreria Raylib per grafica, input e audio. Il programma offre:

- menu iniziale con accesso, registrazione e modalita ospite;
- gioco UNO contro bot in modalita `1vs1` oppure `1vs3`;
- schermata regole, impostazioni audio/musica e archivio salvataggi;
- salvataggio e ripresa delle partite per utenti registrati;
- statistiche di vittorie e sconfitte;
- pannello amministratore per visualizzare i log binari di accesso.

La finestra di gioco ha dimensione fissa `1200x800` e gli asset sono caricati dalle cartelle `Sfondo`, `Pulsanti`, `Carte`, `Animazioni` e `Musiche`.

## Tecnologie e dipendenze

Il progetto e scritto in C99 e viene compilato con CMake.

Dipendenza principale:

- Raylib

Su Fedora, come indicato in `CMakeLists.txt`, la libreria puo essere installata con:

```bash
sudo dnf install raylib-devel
```

Il target viene linkato anche con librerie di sistema usate da Raylib su Linux:

- `GL`
- `m`
- `pthread`
- `dl`
- `rt`
- `X11`

## Compilazione ed esecuzione

Dalla directory principale del progetto:

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
./cmake-build-debug/UNO
```

Il file `CMakeLists.txt` copia automaticamente nella directory di build le cartelle degli asset:

- `Musiche`
- `Sfondo`
- `Animazioni`
- `Pulsanti`
- `Carte`

Questo passaggio e necessario perche il gioco carica immagini e suoni usando percorsi relativi.

## Struttura del progetto

```text
.
├── main.c              # Ciclo principale, macchina degli stati e coordinamento moduli
├── game.c / game.h     # Regole UNO, stato partita, bot, turni e animazioni logiche
├── graphics.c / .h     # Caricamento texture e rendering della partita
├── audio.c / audio.h   # Musica, effetti sonori e mute/unmute
├── menu.c / menu.h     # Menu iniziale, login, registrazione e gestione utenti
├── menu_scelta.c / .h  # Menu dopo login/ospite: modalita, regole, archivio, impostazioni
├── archivio.c / .h     # Salvataggi, ripresa partite e statistiche utente
├── CMakeLists.txt      # Configurazione build
├── Carte/              # Texture delle carte UNO
├── Sfondo/             # Sfondi delle schermate
├── Pulsanti/           # Immagini dei pulsanti
├── Animazioni/         # Immagini finali e popup
└── Musiche/            # Musica ed effetti audio
```

## Flusso generale dell'applicazione

Il file `main.c` inizializza finestra, audio, grafica, menu e archivio. Poi entra nel ciclo principale Raylib:

1. aggiorna l'audio con `UpdateAudio()`;
2. legge mouse e tastiera;
3. aggiorna la schermata corrente tramite lo stato `StatoGioco`;
4. disegna la schermata con `BeginDrawing()` e `EndDrawing()`;
5. alla chiusura scarica texture, suoni e dispositivo audio.

Gli stati principali sono definiti in `menu.h`:

- `STATO_MENU`: schermata iniziale;
- `STATO_ACCEDI`: login;
- `STATO_REGISTRATI`: registrazione;
- `STATO_DEBUG`: pannello amministratore;
- `STATO_MENU_SCELTA`: menu principale dopo accesso o ospite;
- `STATO_GIOCO`: partita UNO;
- `STATO_REGOLE`: schermata regole;
- `STATO_IMPOSTAZIONI`: attivazione/disattivazione musica ed effetti;
- `STATO_ARCHIVIO`: elenco salvataggi;
- `STATO_STATISTICHE`: vittorie e sconfitte.

## Modulo `game`

File: `game.c`, `game.h`

Questo modulo contiene la logica della partita UNO.

### Strutture principali

`Carta`

- `colore`: valore dell'enum `Colore`, cioe `ROSSO`, `GIALLO`, `VERDE`, `BLU`, `NERO`;
- `tipo`: valore dell'enum `Tipo`, cioe `NUMERO`, `SALTA`, `INVERTI`, `PIU_DUE`, `PIU_QUATTRO`, `CAMBIA_COLORE`;
- `valore`: numero della carta, usato solo per le carte numeriche.

`Giocatore`

- contiene la mano del giocatore;
- tiene il numero di carte;
- memorizza se e gia stata notificata l'ultima carta;
- per i bot contiene la difficolta IA.

`Partita`

- contiene giocatori, mazzo, scarti e carta in cima;
- gestisce turno, direzione, colore attuale e vincitore;
- contiene flag di animazione e messaggi UI;
- conserva dati di sessione come utente corrente, ospite e data salvataggio.

### Funzioni principali

- `InizializzaPartita(Partita *p, int num_giocatori)`: resetta e prepara una nuova partita, crea il mazzo, distribuisce 7 carte e imposta la prima carta.
- `AggiornaPartita(Partita *p)`: aggiorna animazioni, messaggi, turni dei bot e fine partita.
- `GestisceInput(Partita *p)`: gestisce input del giocatore umano, pescata, passaggio turno, scelta colore e sospensione.
- `PuoGiocare(Carta c, Partita *p)`: verifica se una carta puo essere giocata sulla carta corrente.
- `GiocatoreHaMosse(Partita *p, int id_giocatore)`: controlla se un giocatore possiede almeno una carta giocabile.
- `Pesca(Partita *p)`: pesca una carta e rigenera il mazzo se necessario.
- `OttieniPosizioneMazzoGiocatore(...)`: restituisce la posizione grafica della mano di un giocatore, utile anche per le animazioni.

### Regole implementate

- carte numeriche giocabili per colore o valore;
- carte speciali giocabili per colore o tipo;
- carte nere sempre giocabili;
- `SALTA` salta il turno successivo;
- `INVERTI` cambia direzione nelle partite a piu di due giocatori, mentre in `1vs1` fa giocare ancora chi l'ha usata;
- `PIU_DUE` fa pescare 2 carte al giocatore successivo e salta il suo turno;
- `PIU_QUATTRO` fa pescare 4 carte e richiede/scelta colore;
- `CAMBIA_COLORE` cambia il colore attuale.

I bot giocano automaticamente. La difficolta facile sceglie la prima carta valida; la difficolta difficile preferisce prima carte numeriche e poi carte speciali.

## Modulo `graphics`

File: `graphics.c`, `graphics.h`

Questo modulo gestisce texture e disegno della partita.

Responsabilita principali:

- caricare sfondo, carte, immagini finali, popup e pulsante sospendi;
- fornire una texture fallback se un asset manca;
- disegnare carte con rotazione e dimensioni fisse;
- evidenziare carte giocabili e mazzo;
- disegnare bot, mano del giocatore, carta in cima e mazzo;
- disegnare popup di scelta colore, animazione deltaplano e schermata finale.

Funzioni principali:

- `CaricaSicuro(const char *path)`: carica una texture se il file esiste, altrimenti restituisce un fallback.
- `InizializzaGrafica(Grafica *gfx)`: carica tutte le texture necessarie alla partita.
- `DisegnaPartita(Grafica *gfx, Partita *p)`: renderizza l'intera schermata di gioco.
- `ScaricaGrafica(Grafica *gfx)`: scarica dalla VRAM le texture caricate.
- `DisegnaTestoPixelGrassetto(...)`: disegna testo con bordo nero simulando un effetto grassetto.

## Modulo `audio`

File: `audio.c`, `audio.h`

Questo modulo incapsula la gestione audio Raylib.

Funzioni principali:

- `InitAudio()`: inizializza il dispositivo audio e carica musica/suoni;
- `UpdateAudio()`: aggiorna lo stream della musica di sottofondo;
- `PlayBackgroundMusic()` e `StopBackgroundMusic()`: controllano la musica;
- `PlayCardSound()`, `PlayWinSound()`, `PlayDefeatSound()`: riproducono effetti;
- `ImpostaStatoMusica(bool attiva)`: abilita o disabilita la musica;
- `ImpostaStatoEffetti(bool attiva)`: abilita o disabilita gli effetti sonori;
- `CloseAudio()`: scarica risorse e chiude il dispositivo audio.

## Modulo `menu`

File: `menu.c`, `menu.h`

Gestisce la schermata iniziale, login, registrazione e pannello admin.

### Dati utente

La struttura `UtenteRegistrato` contiene:

- `nickname`;
- `password`;
- `ultimo_accesso`;
- `ultima_azione`, dove `0` indica registrazione e `1` login.

Gli utenti sono salvati in `utenti.dat`.

### Autenticazione

Funzioni interne principali:

- `EsisteNickname(...)`: controlla se un nickname e gia presente;
- `VerificaCredenzialiDettagliata(...)`: verifica login e riconosce l'admin;
- `RegistraNuovoUtenteInTabella(...)`: aggiunge un nuovo utente;
- `AggiornaTimestampAccesso(...)`: aggiorna ultimo accesso dell'utente;
- `SalvaLogStoricoAzione(...)`: aggiunge un record allo storico.

Credenziali amministratore:

- username: `admin`
- password: `1234`

Con queste credenziali il programma entra in `STATO_DEBUG`, dove `main.c` legge `registro_accessi.bin` e mostra una tabella di registrazioni e login.

## Modulo `menu_scelta`

File: `menu_scelta.c`, `menu_scelta.h`

Gestisce il menu mostrato dopo l'accesso oppure dopo l'ingresso come ospite.

Azioni possibili:

- avviare partita `1vs1`;
- avviare partita multigiocatore locale contro 3 bot;
- aprire regole;
- aprire impostazioni;
- aprire archivio, solo per utenti registrati;
- tornare al menu iniziale.

Il menu mantiene anche lo stato di:

- `musicaAttiva`;
- `audioAttivo`;
- `nascondi_archivio`, usato per impedire agli ospiti di vedere i salvataggi.

## Modulo `archivio`

File: `archivio.c`, `archivio.h`

Gestisce salvataggi, archivio storico e statistiche.

### Salvataggi

I salvataggi sono memorizzati in `salvataggi.dat` come record binari di tipo `Partita`.

Funzioni principali:

- `SalvaPartitaSospesa(Partita *partitaCorrente)`: salva la partita se l'utente non e ospite e la partita non e finita;
- `CaricaPartiteSospeseUtente(...)`: carica fino a 5 salvataggi dell'utente, ordinati dal piu recente;
- `EliminaPartitaDaArchivio(...)`: elimina il salvataggio selezionato dopo la ripresa;
- `PreparaArchivioStorico(...)`: precarica i salvataggi in memoria per evitare letture dal disco a ogni frame;
- `DisegnaArchivioStorico(...)`: disegna la schermata archivio e gestisce il pulsante riprendi.

Ogni utente puo avere al massimo `MAX_SALVATAGGI_PER_UTENTE`, cioe 5 salvataggi. Quando il limite viene superato, il codice rimuove i salvataggi piu vecchi dello stesso utente.

### Statistiche

Le statistiche sono salvate in `statistiche_utenti.dat`.

Funzioni principali:

- `SalvaRisultatoPartita(const char *username, bool haVinto)`: incrementa vittorie o sconfitte;
- `OttieniStatisticheUtente(...)`: legge vittorie e sconfitte dell'utente.

Gli ospiti non vengono registrati nelle statistiche.

## File dati generati o usati

Il progetto usa diversi file binari o dati locali:

- `utenti.dat`: archivio utenti registrati;
- `accessi_log.dat`: storico accessi/registrazioni in formato binario interno;
- `registro_accessi.bin`: log binario letto dal pannello amministratore;
- `salvataggi.dat`: partite sospese usate dall'archivio;
- `salvataggi_partite.dat`: salvataggi nel formato `RecordSalvataggio`, presenti nel codice ma non usati dal flusso archivio principale;
- `statistiche_utenti.dat`: conteggio vittorie/sconfitte;
- `storico_accessi.dat`: file presente nel progetto, non gestito direttamente dal flusso principale attuale.

Questi file non sono portabili tra versioni diverse delle strutture C se cambia la definizione di `Partita`, `UtenteRegistrato` o dei record statistici.

## Gestione degli asset

Le texture sono caricate tramite percorsi relativi, per esempio:

- `Carte/rosso_0.png`;
- `Sfondo/sfondo_partita.png`;
- `Pulsanti/sospendi.png`;
- `Animazioni/win.png`;
- `Musiche/musica.mp3`.

Per aggiungere una nuova carta o sostituire una grafica, mantenere i nomi attesi dal codice. Le carte sono caricate con convenzioni come:

- `Carte/rosso_5.png`;
- `Carte/blu_stop.png`;
- `Carte/giallo_rev.png`;
- `Carte/verde_p2.png`;
- `Carte/wild.png`;
- `Carte/wild_4.png`.

## Note di manutenzione

- Le dimensioni finestra, carta e mano massima sono definite in `game.h`.
- Le strutture vengono salvate direttamente su file binari: modificare l'ordine o la dimensione dei campi puo rendere incompatibili i vecchi salvataggi.
- Le password sono salvate in chiaro nei file locali: il sistema e adatto a un progetto didattico, non a un uso reale.
- Il rendering e legato alla risoluzione `1200x800`; per supportare risoluzioni diverse serve rendere proporzionali coordinate e layout.
- Alcune funzioni globali e variabili condivise sono usate tra `main.c` e `archivio.c` per pulsanti e sfondi; eventuali refactor dovrebbero ridurre queste dipendenze.
- `game.c` contiene anche una funzione `SalvaPartita(...)` che salva su `salvataggi_partite.dat`, mentre il flusso principale usa `SalvaPartitaSospesa(...)` da `archivio.c`.

## Flusso tipico di gioco

1. L'utente apre il programma.
2. Sceglie login, registrazione o ospite.
3. Dal menu scelta avvia `1vs1` o multigiocatore.
4. Durante la partita:
   - clicca una carta valida per giocarla;
   - clicca il mazzo per pescare;
   - se la carta pescata e giocabile puo passare cliccando di nuovo il mazzo;
   - se gioca una carta nera sceglie il nuovo colore;
   - se e registrato puo sospendere la partita.
5. A fine partita il programma aggiorna vittorie o sconfitte per l'utente registrato.
6. Dall'archivio l'utente puo riprendere una partita salvata.

## File principali da consultare

- `main.c`: punto di partenza per capire il flusso complessivo.
- `game.h`: definizioni di carte, giocatori e stato partita.
- `game.c`: regole UNO e gestione turni.
- `graphics.c`: disegno della schermata di gioco.
- `menu.c`: login, registrazione e admin.
- `archivio.c`: salvataggi e statistiche.
