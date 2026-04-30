#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define MANO_MAX 50
#define MAZZO_TOTALE 108

// Definizione tipi di dato per colori e categorie di carte
typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SKIP, REVERSE, PLUS2, PLUS4, CAMBIO_COLORE } Tipo;

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
    Giocatore player;
    Giocatore cpu;
    Colore colore_attuale;  // Necessario per gestire il cambio colore (Nere)
    int turno;              // 0 = Player, 1 = BOT
    int prossimo_turno;     // Supporto per la gestione turni dopo scelta colore
    bool scegliColore;      // Flag per bloccare gioco durante scelta colore
    bool animFoca;          // Trigger animazione foca popup
    float animTimer;
    bool game_over;
    char messaggio[64];     // Messaggio di stato visualizzato a schermo
    float msgTimer;         // Durata visibilità messaggio
} Game;

void InitGame(Game *g);
void UpdateGame(Game *g);
void HandleInput(Game *g);
bool PuoGiocare(Carta c, Game *g);
Carta Pesca(Game *g);

#endif