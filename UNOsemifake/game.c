#include "game.h"

void PreparaMazzo(Game *g) {
    int i = 0;

    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++)
            g->mazzo[i++] = (Carta){c, NUMERO, v};

        for (int k = 0; k < 2; k++) {
            g->mazzo[i++] = (Carta){c, SKIP};
            g->mazzo[i++] = (Carta){c, REVERSE};
            g->mazzo[i++] = (Carta){c, PLUS2};
        }
    }

    for (int j = 0; j < 4; j++) {
        g->mazzo[i++] = (Carta){NERO, PLUS4};
        g->mazzo[i++] = (Carta){NERO, CAMBIO_COLORE};
    }

    g->indice_mazzo = i;

    for (int j = 0; j < i; j++) {
        int r = GetRandomValue(0, i - 1);
        Carta tmp = g->mazzo[j];
        g->mazzo[j] = g->mazzo[r];
        g->mazzo[r] = tmp;
    }
}

Carta Pesca(Game *g) {
    return g->mazzo[--g->indice_mazzo];
}

bool PuoGiocare(Carta c, Game *g) {
    return (c.colore == g->colore_attuale ||
            c.valore == g->cima.valore ||
            c.colore == NERO);
}

void ApplicaEffetto(Game *g, Carta c) {
    if (c.tipo == PLUS2)
        for (int i = 0; i < 2; i++)
            g->cpu.mano[g->cpu.num_carte++] = Pesca(g);

    if (c.tipo == PLUS4)
        for (int i = 0; i < 4; i++)
            g->cpu.mano[g->cpu.num_carte++] = Pesca(g);

    g->turno = (g->turno + 1) % 2;
}

void InitGame(Game *g) {
    g->player.num_carte = 0;
    g->cpu.num_carte = 0;

    PreparaMazzo(g);

    for (int i = 0; i < 7; i++) {
        g->player.mano[g->player.num_carte++] = Pesca(g);
        g->cpu.mano[g->cpu.num_carte++] = Pesca(g);
    }

    g->cima = Pesca(g);
    g->colore_attuale = g->cima.colore;
    g->turno = 0;
    g->status = STATUS_PLAYING;
}

void UpdateGameLogic(Game *g) {
    if (g->turno == 1) {
        for (int i = 0; i < g->cpu.num_carte; i++) {
            if (PuoGiocare(g->cpu.mano[i], g)) {
                g->cima = g->cpu.mano[i];

                for (int k = i; k < g->cpu.num_carte - 1; k++)
                    g->cpu.mano[k] = g->cpu.mano[k + 1];

                g->cpu.num_carte--;
                ApplicaEffetto(g, g->cima);
                return;
            }
        }

        g->cpu.mano[g->cpu.num_carte++] = Pesca(g);
        g->turno = 0;
    }
}