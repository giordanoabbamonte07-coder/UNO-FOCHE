#ifndef CARTE_H
#define CARTE_H

#include "raylib.h"

// COLORI
#define ROSSO 0
#define BLU 1
#define GIALLO 2
#define VERDE 3
#define JOLLY 4

// VALORI
#define SKIP 10
#define REVERSE 11
#define PIU2 12
#define JOLLY_CARD 13
#define PIU4 14

typedef struct {
    int colore;
    int valore;
    Texture2D texture;
} Carta;

Carta creaCarta(int colore, int valore);
Texture2D caricaTexture(int colore, int valore);

#endif