#include "menu.h"
#include "graphics.h"
#include "audio.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

// Definizioni di costanti per evitare "magic numbers" e rendere il codice manutenibile
#define MAX_INPUT_STR    10   // Lunghezza massima consentita per nickname e password (escluso \0)
#define BUFFER_AUTH_SIZE 12   // Dimensione effettiva dei vettori in MenuGrafica (11 + \0)

// Variabili statiche del modulo per la gestione degli errori e del comparto audio
static int codice_errore = 0;
static Sound suono_bottone;

// Struttura ausiliaria per mantenere la compatibilità con i log storici binari
typedef struct {
    char nickname[11];
    char password[11];
    char data_ora[25];
    int tipo_azione;
} LogAccesso;

// =========================================================================
// --- FUNZIONI DI SERVIZIO E GESTIONE DATABASE (FILE BINARI) ---
// =========================================================================

/**
 * Verifica se un nickname è già presente nel database ad accesso binario.
 * @param nick Stringa contenente il nickname da cercare.
 * @return true se il nickname esiste, false altrimenti.
 */
static bool EsisteNickname(const char* nick) {
    // Apertura del file in modalità lettura binaria
    FILE *file = fopen("utenti.dat", "rb");
    if (file == NULL) return false; // Se il file non esiste, il nick non può essere duplicato

    UtenteRegistrato u;
    bool trovato = false;

    // Lettura sequenziale a blocchi della dimensione della struttura record
    while (fread(&u, sizeof(UtenteRegistrato), 1, file) == 1) {
        if (strcmp(u.nickname, nick) == 0) {
            trovato = true;
            break;
        }
    }
    fclose(file); // Chiusura del descrittore del file per liberare la risorsa
    return trovato;
}

/**
 * Esegue il controllo incrociato di credenziali inserite dall'utente.
 * @return 0 se OK, 1 se Utente inesistente, 2 se Password Errata, 99 se Admin.
 */
static int VerificaCredenzialiDettagliata(const char* nick, const char* pass) {
    // Controllo prioritario credenziali di amministrazione (Hardcoded per debug e gestione)
    if (strcmp(nick, "admin") == 0 && strcmp(pass, "1234") == 0) {
        return 99;
    }

    FILE *file = fopen("utenti.dat", "rb");
    if (file == NULL) return 1; // File mancante trattato come utente non trovato

    UtenteRegistrato u;
    int risultato = 1;

    // Ciclo di scansione sequenziale del database utenti
    while (fread(&u, sizeof(UtenteRegistrato), 1, file) == 1) {
        if (strcmp(u.nickname, nick) == 0) {
            if (strcmp(u.password, pass) == 0) {
                risultato = 0; // Credenziali combaciano perfettamente
                break;
            } else {
                risultato = 2; // Nickname trovato ma password errata
                break;
            }
        }
    }
    fclose(file);
    return risultato;
}

/**
 * Registra in modalità "append" un nuovo utente nel database binario standardizzando i record.
 */
static void RegistraNuovoUtenteInTabella(const char* nick, const char* pass) {
    // Apertura in modalità "append binario" per aggiungere i record in coda
    FILE *file = fopen("utenti.dat", "ab");
    if (file != NULL) {
        UtenteRegistrato u;
        // Inizializzazione totale della memoria a 0 per evitare leak di dati residui dello stack nel file binario
        memset(&u, 0, sizeof(UtenteRegistrato));

        // Copia sicura limitata per evitare buffer overflow accidentali
        strncpy(u.nickname, nick, MAX_INPUT_STR);
        u.nickname[MAX_INPUT_STR] = '\0'; // Garantisce la chiusura della stringa

        strncpy(u.password, pass, MAX_INPUT_STR);
        u.password[MAX_INPUT_STR] = '\0';

        // Calcolo e formattazione del timestamp locale attuale
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) {
            strftime(u.ultimo_accesso, sizeof(u.ultimo_accesso) - 1, "%d/%m/%Y %H:%M:%S", tm_info);
        }

        u.ultima_azione = 0; // Convenzione interna: 0 identifica l'evento di Registrazione

        // Scrittura fisica del blocco dati su disco
        fwrite(&u, sizeof(UtenteRegistrato), 1, file);
        fclose(file);
    }
}

/**
 * Cerca un record utente e aggiorna in-place (lettura/scrittura binaria) il timestamp di login.
 */
static void AggiornaTimestampAccesso(const char* nick) {
    // Apertura in modalità rb+ per consentire l'aggiornamento mirato senza riscrivere l'intero file
    FILE *file = fopen("utenti.dat", "rb+");
    if (file == NULL) return;

    UtenteRegistrato u;
    while (fread(&u, sizeof(UtenteRegistrato), 1, file) == 1) {
        if (strcmp(u.nickname, nick) == 0) {
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            if (tm_info) {
                strftime(u.ultimo_accesso, sizeof(u.ultimo_accesso) - 1, "%d/%m/%Y %H:%M:%S", tm_info);
            }

            u.ultima_azione = 1; // Convenzione interna: 1 identifica l'evento di Accesso/Login

            // Arretra il cursore del file esattamente della dimensione del record appena letto
            fseek(file, -(long)sizeof(UtenteRegistrato), SEEK_CUR);
            fwrite(&u, sizeof(UtenteRegistrato), 1, file);
            break; // Aggiornamento eseguito, interrompe il ciclo protettivo
        }
    }
    fclose(file);
}

/**
 * Scrive l'evento storico dettagliato nel file di log separato "accessi_log.dat".
 */
static void SalvaLogStoricoAzione(const char* nick, const char* pass, int tipo_azione) {
    FILE *file = fopen("accessi_log.dat", "ab");
    if (file != NULL) {
        LogAccesso log;
        memset(&log, 0, sizeof(LogAccesso));

        strncpy(log.nickname, nick, MAX_INPUT_STR);
        log.nickname[MAX_INPUT_STR] = '\0';
        strncpy(log.password, pass, MAX_INPUT_STR);
        log.password[MAX_INPUT_STR] = '\0';

        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) {
            strftime(log.data_ora, sizeof(log.data_ora) - 1, "%d/%m/%Y %H:%M:%S", tm_info);
        }

        log.tipo_azione = tipo_azione;
        fwrite(&log, sizeof(LogAccesso), 1, file);
        fclose(file);
    }
}

// =========================================================================
// --- INPUT TASTIERA NEI CAMPI DI TESTO (INTERFACCIA UTENTE) ---
// =========================================================================

/**
 * Gestisce l'acquisizione dei caratteri da tastiera per i campi attivi mediante le API Raylib.
 */
static void GestisceInputTesto(char *buffer, int *campo_attivo, int id) {
    if (*campo_attivo == id) {
        int key = GetCharPressed();

        // Ciclo di svuotamento della coda dei caratteri premuti nel frame corrente
        while (key > 0) {
            // RISOLUZIONE BUG: Se viene premuto lo spazio nel campo password o nickname, blocca l'acquisizione
            if (key == 32) {
                codice_errore = 4; // Codice errore associato a "SPAZIO NON CONSENTITO"
            }
            // Acquisisce solo caratteri stampabili standard e controlla rigorosamente il limite del buffer
            else if ((key >= 33) && (key <= 125) && (strlen(buffer) < MAX_INPUT_STR)) {
                size_t len = strlen(buffer);
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
                codice_errore = 0; // Resetta gli errori pendenti a ogni inserimento valido
            }
            key = GetCharPressed(); // Ottiene il carattere successivo nella coda
        }

        // Gestione della cancellazione del carattere (Backspace)
        if (IsKeyPressed(KEY_BACKSPACE)) {
            size_t len = strlen(buffer);
            if (len > 0) {
                buffer[len - 1] = '\0';
                codice_errore = 0;
            }
        }
    }
}

// =========================================================================
// --- GESTIONE SCHERMATA DI AUTENTICAZIONE ---
// =========================================================================

StatoGioco GestisciSchermataAuth(MenuGrafica *mg, StatoGioco stato) {
    Vector2 mouse = GetMousePosition();
    mg->frame_counter++;

    // Parametri di rendering proporzionale per i box di input grafici
    float ottimaleLarghezza = 312.0f;
    float ottimaleAltezza = 72.0f;
    if (mg->tex_utente.id > 0) {
        ottimaleAltezza = (float)mg->tex_utente.height * (ottimaleLarghezza / (float)mg->tex_utente.width);
    }

    // Coordinate esatte dedotte dai template grafici degli sfondi di login/registrazione
    float posizionaX = 615.0f;
    float posizionaNickY = 442.0f - (ottimaleAltezza / 2.0f);
    float posizionaPassY = 568.0f - (ottimaleAltezza / 2.0f);

    Rectangle rNickDest = { posizionaX, posizionaNickY, ottimaleLarghezza, ottimaleAltezza };
    Rectangle rPassDest = { posizionaX, posizionaPassY, ottimaleLarghezza, ottimaleAltezza };

    // Rilevamento del click del mouse per lo switch del focus del campo attivo
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse, rNickDest)) mg->campo_attivo = 1;
        else if (CheckCollisionPointRec(mouse, rPassDest)) mg->campo_attivo = 2;
        else mg->campo_attivo = 0; // Click fuori dai campi rimuove il focus tastiera
    }

    // Aggiornamento dei buffer di testo interni
    GestisceInputTesto(mg->nickname, &mg->campo_attivo, 1);
    GestisceInputTesto(mg->password, &mg->campo_attivo, 2);

    // --- RENDERING GRAFICO ---
    Texture2D texSfondo = (stato == STATO_REGISTRATI) ? mg->sfondo_registrati : mg->sfondo_accedi;
    if (texSfondo.id > 0) {
        DrawTexturePro(texSfondo, (Rectangle){0, 0, (float)texSfondo.width, (float)texSfondo.height}, (Rectangle){0, 0, 1200, 800}, (Vector2){0, 0}, 0, WHITE);
    }

    // Effetto visivo "Glow" (Bordo Oro) sul campo correntemente attivo per migliorare l'UX
    float spessoreBordo = 5.0f;
    if (mg->campo_attivo == 1 && mg->tex_utente.id > 0) {
        Rectangle rGlow = { rNickDest.x - spessoreBordo, rNickDest.y - spessoreBordo, rNickDest.width + (spessoreBordo * 2), rNickDest.height + (spessoreBordo * 2) };
        DrawTexturePro(mg->tex_utente, (Rectangle){0, 0, (float)mg->tex_utente.width, (float)mg->tex_utente.height}, rGlow, (Vector2){0, 0}, 0, GOLD);
    }
    else if (mg->campo_attivo == 2 && mg->tex_password.id > 0) {
        Rectangle rGlow = { rPassDest.x - spessoreBordo, rPassDest.y - spessoreBordo, rPassDest.width + (spessoreBordo * 2), rPassDest.height + (spessoreBordo * 2) };
        DrawTexturePro(mg->tex_password, (Rectangle){0, 0, (float)mg->tex_password.width, (float)mg->tex_password.height}, rGlow, (Vector2){0, 0}, 0, GOLD);
    }

    // Disegno delle texture dei box dei campi di testo sopra l'eventuale glow
    if (mg->tex_utente.id > 0) {
        DrawTexturePro(mg->tex_utente, (Rectangle){0, 0, (float)mg->tex_utente.width, (float)mg->tex_utente.height}, rNickDest, (Vector2){0, 0}, 0, WHITE);
    }
    if (mg->tex_password.id > 0) {
        DrawTexturePro(mg->tex_password, (Rectangle){0, 0, (float)mg->tex_password.width, (float)mg->tex_password.height}, rPassDest, (Vector2){0, 0}, 0, WHITE);
    }

    // Calcoli geometrici per l'allineamento del testo centrato verticalmente nei box
    float testonickDim = 22.0f;
    float testopassDim = 26.0f;
    float allineamentoTestoY_Nick = posizionaNickY + (ottimaleAltezza / 2.0f) - (testonickDim / 2.0f);
    float allineamentoTestoY_Pass = posizionaPassY + (ottimaleAltezza / 2.0f) - (testopassDim / 2.0f) + 2.0f;
    float allineamentoPlaceholderY_Pass = posizionaPassY + (ottimaleAltezza / 2.0f) - (16.0f / 2.0f);

    float inputNickX = posizionaX + 115.0f;
    float inputPassX = posizionaX + 115.0f;

    // Rendering dei Placeholder descrittivi se i campi sono vuoti
    if (strlen(mg->nickname) == 0) {
        DrawText("max 10 caratteri", (int)inputNickX, (int)allineamentoTestoY_Nick + 2, 16, WHITE);
    } else {
        DrawText(mg->nickname, (int)inputNickX, (int)allineamentoTestoY_Nick, (int)testonickDim, BLACK);
    }

    // Mascheramento della password tramite oscuramento con caratteri asterisco '*'
    char stars[BUFFER_AUTH_SIZE] = {0};
    if (strlen(mg->password) == 0) {
        DrawText("max 10 caratteri", (int)inputPassX, (int)allineamentoPlaceholderY_Pass, 16, WHITE);
    } else {
        size_t pass_len = strlen(mg->password);
        for (size_t i = 0; i < pass_len; i++) {
            stars[i] = '*';
        }
        stars[pass_len] = '\0';
        DrawText(stars, (int)inputPassX, (int)allineamentoTestoY_Pass, (int)testopassDim, BLACK);
    }

    // Rendering del cursore lampeggiante (Blink Cursor) basato sul contatore di frame
    if (mg->campo_attivo == 1) {
        if (((mg->frame_counter / 20) % 2) == 0) {
            int spostamentoTesto = MeasureText(mg->nickname, (int)testonickDim);
            DrawText("|", (int)(inputNickX + spostamentoTesto), (int)allineamentoTestoY_Nick, (int)testonickDim, MAROON);
        }
    } else if (mg->campo_attivo == 2) {
        if (((mg->frame_counter / 20) % 2) == 0) {
            int spostamentoTesto = MeasureText(stars, (int)testopassDim);
            DrawText("|", (int)(inputPassX + spostamentoTesto), (int)allineamentoTestoY_Pass - 2, (int)testopassDim, MAROON);
        }
    }

    // Rendering centralizzato dei Banner di Notifica Errore visivi sullo schermo
    if (codice_errore > 0 && codice_errore != 99) {
        const char* msg = (codice_errore == 1) ? "UTENTE NON TROVATO" :
                          (codice_errore == 2) ? "PASSWORD ERRATA" :
                          (codice_errore == 3) ? "NICKNAME GIA' IN USO" : "SPAZIO NON CONSENTITO";
        int tw = MeasureText(msg, 30);
        DrawRectangle(600 - tw / 2 - 15, 100, tw + 30, 50, Fade(BLACK, 0.8f));
        DrawText(msg, 600 - tw / 2, 110, 30, RED);
    }

    // --- LOGICA DI CONVALIDA (Tasto INVIO) ---
    if (IsKeyPressed(KEY_ENTER) && strlen(mg->nickname) > 0 && strlen(mg->password) > 0) {
        if (stato == STATO_REGISTRATI) {
            if (EsisteNickname(mg->nickname)) {
                codice_errore = 3;
            } else {
                RegistraNuovoUtenteInTabella(mg->nickname, mg->password);
                SalvaLogStoricoAzione(mg->nickname, mg->password, 0); // Log della registrazione
                PlaySound(suono_bottone);
                codice_errore = 0;
                return STATO_MENU_SCELTA;
            }
        } else {
            int res = VerificaCredenzialiDettagliata(mg->nickname, mg->password);

            if (res == 99) { // Switch immediato alla modalità Debug Admin
                PlaySound(suono_bottone);
                codice_errore = 0;
                return STATO_DEBUG;
            }

            if (res == 0) {
                AggiornaTimestampAccesso(mg->nickname);
                SalvaLogStoricoAzione(mg->nickname, mg->password, 1); // Log dell'accesso riuscito
                PlaySound(suono_bottone);
                codice_errore = 0;
                return STATO_MENU_SCELTA;
            } else {
                codice_errore = res;
                // Svuota in sicurezza il buffer della password fallita per motivi di privacy/sicurezza
                memset(mg->password, 0, BUFFER_AUTH_SIZE);
            }
        }
    }

    // Permette di tornare indietro al menu principale ripristinando lo stato dell'errore
    if (IsKeyPressed(KEY_ESCAPE)) {
        codice_errore = 0;
        return STATO_MENU;
    }

    return stato;
}

// =========================================================================
// --- FUNZIONI DI INIZIALIZZAZIONE E COSTRUZIONE INTERFACCIA ---
// =========================================================================

void InizializzaMenu(MenuGrafica *mg) {
    SetExitKey(KEY_NULL); // Disabilita ESC come tasto di chiusura standard della finestra Raylib

    // Caricamento controllato degli asset grafici mediante la funzione sicura del progetto
    mg->sfondo = CaricaSicuro("Sfondo/sfondo_inizio.png");
    mg->btn_accedi = CaricaSicuro("Pulsanti/accedi.png");
    mg->btn_registrati = CaricaSicuro("Pulsanti/registrati.png");
    mg->btn_ospite = CaricaSicuro("Pulsanti/gioca_come_ospite.png");
    mg->sfondo_accedi = CaricaSicuro("Sfondo/sfondo_accedi.png");
    mg->sfondo_registrati = CaricaSicuro("Sfondo/sfondo_registrazione.png");
    mg->tex_utente = CaricaSicuro("Pulsanti/utente.png");
    mg->tex_password = CaricaSicuro("Pulsanti/password.png");

    // CORREZIONE ANTI-LEAK E COMPILATORE: Utilizzo di IsSoundValid al posto dell'inesistente IsSoundReady
    if (!IsSoundValid(suono_bottone)) {
        suono_bottone = LoadSound("Musiche/suono_bottone.mp3");
    }

    // Inizializzazione controllata di tutte le strutture dati interne
    memset(mg->nickname, 0, BUFFER_AUTH_SIZE);
    memset(mg->password, 0, BUFFER_AUTH_SIZE);
    mg->campo_attivo = 0;
    mg->frame_counter = 0;
    codice_errore = 0;
}

StatoGioco AggiornaDisegnaMenu(MenuGrafica *mg) {
    Vector2 mouse = GetMousePosition();
    const float cX = 600.0f; // Centro dello schermo sull'asse X (Risoluzione di riferimento: 1200x800)

    // Calcolo delle altezze proporzionali delle texture dei bottoni per preservare l'Aspect Ratio originale
    const float largh_visuale = 490.0f;
    const float h2 = (float)mg->btn_registrati.height * (largh_visuale / (float)mg->btn_registrati.width);
    const float h1 = (float)mg->btn_accedi.height * (largh_visuale / (float)mg->btn_accedi.width);
    const float h3 = (float)mg->btn_ospite.height * (largh_visuale / (float)mg->btn_ospite.width);

    // Layout verticale bilanciato e matematicamente centrato rispetto al frame principale
    float yReg = 455.0f - (h2 / 2.0f);
    float yAcc = yReg - h1 - (-82.0f);
    float yOsp = yReg + h2 + (-82.0f);

    // Rettangoli di destinazione per le routine di disegno DrawTexturePro
    Rectangle rAccDraw = { cX - (largh_visuale / 2.0f), yAcc, largh_visuale, h1 };
    Rectangle rRegDraw = { cX - (largh_visuale / 2.0f), yReg, largh_visuale, h2 };
    Rectangle rOspDraw = { cX - (largh_visuale / 2.0f), yOsp, largh_visuale, h3 };

    // Dimensione ottimizzata e centrata della Hitbox cliccabile (Risolve i click fuori sagoma)
    float hitbox_W = 430.0f;
    float hitbox_H = 120.0f;

    // Calcolo automatico e centrato dei rettangoli di collisione mouse basati sulla hitbox configurata
    Rectangle rAccHitbox = { cX - (hitbox_W / 2.0f), yAcc + (h1 - hitbox_H) / 2.0f, hitbox_W, hitbox_H };
    Rectangle rRegHitbox = { cX - (hitbox_W / 2.0f), yReg + (h2 - hitbox_H) / 2.0f, hitbox_W, hitbox_H };
    Rectangle rOspHitbox = { cX - (hitbox_W / 2.0f), yOsp + (h3 - hitbox_H) / 2.0f, hitbox_W, hitbox_H };

    // --- RENDERING SCENA ---
    if (mg->sfondo.id > 0) {
        DrawTexturePro(mg->sfondo, (Rectangle) { 0, 0, (float)mg->sfondo.width, (float)mg->sfondo.height }, (Rectangle) { 0, 0, 1200, 800 }, (Vector2) { 0, 0 }, 0, WHITE);
    }

    // Disegno dei pulsanti con effetto "Hover" (Schiarimento con LIGHTGRAY al passaggio del mouse sulla Hitbox)
    if (mg->btn_accedi.id > 0) {
        DrawTexturePro(mg->btn_accedi, (Rectangle) { 0, 0, (float)mg->btn_accedi.width, (float)mg->btn_accedi.height }, rAccDraw, (Vector2) { 0, 0 }, 0, CheckCollisionPointRec(mouse, rAccHitbox) ? LIGHTGRAY : WHITE);
    }

    if (mg->btn_registrati.id > 0) {
        DrawTexturePro(mg->btn_registrati, (Rectangle) { 0, 0, (float)mg->btn_registrati.width, (float)mg->btn_registrati.height }, rRegDraw, (Vector2) { 0, 0 }, 0, CheckCollisionPointRec(mouse, rRegHitbox) ? LIGHTGRAY : WHITE);
    }

    if (mg->btn_ospite.id > 0) {
        DrawTexturePro(mg->btn_ospite, (Rectangle) { 0, 0, (float)mg->btn_ospite.width, (float)mg->btn_ospite.height }, rOspDraw, (Vector2) { 0, 0 }, 0, CheckCollisionPointRec(mouse, rOspHitbox) ? LIGHTGRAY : WHITE);
    }

    // --- GESTIONE INPUT MOUSE ---
    // Sostituito MOUSE_LEFT_BUTTON (deprecato) con MOUSE_BUTTON_LEFT standard delle nuove versioni di Raylib
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse, rAccHitbox)) {
            PlaySound(suono_bottone);
            return STATO_ACCEDI;
        }
        if (CheckCollisionPointRec(mouse, rRegHitbox)) {
            PlaySound(suono_bottone);
            return STATO_REGISTRATI;
        }
        if (CheckCollisionPointRec(mouse, rOspHitbox)) {
            PlaySound(suono_bottone);
            return STATO_MENU_SCELTA;
        }
    }

    return STATO_MENU;
}

void ScaricaMenu(MenuGrafica *mg) {
    // Rilascio completo e sequenziale della memoria video occupata dalle texture della GPU
    UnloadTexture(mg->sfondo);
    UnloadTexture(mg->btn_accedi);
    UnloadTexture(mg->btn_registrati);
    UnloadTexture(mg->btn_ospite);
    UnloadTexture(mg->sfondo_accedi);
    UnloadTexture(mg->sfondo_registrati);
    UnloadTexture(mg->tex_utente);
    UnloadTexture(mg->tex_password);

    // CORREZIONE ANTI-LEAK E COMPILATORE: Sostituito IsSoundReady con il corretto IsSoundValid
    if (IsSoundValid(suono_bottone)) {
        UnloadSound(suono_bottone);
    }
}