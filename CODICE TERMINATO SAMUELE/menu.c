#include "menu.h"
#include "graphics.h"
#include "audio.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

static int codice_errore = 0;
static Sound suono_bottone;

// Struttura ausiliaria per mantenere la compatibilità con i log storici
typedef struct {
    char nickname[11];
    char password[11];
    char data_ora[25];
    int tipo_azione;
} LogAccesso;

// --- FUNZIONI DI SERVIZIO E GESTIONE DATABASE ---

static bool EsisteNickname(const char* nick) {
    FILE *file = fopen("utenti.dat", "rb");
    if (file == NULL) return false;

    UtenteRegistrato u;
    bool trovato = false;

    while (fread(&u, sizeof(UtenteRegistrato), 1, file)) {
        if (strcmp(u.nickname, nick) == 0) {
            trovato = true;
            break;
        }
    }
    fclose(file);
    return trovato;
}

static int VerificaCredenzialiDettagliata(const char* nick, const char* pass) {
    // Controllo credenziali Admin prioritario - Armonizzato in minuscolo con main.c
    if (strcmp(nick, "admin") == 0 && strcmp(pass, "1234") == 0) return 99;

    FILE *file = fopen("utenti.dat", "rb");
    if (file == NULL) return 1;

    UtenteRegistrato u;
    int risultato = 1;

    while (fread(&u, sizeof(UtenteRegistrato), 1, file)) {
        if (strcmp(u.nickname, nick) == 0) {
            if (strcmp(u.password, pass) == 0) {
                risultato = 0;
                break;
            } else {
                risultato = 2;
                break;
            }
        }
    }
    fclose(file);
    return risultato;
}

// Salva un utente unico all'atto della registrazione (Versione Corretta e Protetta)
static void RegistraNuovoUtenteInTabella(const char* nick, const char* pass) {
    FILE *file = fopen("utenti.dat", "ab");
    if (file != NULL) {
        UtenteRegistrato u;
        memset(&u, 0, sizeof(UtenteRegistrato));

        // Copia di sicurezza limitata a 10 caratteri
        strncpy(u.nickname, nick, 10);
        u.nickname[10] = '\0'; // --- CORREZIONE BUG 5: Forza esplicitamente il terminatore nullo

        strncpy(u.password, pass, 10);
        u.password[10] = '\0'; // --- CORREZIONE BUG 5: Forza esplicitamente il terminatore nullo

        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) strftime(u.ultimo_accesso, 24, "%d/%m/%Y %H:%M:%S", tm_info);

        u.ultima_azione = 0; // 0 = REGISTRAZIONE

        fwrite(&u, sizeof(UtenteRegistrato), 1, file);
        fclose(file);
    }
}

// Aggiorna la data di ultimo accesso sul record esistente senza duplicare la riga
static void AggiornaTimestampAccesso(const char* nick) {
    FILE *file = fopen("utenti.dat", "rb+");
    if (file == NULL) return;

    UtenteRegistrato u;
    while (fread(&u, sizeof(UtenteRegistrato), 1, file) == 1) {
        if (strcmp(u.nickname, nick) == 0) {
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            if (tm_info) strftime(u.ultimo_accesso, 24, "%d/%m/%Y %H:%M:%S", tm_info);

            u.ultima_azione = 1; // 1 = ACCESSO

            fseek(file, -((long)sizeof(UtenteRegistrato)), SEEK_CUR);
            fwrite(&u, sizeof(UtenteRegistrato), 1, file);
            break;
        }
    }
    fclose(file);
}

// Mantiene una traccia storica separata
static void SalvaLogStoricoAzione(const char* nick, const char* pass, int tipo_azione) {
    FILE *file = fopen("accessi_log.dat", "ab");
    if (file != NULL) {
        LogAccesso log;
        memset(&log, 0, sizeof(LogAccesso));
        strncpy(log.nickname, nick, 10);
        strncpy(log.password, pass, 10);

        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) strftime(log.data_ora, 24, "%d/%m/%Y %H:%M:%S", tm_info);

        log.tipo_azione = tipo_azione;
        fwrite(&log, sizeof(LogAccesso), 1, file);
        fclose(file);
    }
}

// --- INPUT TASTIERA NEI CAMPI ---

static void GestisceInputTesto(char *buffer, int *campo_attivo, int id) {
    if (*campo_attivo == id) {
        int key = GetCharPressed();
        while (key > 0) {
            if (id == 2 && key == 32) {
                codice_errore = 4;
            }
            else if ((key >= 32) && (key <= 125) && (strlen(buffer) < 10)) {
                size_t len = strlen(buffer);
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
                codice_errore = 0;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && strlen(buffer) > 0) {
            buffer[strlen(buffer) - 1] = '\0';
            codice_errore = 0;
        }
    }
}

StatoGioco GestisciSchermataAuth(MenuGrafica *mg, StatoGioco stato) {
    Vector2 mouse = GetMousePosition();
    mg->frame_counter++;

    float ottimaleLarghezza = 312.0f;
    float ottimaleAltezza = 72.0f;
    if (mg->tex_utente.id > 0) {
        ottimaleAltezza = (float)mg->tex_utente.height * (ottimaleLarghezza / (float)mg->tex_utente.width);
    }

    float posizionaX = 615.0f;
    float posizionaNickY = 442.0f - (ottimaleAltezza / 2.0f);
    float posizionaPassY = 568.0f - (ottimaleAltezza / 2.0f);

    Rectangle rNickDest = { posizionaX, posizionaNickY, ottimaleLarghezza, ottimaleAltezza };
    Rectangle rPassDest = { posizionaX, posizionaPassY, ottimaleLarghezza, ottimaleAltezza };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, rNickDest)) mg->campo_attivo = 1;
        else if (CheckCollisionPointRec(mouse, rPassDest)) mg->campo_attivo = 2;
        else mg->campo_attivo = 0;
    }

    GestisceInputTesto(mg->nickname, &mg->campo_attivo, 1);
    GestisceInputTesto(mg->password, &mg->campo_attivo, 2);

    Texture2D texSfondo = (stato == STATO_REGISTRATI) ? mg->sfondo_registrati : mg->sfondo_accedi;
    if (texSfondo.id > 0) {
        DrawTexturePro(texSfondo, (Rectangle){0, 0, (float)texSfondo.width, (float)texSfondo.height}, (Rectangle){0, 0, 1200, 800}, (Vector2){0, 0}, 0, WHITE);
    }

    float spessoreBordo = 5.0f;
    if (mg->campo_attivo == 1 && mg->tex_utente.id > 0) {
        Rectangle rGlow = { rNickDest.x - spessoreBordo, rNickDest.y - spessoreBordo, rNickDest.width + (spessoreBordo * 2), rNickDest.height + (spessoreBordo * 2) };
        DrawTexturePro(mg->tex_utente, (Rectangle){0, 0, (float)mg->tex_utente.width, (float)mg->tex_utente.height}, rGlow, (Vector2){0, 0}, 0, GOLD);
    }
    else if (mg->campo_attivo == 2 && mg->tex_password.id > 0) {
        Rectangle rGlow = { rPassDest.x - spessoreBordo, rPassDest.y - spessoreBordo, rPassDest.width + (spessoreBordo * 2), rPassDest.height + (spessoreBordo * 2) };
        DrawTexturePro(mg->tex_password, (Rectangle){0, 0, (float)mg->tex_password.width, (float)mg->tex_password.height}, rGlow, (Vector2){0, 0}, 0, GOLD);
    }

    if (mg->tex_utente.id > 0) {
        DrawTexturePro(mg->tex_utente, (Rectangle){0, 0, (float)mg->tex_utente.width, (float)mg->tex_utente.height}, rNickDest, (Vector2){0, 0}, 0, WHITE);
    }
    if (mg->tex_password.id > 0) {
        DrawTexturePro(mg->tex_password, (Rectangle){0, 0, (float)mg->tex_password.width, (float)mg->tex_password.height}, rPassDest, (Vector2){0, 0}, 0, WHITE);
    }

    float testonickDim = 22.0f;
    float testopassDim = 26.0f;
    float allineamentoTestoY_Nick = posizionaNickY + (ottimaleAltezza / 2.0f) - (testonickDim / 2.0f);
    float allineamentoTestoY_Pass = posizionaPassY + (ottimaleAltezza / 2.0f) - (testopassDim / 2.0f) + 2.0f;
    float allineamentoPlaceholderY_Pass = posizionaPassY + (ottimaleAltezza / 2.0f) - (16.0f / 2.0f);

    float inputNickX = posizionaX + 115.0f;
    float inputPassX = posizionaX + 115.0f;

    if (strlen(mg->nickname) == 0) {
        DrawText("max 10 caratteri", (int)inputNickX, (int)allineamentoTestoY_Nick + 2, 16, WHITE);
    } else {
        DrawText(mg->nickname, (int)inputNickX, (int)allineamentoTestoY_Nick, (int)testonickDim, BLACK);
    }

    char stars[12] = {0};
    if (strlen(mg->password) == 0) {
        DrawText("max 10 caratteri", (int)inputPassX, (int)allineamentoPlaceholderY_Pass, 16, WHITE);
    } else {
        for (int i = 0; i < (int)strlen(mg->password); i++) stars[i] = '*';
        DrawText(stars, (int)inputPassX, (int)allineamentoTestoY_Pass, (int)testopassDim, BLACK);
    }

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

    if (codice_errore > 0 && codice_errore != 99) {
        const char* msg = (codice_errore == 1) ? "UTENTE NON TROVATO" :
                          (codice_errore == 2) ? "PASSWORD ERRATA" :
                          (codice_errore == 3) ? "NICKNAME GIA' IN USO" : "SPAZIO NON CONSENTITO";
        int tw = MeasureText(msg, 30);
        DrawRectangle(600 - tw / 2 - 15, 100, tw + 30, 50, Fade(BLACK, 0.8f));
        DrawText(msg, 600 - tw / 2, 110, 30, RED);
    }

    // --- CORREZIONE: VERIFICA DELLE CREDENZIALI ALLA PRESSIONE DI INVIO ---
    if (IsKeyPressed(KEY_ENTER) && strlen(mg->nickname) > 0 && strlen(mg->password) > 0) {
        if (stato == STATO_REGISTRATI) {
            if (EsisteNickname(mg->nickname)) {
                codice_errore = 3;
            } else {
                RegistraNuovoUtenteInTabella(mg->nickname, mg->password);
                SalvaLogStoricoAzione(mg->nickname, mg->password, 0);
                PlaySound(suono_bottone);
                return STATO_MENU_SCELTA;
            }
        } else {
            int res = VerificaCredenzialiDettagliata(mg->nickname, mg->password);

            if (res == 99) { // Rilevato Admin
                PlaySound(suono_bottone);
                codice_errore = 0;
                return STATO_DEBUG; // <-- CORRETTO: Adesso restituisce STATO_DEBUG per attivare il main.c!
            }

            if (res == 0) {
                AggiornaTimestampAccesso(mg->nickname);
                SalvaLogStoricoAzione(mg->nickname, mg->password, 1);
                PlaySound(suono_bottone);
                codice_errore = 0;
                return STATO_MENU_SCELTA;
            } else {
                codice_errore = res;
                memset(mg->password, 0, 12);
            }
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        codice_errore = 0;
        return STATO_MENU;
    }

    return stato;
}

// --- FUNZIONI DI INIZIALIZZAZIONE INTERFACCIA ---

void InizializzaMenu(MenuGrafica *mg) {
    SetExitKey(KEY_NULL);
    mg->sfondo = CaricaSicuro("Sfondo/sfondo_inizio.png");
    mg->btn_accedi = CaricaSicuro("Pulsanti/accedi.png");
    mg->btn_registrati = CaricaSicuro("Pulsanti/registrati.png");
    mg->btn_ospite = CaricaSicuro("Pulsanti/gioca_come_ospite.png");
    mg->sfondo_accedi = CaricaSicuro("Sfondo/sfondo_accedi.png");
    mg->sfondo_registrati = CaricaSicuro("Sfondo/sfondo_registrazione.png");
    mg->tex_utente = CaricaSicuro("Pulsanti/utente.png");
    mg->tex_password = CaricaSicuro("Pulsanti/password.png");
    suono_bottone = LoadSound("Musiche/suono_bottone.mp3");

    memset(mg->nickname, 0, 12);
    memset(mg->password, 0, 12);
    mg->campo_attivo = 0;
    mg->frame_counter = 0;
    codice_errore = 0;
}

StatoGioco AggiornaDisegnaMenu(MenuGrafica *mg) {
    Vector2 mouse = GetMousePosition();
    const float largh = 490.0f;
    const float gap = -82.0f;
    const float cX = 600.0f;
    const float cY = 455.0f;

    float h2 = (float)mg->btn_registrati.height * (largh / (float)mg->btn_registrati.width);
    Rectangle rReg = { cX - (largh / 2.0f), cY - (h2 / 2.0f), largh, h2 };

    float h1 = (float)mg->btn_accedi.height * (largh / (float)mg->btn_accedi.width);
    Rectangle rAcc = { cX - (largh / 2.0f), rReg.y - h1 - gap, largh, h1 };

    float h3 = (float)mg->btn_ospite.height * (largh / (float)mg->btn_ospite.width);
    Rectangle rOsp = { cX - (largh / 2.0f), rReg.y + h2 + gap, largh, h3 };

    if (mg->sfondo.id > 0) {
        DrawTexturePro(mg->sfondo, (Rectangle) { 0, 0, (float)mg->sfondo.width, (float)mg->sfondo.height }, (Rectangle) { 0, 0, 1200, 800 }, (Vector2) { 0, 0 }, 0, WHITE);
    }

    if (mg->btn_accedi.id > 0) {
        DrawTexturePro(mg->btn_accedi, (Rectangle) { 0, 0, (float)mg->btn_accedi.width, (float)mg->btn_accedi.height }, rAcc, (Vector2) { 0, 0 }, 0, CheckCollisionPointRec(mouse, rAcc) ? LIGHTGRAY : WHITE);
    }

    if (mg->btn_registrati.id > 0) {
        DrawTexturePro(mg->btn_registrati, (Rectangle) { 0, 0, (float)mg->btn_registrati.width, (float)mg->btn_registrati.height }, rReg, (Vector2) { 0, 0 }, 0, CheckCollisionPointRec(mouse, rReg) ? LIGHTGRAY : WHITE);
    }

    if (mg->btn_ospite.id > 0) {
        DrawTexturePro(mg->btn_ospite, (Rectangle) { 0, 0, (float)mg->btn_ospite.width, (float)mg->btn_ospite.height }, rOsp, (Vector2) { 0, 0 }, 0, CheckCollisionPointRec(mouse, rOsp) ? LIGHTGRAY : WHITE);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, rAcc)) {
            PlaySound(suono_bottone);
            return STATO_ACCEDI;
        }
        if (CheckCollisionPointRec(mouse, rReg)) {
            PlaySound(suono_bottone);
            return STATO_REGISTRATI;
        }
        if (CheckCollisionPointRec(mouse, rOsp)) {
            PlaySound(suono_bottone);
            return STATO_MENU_SCELTA;
        }
    }

    return STATO_MENU;
}

void ScaricaMenu(MenuGrafica *mg) {
    UnloadTexture(mg->sfondo);
    UnloadTexture(mg->btn_accedi);
    UnloadTexture(mg->btn_registrati);
    UnloadTexture(mg->btn_ospite);
    UnloadTexture(mg->sfondo_accedi);
    UnloadTexture(mg->sfondo_registrati);
    UnloadTexture(mg->tex_utente);
    UnloadTexture(mg->tex_password);
    UnloadSound(suono_bottone);
}