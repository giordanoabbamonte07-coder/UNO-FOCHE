#ifndef GAME_H
#define GAME_H

#include "raylib.h"

#define MANO_MAX 50

typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SKIP, REVERSE, PLUS2, PLUS4, CAMBIO_COLORE } Tipo;
typedef enum { STATUS_PLAYING, STATUS_SELECTING_COLOR, STATUS_GAME_OVER } GameStatus;

typedef struct {
    Colore colore;
    Tipo tipo;
    int valore;
    Vector2 pos;
    float rot;
} Carta;

typedef struct {
    Carta mano[MANO_MAX];
    int num_carte;
} Giocatore;

typedef struct {
    Carta mazzo[108];
    int indice_mazzo;

    Carta cima;
    Carta vecchiaCima;

    Giocatore player;
    Giocatore cpu;

    Colore colore_attuale;
    int turno;
    GameStatus status;

    float cpuTimer;
} Game;

// funzioni
void InitGame(Game *g);
Carta Pesca(Game *g);
bool PuoGiocare(Carta c, Game *g);
void ApplicaEffetto(Game *g, Carta c);
void UpdateGameLogic(Game *g);

#endif