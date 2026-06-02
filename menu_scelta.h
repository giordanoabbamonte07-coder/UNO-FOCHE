#ifndef MENU_SCELTA_H
#define MENU_SCELTA_H

#include "raylib.h"

// =======================================================================================
// COSTANTI DI RITORNO (MACCHINA A STATI)
// =======================================================================================
// Valori restituiti da AggiornaDisegnaMenuScelta per direzionare il flusso nel main loop
#define STATO_MENU_SCELTA_CORRENTE 0  // L'utente non ha premuto nulla, rimani nel menu scelta
#define VAI_1VS1                   1  // Avvia partita contro 1 Bot
#define VAI_MULTIGIOCATORE         2  // Avvia partita contro 3 Bot (4 giocatori totali)
#define VAI_REGOLE                 3  // Vai alla schermata del regolamento
#define VAI_IMPOSTAZIONI           4  // Vai al pannello di controllo Audio/Musica
#define VAI_ARCHIVIO               5  // Vai allo storico delle partite salvate
#define VAI_INDIETRO               6  // Ritorna al Menu Principale (schermata di Login/Auth)

// =======================================================================================
// STRUTTURA DATI: MenuScelta
// =======================================================================================
typedef struct {
    // --- TEXTURE BACKGROUND ---
    Texture2D sfondo;             // Sfondo del menu di selezione principale
    Texture2D sfondoImpostazioni; // Sfondo dedicato al pannello opzioni
    Texture2D sfondoArchivio;     // Sfondo per la schermata dei salvataggi
    Texture2D sfondoRegole;       // Sfondo per la schermata delle regole di gioco

    // --- TEXTURE PULSANTI ---
    Texture2D btn1vs1;            // Bottone per la modalità locale a 2 giocatori
    Texture2D btnMulti;           // Bottone per la modalità multiplayer a 4 giocatori
    Texture2D btnRegole;          // Bottone per accedere al testo del regolamento
    Texture2D btnImpostazioni;    // Bottone per accedere ai settaggi di sistema
    Texture2D btnArchivio;        // Bottone per visualizzare i vecchi salvataggi binari
    Texture2D btnIndietro;        // Bottone universale di ritorno/recesso

    // --- TEXTURE INTERRUTTORI AUDIO ---
    Texture2D btnOnMusica;        // Icona di attivazione musica di sottofondo
    Texture2D btnOnAudio;         // Icona di attivazione effetti sonori (SFX)
    Texture2D btnOff;             // Icona generica di stato disattivato (Mute)

    // --- RETTANGOLI DI COLLISIONE (HITBOX MOUSE) ---
    Rectangle rectOnMusica;       // Area di click per il Mute/Unmute della musica
    Rectangle rectOnAudio;        // Area di click per il Mute/Unmute dei suoni
    Rectangle rect1vs1;           // Area di click per avviare il match 1vs1
    Rectangle rectMulti;          // Area di click per avviare il match a 4 giocatori
    Rectangle rectRegole;         // Area di click per aprire il regolamento
    Rectangle rectImpostazioni;   // Area di click per aprire le impostazioni
    Rectangle rectArchivio;       // Area di click per aprire lo storico partite
    Rectangle rectIndietro;       // Area di click per tornare al menu iniziale

    // --- FLAG DI STATO INTERNI ---
    bool nascondi_archivio;       // Se true, il pulsante archivio viene nascosto e reso incliccabile (es. per Guest)
    bool musicaAttiva;            // Stato corrente della traccia musicale (.mp3) di sottofondo
    bool audioAttivo;             // Stato corrente degli effetti sonori nei menu e nel gameplay
} MenuScelta;

// =======================================================================================
// INTERFACCIA DELLE FUNZIONI (PROTOTIPI)
// =======================================================================================

/**
 * @brief Carica in VRAM le texture e calcola le posizioni iniziali dei rettangoli di collisione.
 * @param menu Puntatore alla struttura MenuScelta da inizializzare.
 */
void InizializzaMenuScelta(MenuScelta *menu);

/**
 * @brief Gestisce l'input del mouse (hover, click) e si occupa del rendering degli elementi grafici attivi.
 * @param menu Puntatore alla struttura MenuScelta corrente.
 * @return int Una delle costanti numeriche definite sopra (es. VAI_1VS1, STATO_MENU_SCELTA_CORRENTE).
 */
int AggiornaDisegnaMenuScelta(MenuScelta *menu);

/**
 * @brief Scarica in modo sicuro tutte le texture precedentemente allocate per evitare memory leak grafici.
 * @param menu Puntatore alla struttura MenuScelta da deallocare.
 */
void DeinizializzaMenuScelta(MenuScelta *menu);

#endif // MENU_SCELTA_H