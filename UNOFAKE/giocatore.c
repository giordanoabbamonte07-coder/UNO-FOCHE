#include "giocatore.h"
#include "gioco.h"

void rimuoviCarta(Giocatore *g, int index) {
    for (int i = index; i < g->numCarte - 1; i++) {
        g->mano[i] = g->mano[i + 1];
    }
    g->numCarte--;
}

int scegliCartaBot(Giocatore *g, Carta tavolo) {
    for (int i = 0; i < g->numCarte; i++) {
        // Il bot controlla se ha una carta valida rispetto a quella sul tavolo
        if (cartaValida(g->mano[i], tavolo)) {
            return i; // Restituisce l'indice della carta da giocare
        }
    }
    return -1; // Se non ha carte valide, deve pescare
}