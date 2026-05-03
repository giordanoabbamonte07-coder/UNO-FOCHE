#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define MANO_MAX 50
#define MAZZO_TOTALE 108
#define SCHERMO_LARGHEZZA 1200
#define SCHERMO_ALTEZZA 800
#define CARTA_LARGHEZZA 100
#define CARTA_ALTEZZA 150

// Definizione tipi di dato per colori e categorie di carte
typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SALTA, INVERTI, PIU_DUE, PIU_QUATTRO, CAMBIA_COLORE } Tipo;

typedef struct {
    Colore colore;
    Tipo tipo;
    int valore; // Usato solo se tipo == NUMERO
} Carta;

typedef struct {
    Carta mano[MANO_MAX];
    int num_carte;
} Giocatore;

// Stato globale della partita
typedef struct {
    Carta mazzo[MAZZO_TOTALE];
    int indice_mazzo;
    Carta cima;             // L'ultima carta scartata
    Giocatore giocatore;
    Giocatore cpu;
    Colore colore_attuale;  // Necessario per gestire il cambio colore (Nere)
    int turno;              // 0 = Giocatore, 1 = CPU
    int prossimo_turno;     // Supporto per la gestione turni dopo scelta colore
    bool scegli_colore;     // Flag per bloccare gioco durante scelta colore
    bool anima_foca;        // Trigger animazione foca popup
    float timer_animazione;
    bool partita_finita;
    bool suono_fine_giocato; 
    char messaggio[64];     
    float timer_messaggio;         

    // === VARIABILI PER LE ANIMAZIONI ===
    bool animando;          // Blocca il gioco mentre una carta si muove
    Carta carta_animata;    // La carta che sta volando verso il mazzo
    Vector2 inizio_animazione; // Coordinate di partenza
    Vector2 fine_animazione;    // Coordinate di arrivo
    Vector2 posizione_animazione; // Coordinate correnti durante il volo
    int da_giocatore;       // 0 = giocatore, 1 = cpu (serve per capire a chi dare gli effetti)
    float timer_anim;       // Da quanto tempo è partita l'animazione
    float durata_animazione; // Quanto deve durare l'animazione in secondi
} Partita;

void InizializzaPartita(Partita *p);
void AggiornaPartita(Partita *p);
void GestisciInput(Partita *p);
bool PuoGiocare(Carta c, Partita *p);
Carta Pesca(Partita *p);

#endif