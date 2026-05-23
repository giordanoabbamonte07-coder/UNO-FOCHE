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

static Sound suonoClickArchivio;

Texture2D btnAvanti;
Texture2D btnIndietro;
Texture2D sfondoStatistiche;
Rectangle rectAvanti;
Rectangle rectIndietro;

int vittorieUtente = 0;
int sconfitteUtente = 0;

// Struttura fissa per il salvataggio binario dei log di accesso
typedef struct {
    char user[32];
    char pass[32];
    char azione[32];
    char dataOra[32];
} RecordAccesso;

// Funzione per scrivere i log di accesso in formato BINARIO
void ScriviLogAccessoBinario(const char* tipo, const char* nickname, const char* password) {
    // Protezione: non registriamo i login dell'admin nel log degli utenti
    if (strcmp(nickname, "admin") == 0) return;

    FILE *f = fopen("registro_accessi.bin", "ab");
    if (f != NULL) {
        RecordAccesso nuovoRecord;
        memset(&nuovoRecord, 0, sizeof(RecordAccesso));

        // Copia sicura: copiamo al massimo 31 caratteri e forziamo lo zero all'indice 31
        strncpy(nuovoRecord.user, nickname, 31);
        nuovoRecord.user[31] = '\0';

        strncpy(nuovoRecord.pass, password, 31);
        nuovoRecord.pass[31] = '\0';

        strncpy(nuovoRecord.azione, tipo, 31);
        nuovoRecord.azione[31] = '\0';

        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) {
            strftime(nuovoRecord.dataOra, 31, "%Y-%m-%d %H:%M:%S", tm_info);
            nuovoRecord.dataOra[31] = '\0'; // Sicurezza extra per la data
        }

        fwrite(&nuovoRecord, sizeof(RecordAccesso), 1, f);
        fclose(f);
        printf("[DEBUG] Scritto record binario per %s (%s)\n", nickname, tipo);
    }
}

int main(void) {
    InitWindow(1200, 800, "UNO FOCA - EDIZIONE DEFINITIVA");
    SetTargetFPS(60);
    InitAudio();

    suonoClickArchivio = LoadSound("Musiche/suono_bottone.mp3");

    Partita partita;
    memset(&partita, 0, sizeof(Partita));
    partita.is_guest = true;
    strcpy(partita.utente_corrente, "Guest");

    Grafica grafica;
    MenuGrafica menuG;
    MenuScelta menuScelta;

    InizializzaGrafica(&grafica);
    InizializzaMenu(&menuG);
    InizializzaMenuScelta(&menuScelta);
    InizializzaRisorseArchivio();

    StatoGioco statoAttuale = STATO_MENU;
    bool utente_loggato = false;

    RecordAccesso logAdminTabella[40];
    int conteggioRigheLog = 0;

    while (!WindowShouldClose()) {
        UpdateAudio();
        Vector2 mousePosGenerico = GetMousePosition();

        switch (statoAttuale) {
            case STATO_GIOCO:
                GestisceInput(&partita);
                AggiornaPartita(&partita);

                if (IsKeyPressed(KEY_ESCAPE) || partita.richiedi_uscita) {
                    if (!partita.is_guest && !partita.partita_finita) {
                        SalvaPartitaSospesa(&partita);
                    }
                    StopBackgroundMusic();
                    partita.richiedi_uscita = false;
                    statoAttuale = STATO_MENU_SCELTA;
                }
                break;

            case STATO_ARCHIVIO:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_MENU_SCELTA;
                if (CheckCollisionPointRec(mousePosGenerico, rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_MENU_SCELTA;
                }
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
                    statoAttuale = STATO_ARCHIVIO;
                }
                break;

            case STATO_IMPOSTAZIONI:
                if (IsKeyPressed(KEY_ESCAPE)) statoAttuale = STATO_MENU_SCELTA;
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                    statoAttuale = STATO_MENU_SCELTA;
                }
                if (CheckCollisionPointRec(mousePosGenerico, menuScelta.rectOnMusica) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    menuScelta.musicaAttiva = !menuScelta.musicaAttiva;
                    ImpostaStatoMusica(menuScelta.musicaAttiva);
                    if (menuScelta.audioAttivo) PlaySound(suonoClickArchivio);
                }
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
                StatoGioco prossimo = AggiornaDisegnaMenu(&menuG);
                if (prossimo == STATO_MENU_SCELTA) {
                    utente_loggato = false;
                    partita.is_guest = true;
                    strcpy(partita.utente_corrente, "Guest");
                    statoAttuale = prossimo;
                } else if (prossimo != STATO_MENU) {
                    statoAttuale = prossimo;
                }
                break;
            }

            case STATO_ACCEDI:
            case STATO_REGISTRATI: {
                StatoGioco prossimo = GestisciSchermataAuth(&menuG, statoAttuale);

                // 1. Gestione Pannello Debug (Admin)
                if (prossimo == STATO_DEBUG) {
                    conteggioRigheLog = 0;
                    FILE *f = fopen("registro_accessi.bin", "rb");
                    if (f != NULL) {
                        while (conteggioRigheLog < 40 && fread(&logAdminTabella[conteggioRigheLog], sizeof(RecordAccesso), 1, f) == 1) {
                            conteggioRigheLog++;
                        }
                        fclose(f);
                    }
                    strcpy(menuG.nickname, "");
                    strcpy(menuG.password, "");
                    statoAttuale = STATO_DEBUG;
                    break;
                }

                // 2. Gestione Login / Registrazione Riuscita
                if (prossimo == STATO_MENU_SCELTA) {
                    utente_loggato = true;
                    partita.is_guest = false;
                    strncpy(partita.utente_corrente, menuG.nickname, 11);
                    partita.utente_corrente[11] = '\0';

                    // SCRITTURA LOG: Verifica se eravamo in fase di accesso o registrazione
                    if (statoAttuale == STATO_ACCEDI) {
                        ScriviLogAccessoBinario("LOGIN", menuG.nickname, menuG.password);
                    } else if (statoAttuale == STATO_REGISTRATI) {
                        ScriviLogAccessoBinario("REGISTRAZIONE", menuG.nickname, menuG.password);
                    }

                    statoAttuale = prossimo;
                } else {
                    // Se non siamo ancora loggati, rimaniamo dove siamo
                    statoAttuale = prossimo;
                }
                break;
            }

            case STATO_MENU_SCELTA: {
                menuScelta.nascondi_archivio = !utente_loggato;
                int azione = AggiornaDisegnaMenuScelta(&menuScelta);

                if (azione == VAI_1VS1) {
                    InizializzaPartita(&partita, 2);
                    partita.num_giocatori = 2;
                    partita.is_guest = !utente_loggato;
                    if (utente_loggato) strncpy(partita.utente_corrente, menuG.nickname, 11);
                    if (menuScelta.musicaAttiva) PlayBackgroundMusic();
                    statoAttuale = STATO_GIOCO;
                } else if (azione == VAI_MULTIGIOCATORE) {
                    InizializzaPartita(&partita, 4);
                    partita.num_giocatori = 4;
                    partita.is_guest = !utente_loggato;
                    if (utente_loggato) strncpy(partita.utente_corrente, menuG.nickname, 11);
                    if (menuScelta.musicaAttiva) PlayBackgroundMusic();
                    statoAttuale = STATO_GIOCO;
                } else if (azione == VAI_REGOLE) {
                    statoAttuale = STATO_REGOLE;
                } else if (azione == VAI_ARCHIVIO && utente_loggato) {
                    OttieniStatisticheUtente(partita.utente_corrente, &vittorieUtente, &sconfitteUtente);

                    // FIX BUG 1: Chiamiamo il precaricamento in memoria un istante prima di cambiare stato!
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

        // --- GESTIONE RENDERING ---
        BeginDrawing();
        ClearBackground(BLACK);

        switch (statoAttuale) {
            case STATO_GIOCO:
                DisegnaPartita(&grafica, &partita);
                break;

            case STATO_ARCHIVIO: {
                int partitaSelezionataIndice = -1;
                DisegnaArchivioStorico(partita.utente_corrente, mousePosGenerico, &partitaSelezionataIndice);

                if (partitaSelezionataIndice != -1) {
                    Partita elencoPartiteUtente[MAX_SALVATAGGI_PER_UTENTE];
                    int caricate = CaricaPartiteSospeseUtente(partita.utente_corrente, elencoPartiteUtente);

                    if (partitaSelezionataIndice < caricate) {
                        partita = elencoPartiteUtente[partitaSelezionataIndice];
                        partita.richiedi_uscita = false;
                        partita.blocco_input = false;
                        partita.animando = false;
                        partita.scegli_colore = false;
                        partita.timer_attesa = 0.0f;
                        EliminaPartitaDaArchivio(partita.utente_corrente, partitaSelezionataIndice);
                        if (menuScelta.musicaAttiva) PlayBackgroundMusic();
                        statoAttuale = STATO_GIOCO;
                    }
                }

                Color colIndietro = CheckCollisionPointRec(mousePosGenerico, rectIndietro) ? LIGHTGRAY : WHITE;
                Color colAvanti = CheckCollisionPointRec(mousePosGenerico, rectAvanti) ? LIGHTGRAY : WHITE;

                DrawTexturePro(btnIndietro, (Rectangle){0, 0, (float)btnIndietro.width, (float)btnIndietro.height}, rectIndietro, (Vector2){0,0}, 0.0f, colIndietro);
                DrawTexturePro(btnAvanti, (Rectangle){0, 0, (float)btnAvanti.width, (float)btnAvanti.height}, rectAvanti, (Vector2){0,0}, 0.0f, colAvanti);
                break;
            }

            case STATO_STATISTICHE: {
                DrawTexturePro(sfondoStatistiche, (Rectangle){ 0, 0, (float)sfondoStatistiche.width, (float)sfondoStatistiche.height }, (Rectangle){ 0, 0, 1200, 800 }, (Vector2){ 0, 0 }, 0.0f, WHITE);

                int vittorieCentroX = 445;
                int sconfitteCentroX = 755;
                int numeriY = 560;
                int dimensioneFont = 75;

                char txtVittorie[16];
                char txtSconfitte[16];
                sprintf(txtVittorie, "%d", vittorieUtente);
                sprintf(txtSconfitte, "%d", sconfitteUtente);

                int offsetVic = MeasureText(txtVittorie, dimensioneFont) / 2;
                int offsetScon = MeasureText(txtSconfitte, dimensioneFont) / 2;

                DrawText(txtVittorie, vittorieCentroX - offsetVic, numeriY, dimensioneFont, GREEN);
                DrawText(txtSconfitte, sconfitteCentroX - offsetScon, numeriY, dimensioneFont, RED);

                Color colIndietro = CheckCollisionPointRec(mousePosGenerico, rectIndietro) ? LIGHTGRAY : WHITE;
                DrawTexturePro(btnIndietro, (Rectangle){0, 0, (float)btnIndietro.width, (float)btnIndietro.height}, rectIndietro, (Vector2){0,0}, 0.0f, colIndietro);
                break;
            }

            case STATO_REGOLE: {
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
                DrawRectangle(0, 0, 1200, 800, (Color){ 20, 24, 30, 255 });
                DrawText("PANNELLO AMMINISTRATORE - TABELLA MONITORAGGIO UTENTI (BINARIO)", 40, 25, 26, GOLD);
                DrawLine(40, 65, 1160, 65, GRAY);

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
                    for (int i = 0; i < conteggioRigheLog; i++) {
                        Color coloreRecord = strcmp(logAdminTabella[i].azione, "REGISTRAZIONE") == 0 ? GREEN : SKYBLUE;

                        DrawText(logAdminTabella[i].user, startX_User, rigaY, 16, WHITE);
                        DrawText(logAdminTabella[i].pass, startX_Pass, rigaY, 16, LIGHTGRAY);
                        DrawText(logAdminTabella[i].azione, startX_Azione, rigaY, 16, coloreRecord);
                        DrawText(logAdminTabella[i].dataOra, startX_Data, rigaY, 16, WHITE);

                        DrawLine(40, rigaY + 22, 1160, rigaY + 22, (Color){ 50, 50, 50, 150 });
                        rigaY += 28;
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

        EndDrawing();
    }

    if (statoAttuale == STATO_GIOCO && !partita.partita_finita && !partita.is_guest) {
        SalvaPartitaSospesa(&partita);
    }

    UnloadSound(suonoClickArchivio);
    ScaricaGrafica(&grafica);
    ScaricaMenu(&menuG);
    DeinizializzaMenuScelta(&menuScelta);
    ScaricaRisorseArchivio();
    CloseAudio();
    CloseWindow();

    return 0;
}