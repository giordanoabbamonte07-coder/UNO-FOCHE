#include "archivio.h"
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static Texture2D sfondoStorico;
static Texture2D btnRiprendi;
static Partita partiteCaricate[MAX_SALVATAGGI_PER_UTENTE];
static int nPartiteCaricate = 0;

void PreparaArchivioStorico(const char *username) {
    nPartiteCaricate = CaricaPartiteSospeseUtente(username, partiteCaricate);
}

void InizializzaRisorseArchivio(void) {
    sfondoStorico = LoadTexture("Sfondo/sfondo_storico.png");
    btnRiprendi = LoadTexture("Pulsanti/riprendi.png");
    btnAvanti = LoadTexture("Pulsanti/avanti.png");
    btnIndietro = LoadTexture("Pulsanti/indietro.png");
    sfondoStatistiche = LoadTexture("Sfondo/sfondo_statistiche.png");

    // --- NUOVA SCALATURA AL 10% (Ultra compatti) ---
    float scalaTasti = 0.10f;

    float larghezzaIndietro = (float)btnIndietro.width * scalaTasti;
    float altezzaIndietro = (float)btnIndietro.height * scalaTasti;

    float larghezzaAvanti = (float)btnAvanti.width * scalaTasti;
    float altezzaAvanti = (float)btnAvanti.height * scalaTasti;

    float margineBordo = 25.0f;
    float posizioneY = 800.0f - altezzaIndietro - margineBordo;

    rectIndietro = (Rectangle){ margineBordo, posizioneY, larghezzaIndietro, altezzaIndietro };
    rectAvanti = (Rectangle){ 1200.0f - larghezzaAvanti - margineBordo, posizioneY, larghezzaAvanti, altezzaAvanti };
}

void ScaricaRisorseArchivio(void) {
    UnloadTexture(sfondoStorico);
    UnloadTexture(btnRiprendi);
    UnloadTexture(btnAvanti);
    UnloadTexture(btnIndietro);
    UnloadTexture(sfondoStatistiche);
}

void SalvaPartitaSospesa(Partita *partitaCorrente) {
    if (partitaCorrente->is_guest || partitaCorrente->partita_finita) {
        return; // Non salva se è un ospite o se la partita è già terminata
    }

    // Imposta la data e l'ora reali del salvataggio
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(partitaCorrente->data_salvataggio, sizeof(partitaCorrente->data_salvataggio), "%d/%m/%Y %H:%M", tm_info);

    // 1. Legge tutte le partite registrate nel file binario
    FILE *fileRead = fopen("salvataggi.dat", "rb");
    Partita *tutteLePartite = NULL;
    int totalePartite = 0;

    if (fileRead) {
        fseek(fileRead, 0, SEEK_END);
        long fileSize = ftell(fileRead);
        fseek(fileRead, 0, SEEK_SET);
        totalePartite = fileSize / sizeof(Partita);

        if (totalePartite > 0) {
            tutteLePartite = (Partita *)malloc(totalePartite * sizeof(Partita));
            fread(tutteLePartite, sizeof(Partita), totalePartite, fileRead);
        }
        fclose(fileRead);
    }

    // 2. Conta quanti salvataggi possiede già questo specifico utente
    int partiteUtenteContate = 0;
    for (int i = 0; i < totalePartite; i++) {
        if (strcmp(tutteLePartite[i].utente_corrente, partitaCorrente->utente_corrente) == 0) {
            partiteUtenteContate++;
        }
    }

    // 3. Riscrive il file applicando la logica FIFO (Max 5 partite)
    FILE *fileWrite = fopen("salvataggi.dat", "wb");
    if (!fileWrite) {
        if (tutteLePartite) free(tutteLePartite);
        return;
    }

    // Se l'utente ha già 5 o più partite, dobbiamo scartare le più vecchie eccedenti
    int incontratiUtente = 0;
    int daEliminare = (partiteUtenteContate >= 5) ? (partiteUtenteContate - 4) : 0;

    for (int i = 0; i < totalePartite; i++) {
        if (strcmp(tutteLePartite[i].utente_corrente, partitaCorrente->utente_corrente) == 0) {
            incontratiUtente++;
            if (incontratiUtente <= daEliminare) {
                continue; // Salta e rimuove la partita più vecchia del giocatore
            }
        }
        fwrite(&tutteLePartite[i], sizeof(Partita), 1, fileWrite);
    }

    // Inserisce la nuova partita corrente come record più recente
    fwrite(partitaCorrente, sizeof(Partita), 1, fileWrite);
    fclose(fileWrite);
    if (tutteLePartite) free(tutteLePartite);
}

int CaricaPartiteSospeseUtente(const char *username, Partita elencoPartite[]) {
    FILE *file = fopen("salvataggi.dat", "rb");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    int totalePartite = fileSize / sizeof(Partita);

    if (totalePartite <= 0) {
        fclose(file);
        return 0;
    }

    Partita *tutteLePartite = (Partita *)malloc(totalePartite * sizeof(Partita));
    fread(tutteLePartite, sizeof(Partita), totalePartite, file);
    fclose(file);

    int contatore = 0;
    for (int i = totalePartite - 1; i >= 0 && contatore < MAX_SALVATAGGI_PER_UTENTE; i--) {
        if (strcmp(tutteLePartite[i].utente_corrente, username) == 0) {
            elencoPartite[contatore] = tutteLePartite[i];
            contatore++;
        }
    }

    free(tutteLePartite);
    return contatore;
}

void EliminaPartitaDaArchivio(const char *username, int indiceDaRimuovere) {
    FILE *fileRead = fopen("salvataggi.dat", "rb");
    if (!fileRead) return;

    fseek(fileRead, 0, SEEK_END);
    long fileSize = ftell(fileRead);
    fseek(fileRead, 0, SEEK_SET);
    int totalePartite = fileSize / sizeof(Partita);

    if (totalePartite <= 0) {
        fclose(fileRead);
        return;
    }

    Partita *tutteLePartite = (Partita *)malloc(totalePartite * sizeof(Partita));
    fread(tutteLePartite, sizeof(Partita), totalePartite, fileRead);
    fclose(fileRead);

    FILE *fileWrite = fopen("salvataggi.dat", "wb");
    if (!fileWrite) {
        free(tutteLePartite);
        return;
    }

    int contatoreUtente = 0;
    for (int i = totalePartite - 1; i >= 0; i--) {
        if (strcmp(tutteLePartite[i].utente_corrente, username) == 0) {
            if (contatoreUtente == indiceDaRimuovere) {
                tutteLePartite[i].utente_corrente[0] = '\0';
                break;
            }
            contatoreUtente++;
        }
    }

    for (int i = 0; i < totalePartite; i++) {
        if (tutteLePartite[i].utente_corrente[0] != '\0') {
            fwrite(&tutteLePartite[i], sizeof(Partita), 1, fileWrite);
        }
    }

    fclose(fileWrite);
    free(tutteLePartite);
}
// 3. SOSTITUISCI INTERAMENTE LA TUA VECCHIA FUNZIONE CON QUESTA
void DisegnaArchivioStorico(const char *username, Vector2 mousePos, int *partitaSelezionataIndice) {
    (void)username;

    DrawTexturePro(sfondoStorico,
                   (Rectangle){ 0, 0, (float)sfondoStorico.width, (float)sfondoStorico.height },
                   (Rectangle){ 0, 0, 1200, 800 },
                   (Vector2){ 0, 0 }, 0.0f, WHITE);

    // MODIFICA: Usiamo nPartiteCaricate che è in memoria invece di leggere ogni frame dal disco
    if (nPartiteCaricate == 0) {
        int larghezzaTestoVuoto = MeasureText("NESSUN RECORD IN ARCHIVIO", 24);
        DrawText("NESSUN RECORD IN ARCHIVIO", (1200 - larghezzaTestoVuoto) / 2, 400, 24, LIGHTGRAY);
        return;
    }

    // --- CENTRATURA COLONNE (Coordinate X) ---
    int centroColonnaData = 260;
    int centroColonnaEvento = 560;
    int centroColonnaPulsante = 890;

    // --- COMPATTAZIONE VERTICALE (Coordinate Y) ---
    int rigaInizialeY = 320;
    int scostamentoY = 78;

    // MODIFICA: Il ciclo ora gira su nPartiteCaricate
    for (int i = 0; i < nPartiteCaricate; i++) {
        int centroRigaY = rigaInizialeY + (i * scostamentoY);

        char dataSolo[12] = "";
        char oraSolo[8] = "";

        // MODIFICA: Accediamo a partiteCaricate[i] invece del vecchio array locale
        if (strlen(partiteCaricate[i].data_salvataggio) >= 16) {
            strncpy(dataSolo, partiteCaricate[i].data_salvataggio, 10);
            dataSolo[10] = '\0';
            strncpy(oraSolo, partiteCaricate[i].data_salvataggio + 11, 5);
            oraSolo[5] = '\0';
        } else {
            strcpy(dataSolo, "21/05/2026");
            strcpy(oraSolo, "19:00");
        }

        int larghezzaData = MeasureText(dataSolo, 20);
        int larghezzaOra = MeasureText(oraSolo, 20);

        DrawText(dataSolo, centroColonnaData - (larghezzaData / 2), centroRigaY - 14, 20, WHITE);
        DrawText(oraSolo, centroColonnaData - (larghezzaOra / 2), centroRigaY + 8, 20, WHITE);

        char stringaEvento[32];
        // MODIFICA: Accediamo a partiteCaricate[i]
        if (partiteCaricate[i].num_giocatori == 2) {
            strcpy(stringaEvento, "1 VS 1 (BOT)");
        } else {
            strcpy(stringaEvento, "MULTIPLAYER (1vs3)");
        }

        int larghezzaEvento = MeasureText(stringaEvento, 20);
        DrawText(stringaEvento, centroColonnaEvento - (larghezzaEvento / 2), centroRigaY - 10, 20, WHITE);

        float larghPulsante = 140.0f;
        float altPulsante = 42.0f;

        Rectangle rectPulsante = {
            (float)(centroColonnaPulsante - (larghPulsante / 2)),
            (float)(centroRigaY - (altPulsante / 2)),
            larghPulsante,
            altPulsante
        };

        bool collisioneMouse = CheckCollisionPointRec(mousePos, rectPulsante);
        Rectangle srcRec = { 0.0f, 0.0f, (float)btnRiprendi.width, (float)btnRiprendi.height };

        if (collisioneMouse) {
            DrawTexturePro(btnRiprendi, srcRec, rectPulsante, (Vector2){ 0, 0 }, 0.0f, LIGHTGRAY);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayCardSound();
                *partitaSelezionataIndice = i;
            }
        } else {
            DrawTexturePro(btnRiprendi, srcRec, rectPulsante, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
    }
}

void SalvaRisultatoPartita(const char *username, bool haVinto) {
    if (strcmp(username, "Guest") == 0) return;

    struct RecordStatistica {
        char username[32];
        int vittorie;
        int sconfitte;
    } record;

    FILE *file = fopen("statistiche_utenti.dat", "rb+");
    bool trovato = false;

    if (file == NULL) {
        file = fopen("statistiche_utenti.dat", "wb+");
    }

    if (file != NULL) {
        while (fread(&record, sizeof(record), 1, file)) {
            if (strcmp(record.username, username) == 0) {
                if (haVinto) record.vittorie++;
                else record.sconfitte++;

                fseek(file, -sizeof(record), SEEK_CUR);
                fwrite(&record, sizeof(record), 1, file);
                trovato = true;
                break;
            }
        }

        if (!trovato) {
            memset(&record, 0, sizeof(record));
            strcpy(record.username, username);
            if (haVinto) record.vittorie = 1;
            else record.sconfitte = 1;

            fseek(file, 0, SEEK_END);
            fwrite(&record, sizeof(record), 1, file);
        }
        fclose(file);
    }
}

void OttieniStatisticheUtente(const char *username, int *vittorie, int *sconfitte) {
    *vittorie = 0;
    *sconfitte = 0;

    struct RecordStatistica {
        char username[32];
        int vittorie;
        int sconfitte;
    } record;

    FILE *file = fopen("statistiche_utenti.dat", "rb");
    if (file != NULL) {
        while (fread(&record, sizeof(record), 1, file)) {
            if (strcmp(record.username, username) == 0) {
                *vittorie = record.vittorie;
                *sconfitte = record.sconfitte;
                break;
            }
        }
        fclose(file);
    }
}
