#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define MANO_MAX 50
#define MAZZO_TOTALE 108

typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SKIP, REVERSE, PLUS2, PLUS4, CAMBIO_COLORE } Tipo;

typedef struct {
    Colore colore;
    Tipo tipo;
    int valore;
} Carta;

typedef struct {
    Carta mano[MANO_MAX];
    int num_carte;
} Giocatore;

typedef struct {
    Carta mazzo[MAZZO_TOTALE];
    int indice_mazzo;
    Carta cima;
    Giocatore player;
    Giocatore cpu;
    Colore colore_attuale;
    int turno;
    int prossimo_turno; // Per gestire i salti turno (+2, +4, skip)
    bool scegliColore;
    bool animFoca;
    float animTimer;
    bool game_over;

    char messaggio[64];
    float msgTimer;
} Game;

void InitGame(Game *g);
void UpdateGame(Game *g);
void HandleInput(Game *g);
bool PuoGiocare(Carta c, Game *g);
Carta Pesca(Game *g);

#endif