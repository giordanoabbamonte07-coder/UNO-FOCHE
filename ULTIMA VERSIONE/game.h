#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <time.h>

#define MANO_MAX 50
#define MAZZO_TOTALE 108
#define SCHERMO_LARGHEZZA 1200
#define SCHERMO_ALTEZZA 800
#define CARTA_LARGHEZZA 100
#define CARTA_ALTEZZA 150

// Definizione di colori personalizzati pronti all'uso per i testi dell'archivio
#define COLORE_TESTO_DATA    (Color){ 60, 60, 60, 255 }      // Un grigio scuro molto elegante
#define COLORE_TESTO_EVENTO  (Color){ 200, 50, 50, 255 }     // Rosso opaco per 1vs1 o Multigiocatore

typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SALTA, INVERTI, PIU_DUE, PIU_QUATTRO, CAMBIA_COLORE } Tipo;

typedef struct {
    Colore colore;
    Tipo tipo;
    int valore;
} Carta;

typedef struct {
    Carta mano[MANO_MAX];
    int num_carte;
    bool ha_notificato_ultima;
    int difficolta_ia; // 0=facile, 1=difficile
} Giocatore;

typedef struct {
    Giocatore giocatori[4];
    int num_giocatori;

    int direzione;
    Carta mazzo[MAZZO_TOTALE];
    Carta scarto[MAZZO_TOTALE];
    int indice_mazzo;
    int carte_mazzo;
    int num_scarto;
    Carta cima;
    Colore colore_attuale;

    int turno;
    bool partita_finita;
    int vincitore;

    // Logica Animazione Dinamica
    bool animando;
    Carta carta_animata;
    Vector2 posizione_animazione;
    Vector2 inizio_animazione;
    Vector2 fine_animazione;
    float timer_anim;
    float durata_animazione;
    int da_giocatore;

    // Interfaccia e Stati Volatili
    char messaggio[64];
    float timer_messaggio;
    bool scegli_colore;
    bool anima_foca;
    int prossimo_turno;
    bool anima_deltaplano;
    float pos_x_deltaplano;
    bool suono_fine_giocato;
    bool puo_passare;
    bool ha_pescato;
    bool blocco_input;      // Flag volatile aggiunto per evitare il congelamento al caricamento
    float timer_attesa;     // Reset dei tempi di calcolo IA o animazioni

    // Sessione e Storico (Sincronizzati con archivio.c)
    bool is_guest;
    bool Whitehall_mode;    // Eventuali modalità future
    bool richiedi_uscita;
    char utente_corrente[12];
    char data_salvataggio[20]; // Aggiunto per salvare la data reale ("GG/MM/AAAA ORA:MIN")
} Partita;

typedef struct {
    char nickname[12];
    char data_ora[25];
    Partita dati_partita;
} RecordSalvataggio;

extern const char* nomi_colori_ita[];

// Funzioni di ciclo e logica di gioco
void InizializzaPartita(Partita *p, int num_giocatori);
void AggiornaPartita(Partita *p);
void GestisceInput(Partita *p);
bool PuoGiocare(Carta c, Partita *p);
bool GiocatoreHaMosse(Partita *p, int id_giocatore);
Carta Pesca(Partita *p);

// Funzione di utilità per risolvere il bug del lancio laterale
Vector2 OttieniPosizioneMazzoGiocatore(int index, int num_giocatori);

#endif // GAME_H
