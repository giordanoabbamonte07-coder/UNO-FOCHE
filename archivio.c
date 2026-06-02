#include "archivio.h"
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =======================================================================================
// VARIABILI DI STATO INTERNE (STATIC / FILE-SCOPE)
// =======================================================================================
static Texture2D sfondoStorico;      // Texture di sfondo per la griglia dei salvataggi
static Texture2D btnRiprendi;        // Pulsante grafico per caricare una partita specifica
static Partita partiteCaricate[MAX_SALVATAGGI_PER_UTENTE]; // Cache in RAM dei salvataggi utente
static int nPartiteCaricate = 0;     // Contatore dei record attualmente presenti nella cache

/**
 * Precarica i salvataggi dell'utente dal file binario alla RAM per evitare letture sincrone sul ciclo di draw
 */
void PreparaArchivioStorico(const char *username) {
    // Interroga il database su disco e popola l'array statico locale
    nPartiteCaricate = CaricaPartiteSospeseUtente(username, partiteCaricate);
}

/**
 * Alloca in memoria video (VRAM) gli asset grafici e calcola le hit-box dei tasti di navigazione
 */
void InizializzaRisorseArchivio(void) {
    sfondoStorico = LoadTexture("Sfondo/sfondo_storico.png");
    btnRiprendi = LoadTexture("Pulsanti/riprendi.png");
    btnAvanti = LoadTexture("Pulsanti/avanti.png");
    btnIndietro = LoadTexture("Pulsanti/indietro.png");
    sfondoStatistiche = LoadTexture("Sfondo/sfondo_statistiche.png");

    // SCALATURA ULTRA COMPATTA (Riduzione al 10% della dimensione nativa del file PNG)
    float scalaTasti = 0.10f;

    float larghezzaIndietro = (float)btnIndietro.width * scalaTasti;
    float altezzaIndietro = (float)btnIndietro.height * scalaTasti;

    float larghezzaAvanti = (float)btnAvanti.width * scalaTasti;
    float altezzaAvanti = (float)btnAvanti.height * scalaTasti;

    float margineBordo = 25.0f; // Distanza di sicurezza dai margini della finestra
    float posizioneY = 800.0f - altezzaIndietro - margineBordo; // Allineamento sul fondo dello schermo (Y = 800)

    // Definizione dei rettangoli di disegno e collisione per le frecce di navigazione dell'archivio
    rectIndietro = (Rectangle){ margineBordo, posizioneY, larghezzaIndietro, altezzaIndietro };
    rectAvanti = (Rectangle){ 1200.0f - larghezzaAvanti - margineBordo, posizioneY, larghezzaAvanti, altezzaAvanti };
}

/**
 * Dealloca in sicurezza le texture dalla VRAM per evitare leak di memoria grafica
 */
void ScaricaRisorseArchivio(void) {
    UnloadTexture(sfondoStorico);
    UnloadTexture(btnRiprendi);
    UnloadTexture(btnAvanti);
    UnloadTexture(btnIndietro);
    UnloadTexture(sfondoStatistiche);
}

/**
 * Salva lo stato del match applicando la politica FIFO se l'utente ha più di 5 salvataggi attivi
 */
void SalvaPartitaSospesa(Partita *partitaCorrente) {
    // Il salvataggio viene negato se l'utente è un Ospite o se la partita è giunta al termine
    if (partitaCorrente->is_guest || partitaCorrente->partita_finita) {
        return;
    }

    // Cattura il timestamp di sistema per marcare il record
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(partitaCorrente->data_salvataggio, sizeof(partitaCorrente->data_salvataggio), "%d/%m/%Y %H:%M", tm_info);

    // 1. LETTURA COMPLETA DEI SALVATAGGI ESISTENTI NEL FILE
    FILE *fileRead = fopen("salvataggi.dat", "rb");
    Partita *tutteLePartite = NULL;
    int totalePartite = 0;

    if (fileRead) {
        fseek(fileRead, 0, SEEK_END);
        long fileSize = ftell(fileRead);
        fseek(fileRead, 0, SEEK_SET);
        totalePartite = fileSize / sizeof(Partita); // Calcola quanti record Partita contiene il file

        if (totalePartite > 0) {
            tutteLePartite = (Partita *)malloc(totalePartite * sizeof(Partita));
            fread(tutteLePartite, sizeof(Partita), totalePartite, fileRead);
        }
        fclose(fileRead);
    }

    // 2. CONTEGGIO DEI SALVATAGGI GIÀ ASSOCIATI A QUESTO SPECIFICO UTENTE
    int partiteUtenteContate = 0;
    for (int i = 0; i < totalePartite; i++) {
        if (strcmp(tutteLePartite[i].utente_corrente, partitaCorrente->utente_corrente) == 0) {
            partiteUtenteContate++;
        }
    }

    // 3. RISCRITTURA CON LOGICA STRUTTURALE FIFO
    FILE *fileWrite = fopen("salvataggi.dat", "wb");
    if (!fileWrite) {
        if (tutteLePartite) free(tutteLePartite);
        return;
    }

    int incontratiUtente = 0;
    // Se l'utente ha 5 o più partite, calcola quante vecchie deve scartare per far spazio alla nuova (partiteUtenteContate - 4)
    int daEliminare = (partiteUtenteContate >= 5) ? (partiteUtenteContate - 4) : 0;

    for (int i = 0; i < totalePartite; i++) {
        if (strcmp(tutteLePartite[i].utente_corrente, partitaCorrente->utente_corrente) == 0) {
            incontratiUtente++;
            if (incontratiUtente <= daEliminare) {
                continue; // Salta il record: cancella la partita più remota nel tempo di questo profilo
            }
        }
        // Conserva le partite degli altri utenti e i salvataggi regolari rimanenti dell'utente attuale
        fwrite(&tutteLePartite[i], sizeof(Partita), 1, fileWrite);
    }

    // Inserisce in coda l'attuale sessione sospesa (diventa il record più fresco e recente)
    fwrite(partitaCorrente, sizeof(Partita), 1, fileWrite);
    fclose(fileWrite);
    if (tutteLePartite) free(tutteLePartite);
}

/**
 * Scansiona a ritroso il database estrando i match salvati ordinati dal più recente al più vecchio
 */
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
    // Lettura dall'ultimo elemento inserito (totalePartite - 1) scendendo verso l'inizio del file per implementare la LIFO
    for (int i = totalePartite - 1; i >= 0 && contatore < MAX_SALVATAGGI_PER_UTENTE; i--) {
        if (strcmp(tutteLePartite[i].utente_corrente, username) == 0) {
            elencoPartite[contatore] = tutteLePartite[i];
            contatore++;
        }
    }

    free(tutteLePartite);
    return contatore; // Restituisce il numero effettivo di match caricati nella lista dell'utente
}

/**
 * Marca temporaneamente come rimosso un record di salvataggio utente e ricompatta l'intero file binario
 */
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
    // Individua la partita corrispondente all'indice logico selezionato dall'archivio grafico
    for (int i = totalePartite - 1; i >= 0; i--) {
        if (strcmp(tutteLePartite[i].utente_corrente, username) == 0) {
            if (contatoreUtente == indiceDaRimuovere) {
                tutteLePartite[i].utente_corrente[0] = '\0'; // Svuota la stringa dello username per invalidare il blocco
                break;
            }
            contatoreUtente++;
        }
    }

    // Scrive sul file solo i record che non sono stati marchiati con il carattere di fine stringa iniziale
    for (int i = 0; i < totalePartite; i++) {
        if (tutteLePartite[i].utente_corrente[0] != '\0') {
            fwrite(&tutteLePartite[i], sizeof(Partita), 1, fileWrite);
        }
    }

    fclose(fileWrite);
    free(tutteLePartite);
}

/**
 * Renderizza l'interfaccia ad alte prestazioni dell'archivio attingendo ai dati stoccati in RAM
 */
void DisegnaArchivioStorico(const char *username, Vector2 mousePos, int *partitaSelezionataIndice) {
    // Rendering dello sfondo scalato a 1200x800 pixel costanti
    DrawTexturePro(sfondoStorico,
                   (Rectangle){ 0, 0, (float)sfondoStorico.width, (float)sfondoStorico.height },
                   (Rectangle){ 0, 0, 1200, 800 },
                   (Vector2){ 0, 0 }, 0.0f, WHITE);

    // Se la cache indica zero partite caricate, interrompe il rendering mostrando un avviso di archivio vuoto
    if (nPartiteCaricate == 0) {
        int larghezzaTestoVuoto = MeasureText("NESSUN RECORD IN ARCHIVIO", 24);
        DrawText("NESSUN RECORD IN ARCHIVIO", (1200 - larghezzaTestoVuoto) / 2, 400, 24, LIGHTGRAY);
        return;
    }

    // CENTRATURA DELLE COLONNE GRAFICHE (Allineamento orizzontale X rispetto al PNG dello sfondo)
    int centroColonnaData = 260;
    int centroColonnaEvento = 560;
    int centroColonnaPulsante = 890;

    // COMPATTAZIONE GRIGLIA VERTICALE (Coordinata Y di partenza e offset tra le righe)
    int rigaInizialeY = 320;
    int scostamentoY = 78;

    // Ciclo di disegno ottimizzato basato sulle partite registrate in RAM
    for (int i = 0; i < nPartiteCaricate; i++) {
        int centroRigaY = rigaInizialeY + (i * scostamentoY);

        char dataSolo[12] = "";
        char oraSolo[8] = "";

        // Parsifica e divide in due stringhe distinte la data e l'ora ("GG/MM/AAAA" e "HH:MM")
        if (strlen(partiteCaricate[i].data_salvataggio) >= 16) {
            strncpy(dataSolo, partiteCaricate[i].data_salvataggio, 10);
            dataSolo[10] = '\0';
            strncpy(oraSolo, partiteCaricate[i].data_salvataggio + 11, 5);
            oraSolo[5] = '\0';
        } else {
            strcpy(dataSolo, "21/05/2026");
            strcpy(oraSolo, "19:00");
        }

        // Centratura geometrica dei testi della data e dell'ora all'interno della rispettiva colonna
        int larghezzaData = MeasureText(dataSolo, 20);
        int larghezzaOra = MeasureText(oraSolo, 20);
        DrawText(dataSolo, centroColonnaData - (larghezzaData / 2), centroRigaY - 14, 20, WHITE);
        DrawText(oraSolo, centroColonnaData - (larghezzaOra / 2), centroRigaY + 8, 20, WHITE);

        // Identificazione della tipologia di partita caricata in base al numero di bot configurati
        char stringaEvento[32];
        if (partiteCaricate[i].num_giocatori == 2) {
            strcpy(stringaEvento, "1 VS 1 (BOT)");
        } else {
            strcpy(stringaEvento, "MULTIPLAYER (1vs3)");
        }

        int larghezzaEvento = MeasureText(stringaEvento, 20);
        DrawText(stringaEvento, centroColonnaEvento - (larghezzaEvento / 2), centroRigaY - 10, 20, WHITE);

        // CONFIGURAZIONE DEL TASTO GRAFICO "RIPRENDI"
        float larghPulsante = 140.0f;
        float altPulsante = 42.0f;

        Rectangle rectPulsante = {
            (float)(centroColonnaPulsante - (larghPulsante / 2)),
            (float)(centroRigaY - (altPulsante / 2)),
            larghPulsante,
            altPulsante
        };

        // Rilevamento delle collisioni del mouse per gestire gli stati grafici Hover e Click
        bool collisioneMouse = CheckCollisionPointRec(mousePos, rectPulsante);
        Rectangle srcRec = { 0.0f, 0.0f, (float)btnRiprendi.width, (float)btnRiprendi.height };

        if (collisioneMouse) {
            // Effetto Hover: la texture viene scurita con una tinta LIGHTGRAY
            DrawTexturePro(btnRiprendi, srcRec, rectPulsante, (Vector2){ 0, 0 }, 0.0f, LIGHTGRAY);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayCardSound();
                *partitaSelezionataIndice = i; // Passa l'indice della partita scelta al puntatore di ritorno
            }
        } else {
            // Stato standard di riposo del bottone
            DrawTexturePro(btnRiprendi, srcRec, rectPulsante, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
    }
}

/**
 * Aggiorna incrementando le vittorie o le sconfitte globali all'interno del file delle statistiche utenti
 */
void SalvaRisultatoPartita(const char *username, bool haVinto) {
    if (strcmp(username, "Guest") == 0) return; // Esclude il tracciamento se si gioca in modalità Ospite

    struct RecordStatistica {
        char username[32];
        int vittorie;
        int sconfitte;
    } record;

    // Apre il file in modalità lettura/scrittura binaria modificabile ("rb+")
    FILE *file = fopen("statistiche_utenti.dat", "rb+");
    bool trovato = false;

    // Se il file non esiste ancora sul disco fisso, lo crea da zero ("wb+")
    if (file == NULL) {
        file = fopen("statistiche_utenti.dat", "wb+");
    }

    if (file != NULL) {
        // Scansiona il file alla ricerca del record associato al nickname specificato
        while (fread(&record, sizeof(record), 1, file)) {
            if (strcmp(record.username, username) == 0) {
                if (haVinto) record.vittorie++;
                else record.sconfitte++;

                // Arretra l'indicatore di file per sovrascrivere esattamente il blocco appena letto
                fseek(file, -sizeof(record), SEEK_CUR);
                fwrite(&record, sizeof(record), 1, file);
                trovato = true;
                break;
            }
        }

        // Se l'utente non è censito nel database, crea una nuova scheda inserendo il primo punteggio ottenuto
        if (!trovato) {
            memset(&record, 0, sizeof(record));
            strcpy(record.username, username);
            if (haVinto) record.vittorie = 1;
            else record.sconfitte = 1;

            fseek(file, 0, SEEK_END); // Si posiziona in fondo al file per appendere il nuovo record
            fwrite(&record, sizeof(record), 1, file);
        }
        fclose(file);
    }
}

/**
 * Estrae dal file binario il computo totale di vittorie e sconfitte abbinate ad un utente
 */
void OttieniStatisticheUtente(const char *username, int *vittorie, int *sconfitte) {
    *vittorie = 0;  // Valore di inizializzazione e fallback di sicurezza
    *sconfitte = 0; // Valore di inizializzazione e fallback di sicurezza

    struct RecordStatistica {
        char username[32];
        int vittorie;
        int sconfitte;
    } record;

    FILE *file = fopen("statistiche_utenti.dat", "rb");
    if (file != NULL) {
        // Legge sequenzialmente il file finché non individua lo username corrispondente
        while (fread(&record, sizeof(record), 1, file)) {
            if (strcmp(record.username, username) == 0) {
                *vittorie = record.vittorie;   // Carica il valore all'interno del puntatore di output
                *sconfitte = record.sconfitte; // Carica il valore all'interno del puntatore di output
                break;
            }
        }
        fclose(file);
    }
}