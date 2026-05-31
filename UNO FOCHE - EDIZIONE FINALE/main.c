#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"
#include "menu.h"
#include "menu_scelta.h"
#include "archivio.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

// Effetto sonoro locale per la navigazione nell'archivio
static Sound suonoClickArchivio;

// Asset grafici globali per la schermata dell'archivio e delle statistiche
Texture2D btnAvanti;
Texture2D btnIndietro;
Texture2D sfondoStatistiche;
Rectangle rectAvanti;
Rectangle rectIndietro;

// Variabili globali per memorizzare i dati di gioco del profilo attivo
int vittorieUtente = 0;
int sconfitteUtente = 0;

// Struttura fissa a dimensione costante per il salvataggio binario dei log di accesso
typedef struct {
    char user[32];      // Nome utente (max 31 caratteri + terminatore)
    char pass[32];      // Password (max 31 caratteri + terminatore)
    char azione[32];    // Tipo di operazione: "LOGIN" o "REGISTRAZIONE"
    char dataOra[32];   // Timestamp formattato come stringa
} RecordAccesso;

// Funzione per appendere i log di accesso in formato BINARIO all'interno del file di registro
void ScriviLogAccessoBinario(const char* tipo, const char* nickname, const char* password) {
    // Protezione: i movimenti dell'amministratore non vengono inseriti nel log utenti
    if (strcmp(nickname, "admin") == 0) return;

    // Apertura in modalità "ab" (Append Binary): scrive in coda senza sovrascrivere il file
    FILE *f = fopen("registro_accessi.bin", "ab");
    if (f != NULL) {
        RecordAccesso nuovoRecord;
        memset(&nuovoRecord, 0, sizeof(RecordAccesso)); // Inizializza la struttura azzerando la memoria

        // Copia sicura dei dati con inserimento forzato del carattere terminatore '\0'
        strncpy(nuovoRecord.user, nickname, 31);
        nuovoRecord.user[31] = '\0';

        strncpy(nuovoRecord.pass, password, 31);
        nuovoRecord.pass[31] = '\0';

        strncpy(nuovoRecord.azione, tipo, 31);
        nuovoRecord.azione[31] = '\0';

        // Recupero del tempo di sistema attuale e formattazione della stringa data/ora
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) {
            strftime(nuovoRecord.dataOra, 31, "%Y-%m-%d %H:%M:%S", tm_info);
            nuovoRecord.dataOra[31] = '\0'; // Sicurezza stringa per la data
        }

        // Scrittura dell'intero blocco struct nel file binario
        fwrite(&nuovoRecord, sizeof(RecordAccesso), 1, f);
        fclose(f); // Chiusura del file stream
        printf("[DEBUG] Scritto record binario per %s (%s)\n", nickname, tipo);
    }
}

int main(void) {
    // Inizializzazione della finestra di gioco (Risoluzione 1200x800) e del comparto audio
    InitWindow(1200, 800, "UNO FOCA - EDIZIONE DEFINITIVA");
    SetTargetFPS(60); // Limitazione del framerate a 60 FPS stabili
    InitAudio();

    // Caricamento del suono del click dedicato all'archivio dal relativo percorso
    suonoClickArchivio = LoadSound("Musiche/suono_bottone.mp3");

    // Inizializzazione della struttura di stato della partita corrente
    Partita partita;
    memset(&partita, 0, sizeof(Partita));
    partita.is_guest = true; // Di base il gioco parte in modalità ospite
    strcpy(partita.utente_corrente, "Guest");

    // Dichiarazione delle strutture di gestione dei vari menu e della grafica di gioco
    Grafica grafica;
    MenuGrafica menuG;
    MenuScelta menuScelta;

    // Allocazione delle risorse grafiche e testuali dei rispettivi moduli
    InizializzaGrafica(&grafica);
    InizializzaMenu(&menuG);
    InizializzaMenuScelta(&menuScelta);
    InizializzaRisorseArchivio();

    // Impostazione dello stato iniziale del gioco nel Menu Principale
    StatoGioco statoAttuale = STATO_MENU;
    bool utente_loggato = false;

    // Buffer locale per memorizzare i log visualizzabili nel pannello di debug admin
    RecordAccesso logAdminTabella[40];
    int conteggioRigheLog = 0;

    // --- GAME LOOP PRINCIPALE ---
    while (!WindowShouldClose()) {
        UpdateAudio(); // Aggiornamento dei flussi di streaming audio audio
        Vector2 mousePosGenerico = GetMousePosition(); // Tracciamento continuo delle coordinate del mouse

        // ==========================================
        // PHASE 1: GESTIONE LOGICA E INPUT (UPDATE)
        // ==========================================
        switch (statoAttuale) {
            case STATO_GIOCO:
                GestisceInput(&partita);  // Elaborazione input del giocatore umano
                AggiornaPartita(&partita); // Avanzamento logica di gioco (Bot, turni, ecc.)

                // Gestione uscita forzata tramite tasto ESC o flag interno di richiesta uscita
                if (IsKeyPressed(KEY_ESCAPE) || partita.richiedi_uscita) {
                    // Se l'utente è registrato e la partita è ancora in corso, esegui il salvataggio automatico
                    if (!partita.is_guest && !partita.partita_finita) {
                        SalvaPartitaSospesa(&partita);
                    }
                    StopBackgroundMusic();
                    partita.richiedi_uscita = false;
                    statoAttuale = STATO_MENU_SCELTA; // Ritorno alla schermata di selezione modalità
                }
                break;

            case STATO_ARCHIVIO:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_MENU_SCELTA;

                // Click su pulsante "Indietro": ritorna al menu di scelta precedente
                if (CheckCollisionPointRec(mousePosGenerico, rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_MENU_SCELTA;
                }
                // Click su pulsante "Avanti": carica le statistiche globali del profilo ed avanza di schermata
                if (CheckCollisionPointRec(mousePosGenerico, rectAvanti) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    OttieniStatisticheUtente(partita.utente_corrente, &vittorieUtente, &sconfitteUtente);
                    statoAttuale = STATO_STATISTICHE;
                }
                break;

            case STATO_STATISTICHE:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_ARCHIVIO;
                if (CheckCollisionPointRec(mousePosGenerico, rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_ARCHIVIO; // Ritorno alla visualizzazione dello storico partite
                }
                break;

            case STATO_IMPOSTAZIONI:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_MENU_SCELTA;
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_MENU_SCELTA;
                }
                // Switch ON/OFF della musica di sottofondo
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectOnMusica) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    menuScelta.musicaAttiva = !menuScelta.musicaAttiva;
                    ImpostaStatoMusica(menuScelta.musicaAttiva);
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                }
                // Switch ON/OFF degli effetti sonori del gioco
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectOnAudio) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    menuScelta.audioAttivo = !menuScelta.audioAttivo;
                    ImpostaStatoEffetti(menuScelta.audioAttivo);
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                }
                break;

            case STATO_REGOLE:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_MENU_SCELTA;
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_MENU_SCELTA;
                }
                break;

            case STATO_DEBUG:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_MENU;
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_MENU;
                }
                break;

            case STATO_MENU: {
                // Aggiornamento logico e disegno integrato del menu principale iniziale
                StatoGioco prossimo = AggiornaDisegnaMenu(&menuG);
                if (prossimo == STATO_MENU_SCELTA) {
                    // Se l'utente decide di proseguire senza autenticazione (modalità Guest)
                    utente_loggato = false;
                    partita.is_guest = true;
                    strcpy(partita.utente_corrente, "Guest");
                    statoAttuale = prossimo;
                } else if (prossimo != STATO_MENU) {
                    statoAttuale = prossimo; // Navigazione verso LOGIN o REGISTRAZIONE
                }
                break;
            }

            case STATO_ACCEDI:
            case STATO_REGISTRATI: {
                // Interfaccia di inserimento credenziali
                StatoGioco prossimo = GestisciSchermataAuth(&menuG, statoAttuale);

                // 1. Condizione di sblocco Pannello Debug per l'amministratore di sistema
                if (prossimo == STATO_DEBUG) {
                    conteggioRigheLog = 0;
                    FILE *f = fopen("registro_accessi.bin", "rb"); // Lettura del file binario dei log
                    if (f != NULL) {
                        // Caricamento in memoria di massimo 40 righe di record storici
                        while (conteggioRigheLog < 40 && fread(&logAdminTabella[conteggioRigheLog], sizeof(RecordAccesso), 1, f) == 1) {
                            conteggioRigheLog++;
                        }
                        fclose(f);
                    }
                    // Reset dei campi di input testuali per motivi di sicurezza
                    strcpy(menuG.nickname, "");
                    strcpy(menuG.password, "");
                    statoAttuale = STATO_DEBUG;
                    break;
                }

                // 2. Condizione di Login o Registrazione andata a buon fine
                if (prossimo == STATO_MENU_SCELTA) {
                    utente_loggato = true;
                    partita.is_guest = false;
                    strncpy(partita.utente_corrente, menuG.nickname, 11); // Taglio di sicurezza a 11 caratteri
                    partita.utente_corrente[11] = '\0';

                    // Scrittura nel registro storico in base all'azione conclusa con successo
                    if (statoAttuale == STATO_ACCEDI) {
                        ScriviLogAccessoBinario("LOGIN", menuG.nickname, menuG.password);
                    } else if (statoAttuale == STATO_REGISTRATI) {
                        ScriviLogAccessoBinario("REGISTRAZIONE", menuG.nickname, menuG.password);
                    }

                    statoAttuale = prossimo;
                } else {
                    statoAttuale = prossimo; // Rimane nello stato di auth in caso di immissione incompleta
                }
                break;
            }

            case STATO_MENU_SCELTA: {
                // Se l'utente è un ospite (Guest), il pulsante per accedere all'archivio viene nascosto
                menuScelta.nascondi_archivio = !utente_loggato;
                int azione = AggiornaDisegnaMenuScelta(&menuScelta);

                if (azione == VAI_1VS1) {
                    InizializzaPartita(&partita, 2); // Avvia partita a 2 giocatori totali
                    partita.num_giocatori = 2;
                    partita.is_guest = !utente_loggato;
                    if (utente_loggato) strncpy(partita.utente_corrente, menuG.nickname, 11);
                    if (menuScelta.musicaAttiva) PlayBackgroundMusic();
                    statoAttuale = STATO_GIOCO;
                } else if (azione == VAI_MULTIGIOCATORE) {
                    InizializzaPartita(&partita, 4); // Avvia partita completa a 4 giocatori totali
                    partita.num_giocatori = 4;
                    partita.is_guest = !utente_loggato;
                    if (utente_loggato) strncpy(partita.utente_corrente, menuG.nickname, 11);
                    if (menuScelta.musicaAttiva) PlayBackgroundMusic();
                    statoAttuale = STATO_GIOCO;
                } else if (azione == VAI_REGOLE) {
                    statoAttuale = STATO_REGOLE;
                } else if (azione == VAI_ARCHIVIO && utente_loggato) {
                    // Ottiene preventivamente il conteggio di vittorie e sconfitte totali
                    OttieniStatisticheUtente(partita.utente_corrente, &vittorieUtente, &sconfitteUtente);

                    // FIX BUG 1: Lettura e precaricamento dell'archivio delle partite sospese in memoria un istante prima del cambio di stato
                    PreparaArchivioStorico(partita.utente_corrente);

                    statoAttuale = STATO_ARCHIVIO;
                } else if (azione == VAI_INDIETRO) {
                    statoAttuale = STATO_MENU;
                } else if (azione == VAI_IMPOSTAZIONI) {
                    statoAttuale = STATO_IMPOSTAZIONI;
                }
                break;
            }
            default: break;
        }

        // ==========================================
        // PHASE 2: GESTIONE RENDERING GRAFICO (DRAW)
        // ==========================================
        BeginDrawing();
        ClearBackground(BLACK); // Reset della tela grafica con sfondo nero di sicurezza

        switch (statoAttuale) {
            case STATO_GIOCO:
                DisegnaPartita(&grafica, &partita); // Renderizza il tavolo di gioco, carte e bot
                break;

            case STATO_ARCHIVIO: {
                int partitaSelezionataIndice = -1;
                // Disegna la griglia dello storico delle partite salvate e cattura l'eventuale selezione dell'utente
                DisegnaArchivioStorico(partita.utente_corrente, mousePosGenerico, &partitaSelezionataIndice);

                // Se l'utente clicca su una vecchia partita salvata, la ripristina all'istante
                if (partitaSelezionataIndice != -1) {
                    Partita elencoPartiteUtente[MAX_SALVATAGGI_PER_UTENTE];
                    int caricate = CaricaPartiteSospeseUtente(partita.utente_corrente, elencoPartiteUtente);

                    if (partitaSelezionataIndice < caricate) {
                        // Ripristino completo dello stato della partita estratta dal file binario
                        partita = elencoPartiteUtente[partitaSelezionataIndice];
                        partita.richiedi_uscita = false;
                        partita.blocco_input = false;
                        partita.animando = false;
                        partita.scegli_colore = false;
                        partita.timer_attesa = 0.0f;

                        // Rimozione della partita dall'elenco dei salvataggi per impedirne il riutilizzo infinito
                        EliminaPartitaDaArchivio(partita.utente_corrente, partitaSelezionataIndice);
                        if (menuScelta.musicaAttiva) PlayBackgroundMusic();
                        statoAttuale = STATO_GIOCO;
                    }
                }

                // Calcolo degli effetti hover (cambio colore al passaggio del mouse) sui bottoni di navigazione dell'archivio
                Color colIndietro = CheckCollisionPointRec(mousePosGenerico, rectIndietro) ? LIGHTGRAY : WHITE;
                Color colAvanti = CheckCollisionPointRec(mousePosGenerico, rectAvanti) ? LIGHTGRAY : WHITE;

                // Disegno proiettato delle frecce avanti/indietro dell'archivio
                DrawTexturePro(btnIndietro, (Rectangle){0, 0, (float)btnIndietro.width, (float)btnIndietro.height}, rectIndietro, (Vector2){0,0}, 0.0f, colIndietro);
                DrawTexturePro(btnAvanti, (Rectangle){0, 0, (float)btnAvanti.width, (float)btnAvanti.height}, rectAvanti, (Vector2){0,0}, 0.0f, colAvanti);
                break;
            }

            case STATO_STATISTICHE: {
                // Rendering a schermo intero dello sfondo del tabellone statistiche
                DrawTexturePro(sfondoStatistiche, (Rectangle){ 0, 0, (float)sfondoStatistiche.width, (float)sfondoStatistiche.height }, (Rectangle){ 0, 0, 1200, 800 }, (Vector2){ 0, 0 }, 0.0f, WHITE);

                // Coordinate di riferimento e allineamento per i contatori grafici delle vittorie/sconfitte
                int vittorieCentroX = 445;
                int sconfitteCentroX = 755;
                int numeriY = 560;
                int dimensioneFont = 75;

                char txtVittorie[16];
                char txtSconfitte[16];
                sprintf(txtVittorie, "%d", vittorieUtente);
                sprintf(txtSconfitte, "%d", sconfitteUtente);

                // Calcolo dell'offset per centrare perfettamente il testo numerico rispetto alle coordinate X desiderate
                int offsetVic = MeasureText(txtVittorie, dimensioneFont) / 2;
                int offsetScon = MeasureText(txtSconfitte, dimensioneFont) / 2;

                // Stampa a schermo dei valori numerici con codice colore differenziato
                DrawText(txtVittorie, vittorieCentroX - offsetVic, numeriY, dimensioneFont, GREEN);
                DrawText(txtSconfitte, sconfitteCentroX - offsetScon, numeriY, dimensioneFont, RED);

                Color colIndietro = CheckCollisionPointRec(mousePosGenerico, rectIndietro) ? LIGHTGRAY : WHITE;
                DrawTexturePro(btnIndietro, (Rectangle){0, 0, (float)btnIndietro.width, (float)btnIndietro.height}, rectIndietro, (Vector2){0,0}, 0.0f, colIndietro);
                break;
            }

            case STATO_REGOLE: {
                // Algoritmo di calcolo dell'Aspect Ratio per adattare l'immagine delle regole alla finestra senza deformarla
                if (menuScelta.sfondoRegole.id > 0) {
                    float scalaX = 1200.0f / (float)menuScelta.sfondoRegole.width;
                    float scalaY = 800.0f / (float)menuScelta.sfondoRegole.height;
                    float scalaFinale = (scalaX < scalaY) ? scalaX : scalaY;

                    float larghVisualizzata = (float)menuScelta.sfondoRegole.width * scalaFinale;
                    float altVisualizzata = (float)menuScelta.sfondoRegole.height * scalaFinale;

                    float posX = (1200.0f - larghVisualizzata) / 2.0f;
                    float posY = (800.0f - altVisualizzata) / 2.0f;

                    DrawTexturePro(menuScelta.sfondoRegole,
                        (Rectangle){ 0, 0, (float)menuScelta.sfondoRegole.width, (float)menuScelta.sfondoRegole.height },
                        (Rectangle){ posX, posY, larghVisualizzata, altVisualizzata },
                        (Vector2){ 0, 0 }, 0.0f, WHITE);
                }

                Color colIndietroRegole = CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) ? LIGHTGRAY : WHITE;
                if (menuScelta.btnIndietro.id > 0) {
                    DrawTexturePro(menuScelta.btnIndietro, (Rectangle){ 0, 0, (float)menuScelta.btnIndietro.width, (float)menuScelta.btnIndietro.height }, menuScelta.rectIndietro, (Vector2){ 0, 0 }, 0.0f, colIndietroRegole);
                }
                break;
            }

            case STATO_DEBUG: {
                // Rendering del layout scuro stile terminale per il pannello amministratore
                DrawRectangle(0, 0, 1200, 800, (Color){ 20, 24, 30, 255 });
                DrawText("PANNELLO AMMINISTRATORE - TABELLA MONITORAGGIO UTENTI (BINARIO)", 40, 25, 26, GOLD);
                DrawLine(40, 65, 1160, 65, GRAY);

                // Configurazione delle colonne della tabella log
                int startX_User = 50;
                int startX_Pass = 250;
                int startX_Azione = 450;
                int startX_Data = 700;
                int intestazioneY = 85;

                DrawText("USERNAME", startX_User, intestazioneY, 18, ORANGE);
                DrawText("PASSWORD", startX_Pass, intestazioneY, 18, ORANGE);
                DrawText("AZIONE EFFETTUATA", startX_Azione, intestazioneY, 18, ORANGE);
                DrawText("DATA E ORA", startX_Data, intestazioneY, 18, ORANGE);
                DrawLine(40, 110, 1160, 110, LIGHTGRAY);

                if (conteggioRigheLog == 0) {
                    DrawText("Nessun record di registrazione o login binario rilevato.", 60, 140, 20, LIGHTGRAY);
                } else {
                    int rigaY = 125;
                    // Iterazione e stampa a schermo dei record letti dal file binario
                    for (int i = 0; i < conteggioRigheLog; i++) {
                        // Colore dinamico: verde per utenti nuovi, azzurro per accessi standard
                        Color coloreRecord = strcmp(logAdminTabella[i].azione, "REGISTRAZIONE") == 0 ? GREEN : SKYBLUE;

                        DrawText(logAdminTabella[i].user, startX_User, rigaY, 16, WHITE);
                        DrawText(logAdminTabella[i].pass, startX_Pass, rigaY, 16, LIGHTGRAY);
                        DrawText(logAdminTabella[i].azione, startX_Azione, rigaY, 16, coloreRecord);
                        DrawText(logAdminTabella[i].dataOra, startX_Data, rigaY, 16, WHITE);

                        DrawLine(40, rigaY + 22, 1160, rigaY + 22, (Color){ 50, 50, 50, 150 });
                        rigaY += 28; // Spostamento verticale verso il basso per la riga successiva
                    }
                }

                Color colIndietroAdmin = CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) ? LIGHTGRAY : WHITE;
                if (menuScelta.btnIndietro.id > 0) {
                    DrawTexturePro(menuScelta.btnIndietro, (Rectangle){ 0, 0, (float)menuScelta.btnIndietro.width, (float)menuScelta.btnIndietro.height }, menuScelta.rectIndietro, (Vector2){ 0, 0 }, 0.0f, colIndietroAdmin);
                }
                break;
            }

            case STATO_IMPOSTAZIONI: {
                if (menuScelta.sfondoImpostazioni.id > 0) {
                    DrawTexturePro(menuScelta.sfondoImpostazioni, (Rectangle){ 0, 0, (float)menuScelta.sfondoImpostazioni.width, (float)menuScelta.sfondoImpostazioni.height }, (Rectangle){ 0, 0, 1200, 800 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
                }

                // Posizionamento fisso e riscalatura dei bottoni ON/OFF delle opzioni audio
                float onMusicaX = 550.0f;
                float onMusicaY = 240.0f;
                float onAudioX = 550.0f;
                float onAudioY = 440.0f;
                float scalaPulsantiOn = 0.15f;

                menuScelta.rectOnMusica = (Rectangle){ onMusicaX, onMusicaY, (float)menuScelta.btnOnMusica.width * scalaPulsantiOn, (float)menuScelta.btnOnMusica.height * scalaPulsantiOn };
                menuScelta.rectOnAudio = (Rectangle){ onAudioX, onAudioY, (float)menuScelta.btnOnAudio.width * scalaPulsantiOn, (float)menuScelta.btnOnAudio.height * scalaPulsantiOn };

                Color colOnMusica = CheckCollisionPointRec(mousePosGenerico, menuScelta.rectOnMusica) ? LIGHTGRAY : WHITE;
                Color colOnAudio = CheckCollisionPointRec(mousePosGenerico, menuScelta.rectOnAudio) ? LIGHTGRAY : WHITE;
                Color colIndietroImpo = CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) ? LIGHTGRAY : WHITE;

                // Rendering condizionale delle texture in base allo stato booleano (Abilitato/Disabilitato)
                if (menuScelta.musicaAttiva) {
                    if (menuScelta.btnOnMusica.id > 0) DrawTexturePro(menuScelta.btnOnMusica, (Rectangle){ 0, 0, (float)menuScelta.btnOnMusica.width, (float)menuScelta.btnOnMusica.height }, menuScelta.rectOnMusica, (Vector2){0,0}, 0.0f, colOnMusica);
                } else {
                    if (menuScelta.btnOff.id > 0) DrawTexturePro(menuScelta.btnOff, (Rectangle){ 0, 0, (float)menuScelta.btnOff.width, (float)menuScelta.btnOff.height }, menuScelta.rectOnMusica, (Vector2){0,0}, 0.0f, colOnMusica);
                }

                if (menuScelta.audioAttivo) {
                    if (menuScelta.btnOnAudio.id > 0) DrawTexturePro(menuScelta.btnOnAudio, (Rectangle){ 0, 0, (float)menuScelta.btnOnAudio.width, (float)menuScelta.btnOnAudio.height }, menuScelta.rectOnAudio, (Vector2){0,0}, 0.0f, colOnAudio);
                } else {
                    if (menuScelta.btnOff.id > 0) DrawTexturePro(menuScelta.btnOff, (Rectangle){ 0, 0, (float)menuScelta.btnOff.width, (float)menuScelta.btnOff.height }, menuScelta.rectOnAudio, (Vector2){0,0}, 0.0f, colOnAudio);
                }

                if (menuScelta.btnIndietro.id > 0) {
                    DrawTexturePro(menuScelta.btnIndietro, (Rectangle){ 0, 0, (float)menuScelta.btnIndietro.width, (float)menuScelta.btnIndietro.height }, menuScelta.rectIndietro, (Vector2){ 0, 0 }, 0.0f, colIndietroImpo);
                }
                break;
            }
            default: break;
        }

        EndDrawing(); // Fine della fase di rendering ed invio del frame alla GPU
    }

    // --- DEINIZIALIZZAZIONE E CHIUSURA DI SICUREZZA ---

    // Salvataggio di emergenza in caso di chiusura improvvisa della finestra tramite la "X" del sistema operativo
    if (statoAttuale == STATO_GIOCO && !partita.partita_finita && !partita.is_guest) {
        SalvaPartitaSospesa(&partita);
    }

    // Scaricamento delle risorse audio e grafiche dalla memoria video RAM/VRAM
    UnloadSound(suonoClickArchivio);
    ScaricaGrafica(&grafica);
    ScaricaMenu(&menuG);
    DeinizializzaMenuScelta(&menuScelta);
    ScaricaRisorseArchivio();

    // Disattivazione dei sottosistemi hardware
    CloseAudio();
    CloseWindow();

    return 0;
}