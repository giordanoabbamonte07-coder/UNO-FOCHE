#ifndef GIOCATORE_H
#define GIOCATORE_H

#include "carte.h"

#define MAX_CARTE 30

typedef struct {
    Carta mano[MAX_CARTE];
    int numCarte;
    int isBot;
} Giocatore;

void rimuoviCarta(Giocatore *g, int index);
int scegliCartaBot(Giocatore *g, Carta tavolo);

#endif