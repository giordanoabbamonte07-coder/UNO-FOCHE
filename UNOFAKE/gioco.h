#ifndef GIOCO_H
#define GIOCO_H

#include "giocatore.h"
#include "mazzo.h"

#define MAX_GIOCATORI 4

typedef struct {
    Carta mazzo[MAZZO_SIZE];
    int topMazzo;

    Carta scarti[MAZZO_SIZE];
    int topScarti;

    Giocatore giocatori[MAX_GIOCATORI];
    int numGiocatori;

    int turno;
    int direzione;
} Gioco;

void inizializzaGioco(Gioco *g);
void prossimoTurno(Gioco *g);
void applicaEffetto(Gioco *g, Carta c);
int cartaValida(Carta c, Carta tavolo);

#endif