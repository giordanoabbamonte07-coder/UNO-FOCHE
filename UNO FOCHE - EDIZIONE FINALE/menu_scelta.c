#include "menu_scelta.h"
#include "audio.h"
#include <string.h>

// Definizioni delle dimensioni logiche standard dello schermo di gioco
#define SCHERMO_LARGHEZZA 1200.0f
#define SCHERMO_ALTEZZA   800.0f

// Variabili statiche per la gestione degli effetti sonori interni al modulo
static Sound suono_bottone;
static bool effettiMenuAttivi = true;

/**
 * Inizializza l'interfaccia grafica del menu di scelta, calcolando i posizionamenti
 * e i rettangoli di disegno mantenendo le proporzioni native delle immagini (Aspect Ratio).
 */
void InizializzaMenuScelta(MenuScelta *menu) {
    // Resetta completamente la struttura per evitare valori residui "spazzatura" nello stack
    memset(menu, 0, sizeof(MenuScelta));

    // --- CARICAMENTO ASSET GRAFICI (Texture di sfondo e pannelli) ---
    menu->sfondo = LoadTexture("Sfondo/sfondo_pagina_main.png");
    menu->sfondoImpostazioni = LoadTexture("Sfondo/sfondo_impostazioni.png");
    menu->sfondoArchivio = LoadTexture("Sfondo/sfondo_storico.png");
    menu->sfondoRegole = LoadTexture("Sfondo/regole.png");

    // --- CARICAMENTO ASSET GRAFICI (Pulsanti d'interfaccia) ---
    menu->btn1vs1 = LoadTexture("Pulsanti/1vs1.png");
    menu->btnMulti = LoadTexture("Pulsanti/multigiocatore.png");
    menu->btnRegole = LoadTexture("Pulsanti/regole.png");
    menu->btnImpostazioni = LoadTexture("Pulsanti/impostazioni.png");
    menu->btnArchivio = LoadTexture("Pulsanti/archivio.png");
    menu->btnIndietro = LoadTexture("Pulsanti/indietro.png");

    menu->btnOnMusica = LoadTexture("Pulsanti/on.png");
    menu->btnOnAudio = LoadTexture("Pulsanti/on.png");
    menu->btnOff = LoadTexture("Pulsanti/off.png");

    // --- CONTROLLO ANTI-LEAK AUDIO ---
    // Carica l'effetto sonoro in RAM solo se non è già presente un campione valido nel contesto del gioco.
    // Questo evita allocazioni multiple e ridondanti ogni volta che l'utente torna al menu principale.
    if (!IsSoundValid(suono_bottone)) {
        suono_bottone = LoadSound("Musiche/suono_bottone.mp3");
    }

    // --- CONFIGURAZIONE DELLO STATO INIZIALE ---
    menu->musicaAttiva = true;
    menu->audioAttivo = true;
    effettiMenuAttivi = true;
    menu->nascondi_archivio = false;

    // --- CALCOLO DELLE DIMENSIONI PROPORZIONALI (ASPECT RATIO) ---
    float larghPulsantiBase = 440.0f; // Larghezza standard desiderata per la colonna centrale
    float larghArchivio = 340.0f;     // Larghezza standard per l'archivio storico
    float larghImpostazioni = 250.0f; // Larghezza standard per il pannello opzioni

    // Formule proporzionali: Altezza = AltezzaOriginale * (LarghezzaDesiderata / LarghezzaOriginale)
    menu->rect1vs1.width = larghPulsantiBase;
    menu->rect1vs1.height = (float)menu->btn1vs1.height * (larghPulsantiBase / (float)menu->btn1vs1.width);

    menu->rectMulti.width = larghPulsantiBase;
    menu->rectMulti.height = (float)menu->btnMulti.height * (larghPulsantiBase / (float)menu->btnMulti.width);

    menu->rectRegole.width = larghPulsantiBase;
    menu->rectRegole.height = (float)menu->btnRegole.height * (larghPulsantiBase / (float)menu->btnRegole.width);

    float hArch = (float)menu->btnArchivio.height * (larghArchivio / (float)menu->btnArchivio.width);
    float hImpo = (float)menu->btnImpostazioni.height * (larghImpostazioni / (float)menu->btnImpostazioni.width);

    // --- OFFSET DI CORREZIONE ESTETICA (Allineamento di precisione millimetrica sui pixel) ---
    float regola_1vs1_X = -20.0f;
    float regola_1vs1_Y = 0.0f;
    float regola_multi_X = -15.0f;
    float regola_multi_Y = 0.0f;
    float regola_regole_X = -10.0f;
    float regola_regole_Y = -18.0f;
    float regola_archivio_X = -20.0f;
    float regola_archivio_Y = -20.0f;
    float regola_impostazioni_X = 15.0f;
    float regola_impostazioni_Y = 0.0f;

    // --- POSIZIONAMENTO GEOMETRICO SULLO SCHERMO DI GIOCO ---
    float centroSchermoX = SCHERMO_LARGHEZZA / 2.0f; // Coordinata centrale orizzontale (600.0f)
    float startY = 210.0f;                            // Quota di partenza verticale della colonna centrale
    float spaziatura = -90.0f; // Margine negativo per compensare i bordi vuoti delle texture PNG

    // Posizionamento del Tasto 1vs1
    menu->rect1vs1.x = (centroSchermoX - (menu->rect1vs1.width / 2.0f)) + regola_1vs1_X;
    menu->rect1vs1.y = startY + regola_1vs1_Y;

    // Posizionamento del Tasto Multigiocatore (Agganciato in cascata geometrica sotto il primo)
    menu->rectMulti.x = (centroSchermoX - (menu->rectMulti.width / 2.0f)) + regola_multi_X;
    menu->rectMulti.y = (menu->rect1vs1.y + menu->rect1vs1.height + spaziatura) + regola_multi_Y;

    // Posizionamento del Tasto Regole (Agganciato in cascata geometrica sotto il secondo)
    menu->rectRegole.x = (centroSchermoX - (menu->rectRegole.width / 2.0f)) + regola_regole_X;
    menu->rectRegole.y = (menu->rectMulti.y + menu->rectMulti.height + spaziatura) + regola_regole_Y;

    // Posizionamento del Tasto Archivio (Ancorato nell'angolo in alto a sinistra)
    menu->rectArchivio.x = regola_archivio_X;
    menu->rectArchivio.y = regola_archivio_Y;
    menu->rectArchivio.width = larghArchivio;
    menu->rectArchivio.height = hArch;

    // Posizionamento del Tasto Impostazioni (Ancorato sul margine dell'angolo in alto a destra)
    float margineBordoDestro = 20.0f;
    float rigaAltezzaTopImpostazioni = 20.0f;
    menu->rectImpostazioni.x = (SCHERMO_LARGHEZZA - larghImpostazioni - margineBordoDestro) + regola_impostazioni_X;
    menu->rectImpostazioni.y = rigaAltezzaTopImpostazioni + regola_impostazioni_Y;
    menu->rectImpostazioni.width = larghImpostazioni;
    menu->rectImpostazioni.height = hImpo;

    // Posizionamento del Tasto Indietro (In basso a sinistra, scalato fisso al 10% della texture originale)
    float scalaTasto = 0.10f;
    float larghezzaIndietro = (float)menu->btnIndietro.width * scalaTasto;
    float altezzaIndietro = (float)menu->btnIndietro.height * scalaTasto;
    menu->rectIndietro = (Rectangle){ 25.0f, SCHERMO_ALTEZZA - altezzaIndietro - 25.0f, larghezzaIndietro, altezzaIndietro };
}

/**
 * Gestisce l'aggiornamento logico delle collisioni e il rendering del menu di scelta.
 * @return Intero rappresentante lo stato o la transizione da comunicare al main loop.
 */
int AggiornaDisegnaMenuScelta(MenuScelta *menu) {
    Vector2 mousePos = GetMousePosition();
    int statoSelezionato = STATO_MENU_SCELTA_CORRENTE;

    // Sincronizza lo stato degli effetti audio globali della scena
    effettiMenuAttivi = menu->audioAttivo;

    // =========================================================================
    // CONFIGURAZIONE DELLE HITBOX RETTANGOLARI REALI (AREE DI COLLISIONE)
    // Modifica queste misure per allargare o stringere le zone di click effettive
    // =========================================================================
    float tasti_W = 320.0f;        // Larghezza area click dei tasti centrali
    float tasti_H = 110.0f;        // Altezza area click dei tasti centrali

    float archivio_W = 200.0f;     // Larghezza area click del tasto Archivio
    float archivio_H = 100.0f;     // Altezza area click del tasto Archivio

    float impostazioni_W = 160.0f; // Larghezza area click del tasto Impostazioni
    float impostazioni_H = 90.0f;  // Altezza area click del tasto Impostazioni
    // =========================================================================

    // Generazione geometrica dei rettangoli di Hitbox centrati rispetto ai rettangoli grafici (Draw Rects)
    Rectangle h1vs1 = { menu->rect1vs1.x + (menu->rect1vs1.width - tasti_W) / 2.0f, menu->rect1vs1.y + (menu->rect1vs1.height - tasti_H) / 2.0f, tasti_W, tasti_H };
    Rectangle hMulti = { menu->rectMulti.x + (menu->rectMulti.width - tasti_W) / 2.0f, menu->rectMulti.y + (menu->rectMulti.height - tasti_H) / 2.0f, tasti_W, tasti_H };
    Rectangle hRegole = { menu->rectRegole.x + (menu->rectRegole.width - tasti_W) / 2.0f, menu->rectRegole.y + (menu->rectRegole.height - tasti_H) / 2.0f, tasti_W, tasti_H };

    Rectangle hArchivio = { menu->rectArchivio.x + (menu->rectArchivio.width - archivio_W) / 2.0f, menu->rectArchivio.y + (menu->rectArchivio.height - archivio_H) / 2.0f, archivio_W, archivio_H };
    Rectangle hImpostazioni = { menu->rectImpostazioni.x + (menu->rectImpostazioni.width - impostazioni_W) / 2.0f, menu->rectImpostazioni.y + (menu->rectImpostazioni.height - impostazioni_H) / 2.0f, impostazioni_W, impostazioni_H };

    // Per il tasto "Indietro" utilizziamo l'area nativa, essendo già definita su proporzioni ridotte
    Rectangle hIndietro = menu->rectIndietro;

    // --- GESTIONE INPUT E COLLISIONI MOUSE ---
    if (CheckCollisionPointRec(mousePos, h1vs1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_1VS1;
    }
    if (CheckCollisionPointRec(mousePos, hMulti) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_MULTIGIOCATORE;
    }
    if (CheckCollisionPointRec(mousePos, hRegole) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_REGOLE;
    }
    if (CheckCollisionPointRec(mousePos, hImpostazioni) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_IMPOSTAZIONI;
    }
    if (!menu->nascondi_archivio) {
        if (CheckCollisionPointRec(mousePos, hArchivio) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (effettiMenuAttivi) PlaySound(suono_bottone);
            statoSelezionato = VAI_ARCHIVIO;
        }
    }
    if (CheckCollisionPointRec(mousePos, hIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_INDIETRO;
    }

    // --- RENDERING DELLO SFONDO DI GIOCO ---
    if (menu->sfondo.id > 0) {
        DrawTexturePro(menu->sfondo,
            (Rectangle){ 0, 0, (float)menu->sfondo.width, (float)menu->sfondo.height },
            (Rectangle){ 0, 0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA },
            (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    // --- CALCOLO EFFETTO HOVER (Schiarimento della texture quando il cursore entra nella hitbox reale) ---
    Color c1vs1 = CheckCollisionPointRec(mousePos, h1vs1) ? LIGHTGRAY : WHITE;
    Color cMulti = CheckCollisionPointRec(mousePos, hMulti) ? LIGHTGRAY : WHITE;
    Color cRegole = CheckCollisionPointRec(mousePos, hRegole) ? LIGHTGRAY : WHITE;
    Color cImpostazioni = CheckCollisionPointRec(mousePos, hImpostazioni) ? LIGHTGRAY : WHITE;
    Color cIndietro = CheckCollisionPointRec(mousePos, hIndietro) ? LIGHTGRAY : WHITE;

    // --- DISEGNO DEL SET DI PULSANTI ---
    if (menu->btn1vs1.id > 0) DrawTexturePro(menu->btn1vs1, (Rectangle){0, 0, (float)menu->btn1vs1.width, (float)menu->btn1vs1.height}, menu->rect1vs1, (Vector2){0, 0}, 0.0f, c1vs1);
    if (menu->btnMulti.id > 0) DrawTexturePro(menu->btnMulti, (Rectangle){0, 0, (float)menu->btnMulti.width, (float)menu->btnMulti.height}, menu->rectMulti, (Vector2){0, 0}, 0.0f, cMulti);
    if (menu->btnRegole.id > 0) DrawTexturePro(menu->btnRegole, (Rectangle){0, 0, (float)menu->btnRegole.width, (float)menu->btnRegole.height}, menu->rectRegole, (Vector2){0, 0}, 0.0f, cRegole);
    if (menu->btnImpostazioni.id > 0) DrawTexturePro(menu->btnImpostazioni, (Rectangle){0, 0, (float)menu->btnImpostazioni.width, (float)menu->btnImpostazioni.height}, menu->rectImpostazioni, (Vector2){0, 0}, 0.0f, cImpostazioni);

    // Disegna l'archivio storico solo se l'utente corrente non è un ospite (Guest)
    if (!menu->nascondi_archivio) {
        Color cArchivio = CheckCollisionPointRec(mousePos, hArchivio) ? LIGHTGRAY : WHITE;
        if (menu->btnArchivio.id > 0) DrawTexturePro(menu->btnArchivio, (Rectangle){0, 0, (float)menu->btnArchivio.width, (float)menu->btnArchivio.height}, menu->rectArchivio, (Vector2){0, 0}, 0.0f, cArchivio);
    }

    if (menu->btnIndietro.id > 0) {
        DrawTexturePro(menu->btnIndietro, (Rectangle){ 0, 0, (float)menu->btnIndietro.width, (float)menu->btnIndietro.height }, menu->rectIndietro, (Vector2){ 0, 0 }, 0.0f, cIndietro);
    }

    // --- DEBUG GESTIONE HITBOX (Scommentare per calibrazione in CLion) ---
    // DrawRectangleLinesEx(h1vs1, 2, RED);
    // DrawRectangleLinesEx(hMulti, 2, RED);
    // DrawRectangleLinesEx(hRegole, 2, RED);
    // DrawRectangleLinesEx(hArchivio, 2, RED);
    // DrawRectangleLinesEx(hImpostazioni, 2, RED);

    return statoSelezionato; // Ritorna l'identificatore dello stato della scena al ciclo core
}

/**
 * Libera sistematicamente tutte le risorse caricate nella VRAM e nella memoria di sistema.
 * Previene Memory Leak e arresti anomali del framework grafico all'uscita o riavvio.
 */
void DeinizializzaMenuScelta(MenuScelta *menu) {
    if (menu == NULL) return;

    // Rilascio sicuro delle Texture di sfondo
    UnloadTexture(menu->sfondo);
    UnloadTexture(menu->sfondoImpostazioni);
    UnloadTexture(menu->sfondoArchivio);
    UnloadTexture(menu->sfondoRegole);

    // Rilascio sicuro delle Texture dei pulsanti d'azione
    UnloadTexture(menu->btn1vs1);
    UnloadTexture(menu->btnMulti);
    UnloadTexture(menu->btnRegole);
    UnloadTexture(menu->btnImpostazioni);
    UnloadTexture(menu->btnArchivio);
    UnloadTexture(menu->btnIndietro);

    // Rilascio sicuro degli switch di stato opzioni
    UnloadTexture(menu->btnOnMusica);
    UnloadTexture(menu->btnOnAudio);
    UnloadTexture(menu->btnOff);

    // --- DEALLOCAZIONE CONDIZIONALE SICURA AUDIO ---
    // Libera il file audio caricato dalla memoria del dispositivo solo se l'handle risulta ancora valido
    if (IsSoundValid(suono_bottone)) {
        UnloadSound(suono_bottone);
    }
}