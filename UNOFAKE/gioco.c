#include "gioco.h"

int cartaValida(Carta c, Carta tavolo) {
    return (c.colore == tavolo.colore || c.valore == tavolo.valore || c.colore == JOLLY);
}

void prossimoTurno(Gioco *g) {
    // Gestisce il cambio turno in base alla direzione (normale o invertita)
    g->turno = (g->turno + g->direzione + g->numGiocatori) % g->numGiocatori;
}

void applicaEffetto(Gioco *g, Carta c) {
    if (c.valore == SKIP) {
        prossimoTurno(g);
    } else if (c.valore == REVERSE) {
        g->direzione *= -1;
    } else if (c.valore == PIU2) {
        prossimoTurno(g);
        for (int i = 0; i < 2; i++)
            g->giocatori[g->turno].mano[g->giocatori[g->turno].numCarte++] = pesca(g->mazzo, &g->topMazzo);
    } else if (c.valore == PIU4) {
        prossimoTurno(g);
        for (int i = 0; i < 4; i++)
            g->giocatori[g->turno].mano[g->giocatori[g->turno].numCarte++] = pesca(g->mazzo, &g->topMazzo);
    }
}

void inizializzaGioco(Gioco *g) {
    g->numGiocatori = 4;
    g->direzione = 1;
    g->turno = 0;
    g->topScarti = 0;

    inizializzaMazzo(g->mazzo, &g->topMazzo);
    mescola(g->mazzo, g->topMazzo);

    for (int i = 0; i < g->numGiocatori; i++) {
        g->giocatori[i].numCarte = 0;
        g->giocatori[i].isBot = (i == 0) ? 0 : 1;
        for (int j = 0; j < 7; j++) {
            g->giocatori[i].mano[g->giocatori[i].numCarte++] = pesca(g->mazzo, &g->topMazzo);
        }
    }
    g->scarti[g->topScarti++] = pesca(g->mazzo, &g->topMazzo);
}