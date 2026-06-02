#ifndef MENU_H
#define MENU_H

#include "raylib.h"

// =======================================================================================
// ENUMERAZIONE: STATI MACCHINA DEL GIOCO
// =======================================================================================
// Rappresenta tutte le possibili schermate (scene) gestite dal ciclo principale nel main
typedef enum {
    STATO_MENU,         // Schermata iniziale di avvio (Login / Registrazione / Ospite)
    STATO_ACCEDI,       // Pannello di inserimento credenziali per il Login dell'utente
    STATO_REGISTRATI,   // Pannello per la creazione di un nuovo account sul file binario
    STATO_DEBUG,        // Schermata di servizio per i test delle funzioni logiche
    STATO_MENU_SCELTA,  // Menu di selezione modalità (1vs1, Multigiocatore, Regole, ecc.)
    STATO_GIOCO,        // Tavolo di gioco attivo della partita di UNO FOCA
    STATO_REGOLE,       // Visualizzazione del regolamento ufficiale di gioco
    STATO_IMPOSTAZIONI, // Pannello di controllo dei volumi e dei toggle audio
    STATO_ARCHIVIO,     // Storico delle partite salvate sul disco (Sospese/Concluse)
    STATO_STATISTICHE   // Visualizzazione dei record di vittorie e sconfitte dell'utente
} StatoGioco;

// =======================================================================================
// STRUTTURA DATI: INTERFACCIA GRAFICA DEL MENU (MenuGrafica)
// =======================================================================================
typedef struct {
    // --- TEXTURE SFONDI E PANNELLI ---
    Texture2D sfondo;            // Sfondo animato o fisso della schermata di benvenuto principale
    Texture2D sfondo_accedi;     // Immagine di background per il modulo di Login
    Texture2D sfondo_registrati;  // Immagine di background per il modulo di Registrazione

    // --- TEXTURE BOTTONI ---
    Texture2D btn_accedi;        // Pulsante per confermare l'intenzione di loggarsi
    Texture2D btn_registrati;    // Pulsante per aprire o confermare la registrazione
    Texture2D btn_ospite;        // Pulsante per giocare senza account (Modalità Guest)

    // --- TEXTURE ETICHETTE / TEXTBOX ---
    Texture2D tex_utente;        // Grafica decorativa o icona per il campo "Username"
    Texture2D tex_password;      // Grafica decorativa o icona per il campo "Password"

    // --- BUFFER INPUT TESTUALE ---
    char nickname[12];           // Stringa di input per il nome (11 caratteri + '\0')
    char password[12];           // Stringa di input per la password (11 caratteri + '\0')

    // --- VARIABILI DI CONTROLLO INTERNE ---
    int campo_attivo;            // Determina quale TextBox riceve l'input (es: 0 = Nessuno, 1 = Nick, 2 = Pass)
    int frame_counter;           // Contatore di fotogrammi (usato per l'effetto lampeggiante del cursore '|')
} MenuGrafica;

// =======================================================================================
// STRUTTURA DATI: RECORD DATABASE UTENTE (UtenteRegistrato)
// =======================================================================================
// Rappresenta il layout binario fisso che viene salvato o letto dal file di log degli utenti
typedef struct {
    char nickname[11];           // Nome utente effettivo memorizzato nel database locale
    char password[11];           // Password associata all'account
    char ultimo_accesso[24];     // Stringa formattata contenente data e ora dell'ultimo login
    int ultima_azione;           // Specifica l'operazione eseguita (0 = Registrazione, 1 = Login)
} UtenteRegistrato;

// =======================================================================================
// INTERFACCIA DELLE FUNZIONI (PROTOTIPI)
// =======================================================================================

/**
 * @brief Carica in VRAM le texture del menu e azzera i buffer di testo di nickname e password.
 * @param mg Puntatore alla struttura MenuGrafica da inizializzare.
 */
void InizializzaMenu(MenuGrafica *mg);

/**
 * @brief Gestisce l'input del mouse e disegna i pulsanti iniziali (Accedi, Registrati, Ospite).
 * @param mg Puntatore alle risorse grafiche del menu.
 * @return StatoGioco Il nuovo stato della macchina a stati derivato dal click dell'utente.
 */
StatoGioco AggiornaDisegnaMenu(MenuGrafica *mg);

/**
 * @brief Gestisce l'input della tastiera (scrittura nelle textbox) e renderizza i form di login/registrazione.
 * @param mg Puntatore alle risorse grafiche del menu.
 * @param stato Lo stato corrente (STATO_ACCEDI o STATO_REGISTRATI) per capire quale logica applicare.
 * @return StatoGioco Restituisce lo stato corrente se l'utente sta scrivendo, o cambia stato in caso di successo/annullamento.
 */
StatoGioco GestisciSchermataAuth(MenuGrafica *mg, StatoGioco stato);

/**
 * @brief Scarica in modo sicuro tutte le texture caricate in VRAM per la schermata di autenticazione.
 * @param mg Puntatore alla struttura MenuGrafica da liberare.
 */
void ScaricaMenu(MenuGrafica *mg);

#endif // MENU_H