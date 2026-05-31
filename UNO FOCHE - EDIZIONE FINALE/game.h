#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <time.h>

// =======================================================================================
// COSTANTI DI CONFIGURAZIONE MACRO
// =======================================================================================
#define MANO_MAX 50              // Numero massimo di carte che un giocatore può tenere in mano
#define MAZZO_TOTALE 108         // Dimensione totale del mazzo standard di UNO
#define SCHERMO_LARGHEZZA 1200   // Risoluzione orizzontale nativa della finestra di gioco
#define SCHERMO_ALTEZZA 800      // Risoluzione verticale nativa della finestra di gioco
#define CARTA_LARGHEZZA 100      // Dimensione orizzontale standard delle carte a schermo
#define CARTA_ALTEZZA 150        // Dimensione verticale standard delle carte a schermo

// Definizione di colori personalizzati pronti all'uso per i testi dell'archivio storico
#define COLORE_TESTO_DATA    (Color){ 60, 60, 60, 255 }      // Un grigio scuro opaco ed elegante
#define COLORE_TESTO_EVENTO  (Color){ 200, 50, 50, 255 }     // Rosso opaco per identificare 1vs1 o Multigiocatore

// =======================================================================================
// ENUMERAZIONI: ATTRIBUTI DELLE CARTE
// =======================================================================================
typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SALTA, INVERTI, PIU_DUE, PIU_QUATTRO, CAMBIA_COLORE } Tipo;

// =======================================================================================
// STRUTTURE DATI COMPONENTI
// =======================================================================================

/**
 * Rappresenta l'oggetto Carta con le sue proprietà di gioco.
 */
typedef struct {
    Colore colore;  // Colore della carta (Rosso, Giallo, Verde, Blu, Nero per i Jolly)
    Tipo tipo;      // Categoria (Numero standard o carta speciale ad effetto)
    int valore;     // Valore numerico effettivo (valido solo se tipo == NUMERO, da 0 a 9)
} Carta;

/**
 * Gestisce lo stato e la mano di ogni singolo partecipante (umano o bot).
 */
typedef struct {
    Carta mano[MANO_MAX];       // Vettore statico contenente le carte possedute dal giocatore
    int num_carte;              // Conteggio reale delle carte correntemente presenti in mano
    bool ha_notificato_ultima;  // Flag di sicurezza per verificare se il giocatore ha dichiarato "UNO!"
    int difficolta_ia;          // Profilo comportamentale della CPU (0 = Facile, 1 = Difficile)
} Giocatore;

/**
 * La struttura centrale del motore di gioco. Mantiene l'intera sessione attiva.
 */
typedef struct {
    // --- PARTECIPANTI ---
    Giocatore giocatori[4];     // Array dei 4 giocatori potenziali (Indice 0: Umano, Indici 1-3: Bot)
    int num_giocatori;          // Numero reale di partecipanti configurati nella stanza (2 o 4)

    // --- COSTRUTTI DEL MAZZO ---
    int direzione;              // Senso orario (1) o senso antiorario (-1) del giro di turni
    Carta mazzo[MAZZO_TOTALE];  // Vettore delle carte coperte da cui attingere per pescare
    Carta scarto[MAZZO_TOTALE]; // Vettore delle carte giocate sul tavolo (pila degli scarti)
    int indice_mazzo;           // Posizione dell'indice di lettura nel mazzo di pesca
    int carte_mazzo;            // Numero totale di carte caricate/rimaste nel mazzo corrente
    int num_scarto;             // Numero totale di carte accumulate nella pila degli scarti
    Carta cima;                 // La carta attiva in cima agli scarti che comanda le regole del tavolo
    Colore colore_attuale;      // Colore valido in vigore (cambia in caso di uso di carte jolly nere)

    // --- STATO DEL FLUSSO CORRENTE ---
    int turno;                  // Indice del giocatore di cui si attende la mossa (da 0 a num_giocatori-1)
    bool partita_finita;        // Flag di chiusura: diventa vero quando un giocatore azzera la propria mano
    int vincitore;              // Indice del giocatore che ha trionfato nel match

    // --- LOGICA ANIMAZIONE DINAMICA (Spostamento fluido delle carte) ---
    bool animando;              // Diventa vero quando una carta si sta muovendo da un punto A a un punto B
    Carta carta_animata;        // Copia dei dati della carta attualmente soggetta ad animazione a schermo
    Vector2 posizione_animazione;// Coordinate X,Y calcolate frame per frame durante lo spostamento
    Vector2 inizio_animazione;  // Punto geometrico X,Y di partenza del movimento (es: mazzo di pesca)
    Vector2 fine_animazione;    // Punto geometrico X,Y di arrivo del movimento (es: tavolo degli scarti)
    float timer_anim;           // Progressivo temporale normalizzato dell'animazione (da 0.0f a 1.0f)
    float durata_animazione;    // Tempo totale espresso in secondi per il completamento del movimento grafico
    int da_giocatore;           // Identifica chi ha originato il lancio o il prelievo della carta animata

    // --- INTERFACCIA UTENTE E STATI VOLATILI (FRAME-BY-FRAME) ---
    char messaggio[64];         // Testo informativo da mostrare a schermo (es: "BOT 2 PESCA +4")
    float timer_messaggio;      // Tempo residuo di persistenza a schermo del banner di testo
    bool scegli_colore;         // Attiva il pannello grafico modale di selezione colore per l'utente umano
    bool anima_foca;            // Attiva il rendering del pop-up animato della foca sopra la selezione colore
    int prossimo_turno;         // Variabile ausiliaria per pre-calcolare il turno successivo durante gli effetti
    bool anima_deltaplano;      // Attiva l'animazione dell'easter egg del deltaplano a schermo
    float pos_x_deltaplano;     // Coordinata X orizzontale progressiva del deltaplano in volo
    bool suono_fine_giocato;    // Evita la riproduzione in loop continuo del file audio di vittoria/sconfitta
    bool puo_passare;           // Flag di sblocco del tasto "Passa" se l'utente ha già pescato ma non ha carte utili
    bool ha_pescato;            // Impedisce al giocatore umano di pescare più di una volta nello stesso turno
    bool blocco_input;          // Flag volatile aggiunto per evitare il congelamento al caricamento
    float timer_attesa;         // Reset dei tempi di calcolo IA o animazioni

    // --- SESSIONE E PERSISTENZA (Sincronizzati con il modulo archivio.c) ---
    bool is_guest;              // Identifica se l'utente gioca come Ospite (disabilita salvataggi e cronologia)
    bool Whitehall_mode;        // Flag per abilitare varianti speciali o modalità di debug future
    bool richiedi_uscita;       // Segnala al ciclo principale la volontà di abbandonare anzitempo la partita
    char utente_corrente[12];   // Nome dell'utente loggato associato alla sessione di gioco attiva
    char data_salvataggio[20];  // Stringa temporale reale ("GG/MM/AAAA ORA:MIN") timbrata all'atto del salvataggio
} Partita;

/**
 * Struttura per il record di salvataggio delle partite sul file binario dell'archivio.
 */
typedef struct {
    char nickname[12];          // Nome utente che ha effettuato il salvataggio (11 caratteri + '\0')
    char data_ora[25];          // Data e ora reali in cui è stato generato il file di salvataggio
    Partita dati_partita;       // Dump completo dell'intera struttura partita per consentire il ripristino esatto dello stato
} RecordSalvataggio;

// =======================================================================================
// DICHIARAZIONI DI VARIABILI ESTERNE E PROTOTIPI DI FUNZIONE
// =======================================================================================

// Array globale contenente le stringhe in italiano dei nomi dei colori, definito nel rispettivo file .c
extern const char* nomi_colori_ita[];

/**
 * @brief Azzera la memoria, genera il mazzo da 108 carte, lo mescola e distribuisce le mani iniziali ai giocatori.
 * @param p Puntatore alla struttura Partita da configurare.
 * @param num_giocatori Quantità di sfidanti al tavolo (2 o 4).
 */
void InizializzaPartita(Partita *p, int num_giocatori);

/**
 * @brief Aggiorna la logica, calcola i turni, muove i timer e gestisce le decisioni dell'IA dei bot.
 * @param p Puntatore alla struttura Partita.
 */
void AggiornaPartita(Partita *p);

/**
 * @brief Intercetta i click del mouse dell'utente umano sulle carte in mano o sul mazzo di pesca.
 * @param p Puntatore alla struttura Partita.
 */
void GestisceInput(Partita *p);

/**
 * @brief Applica le regole ufficiali di UNO per stabilire se una carta può essere scartata sopra quella in cima.
 * @param c La carta che si intende giocare dalla mano.
 * @param p Puntatore alla struttura Partita per verificare il colore attuale e il valore in cima.
 * @return true Se la mossa è valida secondo il regolamento, false altrimenti.
 */
bool PuoGiocare(Carta c, Partita *p);

/**
 * @brief Esegue un controllo a tappeto sulla mano di un giocatore per scoprire se possiede almeno una carta valida da scartare.
 * @param p Puntatore alla struttura Partita.
 * @param id_giocatore Identificativo numerico del giocatore da controllare.
 * @return true Se ha mosse valide disponibili, false se è costretto a pescare dal mazzo.
 */
bool GiocatoreHaMosse(Partita *p, int id_giocatore);

/**
 * @brief Estrae una carta dal mazzo coperto. Se il mazzo è esaurito, rigenera le carte prendendole dagli scarti e rimescolandole.
 * @param p Puntatore alla struttura Partita.
 * @return Carta L'oggetto Carta rimosso dal mazzo e pescato.
 */
Carta Pesca(Partita *p);

/**
 * @brief Calcola e restituisce il punto geometrico X,Y sullo schermo in cui risiede il mazzo di un determinato giocatore.
 * Risolve il problema del lancio laterale errato delle carte animate calcolando l'origine esatta per ogni bot.
 * @param index Indice del giocatore (da 0 a 3).
 * @param num_giocatori Numero totale di giocatori attivi nella partita corrente.
 * @return Vector2 Coordinate bidimensionali del mazzo traguardo.
 */
Vector2 OttieniPosizioneMazzoGiocatore(int index, int num_giocatori);

#endif // GAME_H