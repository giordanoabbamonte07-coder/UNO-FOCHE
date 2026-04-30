#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char* nomi_colori_ita[] = {"ROSSO", "GIALLO", "VERDE", "BLU", "NERO"};

// Mescola l'array del mazzo in modo casuale
static void MescolaMazzo(Game *g) {
    for (int j = 0; j < MAZZO_TOTALE; j++) {
        int r = GetRandomValue(0, MAZZO_TOTALE - 1);
        Carta tmp = g->mazzo[j];
        g->mazzo[j] = g->mazzo[r];
        g->mazzo[r] = tmp;
    }
}

// Genera le 108 carte standard di UNO
static void PreparaMazzo(Game *g) {
    int i = 0;
    for (int c = 0; c < 4; c++) {
        g->mazzo[i++] = (Carta){(Colore)c, NUMERO, 0}; // Uno zero per colore
        for (int v = 1; v <= 9; v++) {
            g->mazzo[i++] = (Carta){(Colore)c, NUMERO, v}; // Due numeri 1-9 per colore
            g->mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
        }
        for (int k = 0; k < 2; k++) { // Carte azione (2 per tipo per colore)
            g->mazzo[i++] = (Carta){(Colore)c, SKIP, 0};
            g->mazzo[i++] = (Carta){(Colore)c, REVERSE, 0};
            g->mazzo[i++] = (Carta){(Colore)c, PLUS2, 0};
        }
    }
    for (int j = 0; j < 4; j++) { // Carte nere (4 jolly e 4 +4)
        g->mazzo[i++] = (Carta){NERO, PLUS4, 0};
        g->mazzo[i++] = (Carta){NERO, CAMBIO_COLORE, 0};
    }
    g->indice_mazzo = 0;
    MescolaMazzo(g);
}

// Estrae una carta e rimischia se il mazzo è vuoto
Carta Pesca(Game *g) {
    if (g->indice_mazzo >= MAZZO_TOTALE) {
        g->indice_mazzo = 0;
        MescolaMazzo(g);
    }
    return g->mazzo[g->indice_mazzo++];
}

// Aggiunge carte alla mano del giocatore o del BOT
static void ApplicaMalus(Game *g, int vittima, int quantita) {
    Giocatore *target = (vittima == 0) ? &g->player : &g->cpu;
    for (int i = 0; i < quantita; i++) {
        if (target->num_carte < MANO_MAX) {
            target->mano[target->num_carte++] = Pesca(g);
        }
    }
}

// Logica di convalida mossa secondo le regole ufficiali
bool PuoGiocare(Carta c, Game *g) {
    if (c.colore == NERO) return true; // Le nere sono sempre giocabili
    if (c.colore == g->colore_attuale) return true;
    if (c.tipo == NUMERO && g->cima.tipo == NUMERO && c.valore == g->cima.valore) return true;
    if (c.tipo != NUMERO && c.tipo == g->cima.tipo) return true;
    return false;
}

static void ImpostaMessaggio(Game *g, const char* testo) {
    strncpy(g->messaggio, testo, 63);
    g->msgTimer = 2.5f;
}

// Semplice IA: sceglie il colore che ha di più in mano
static Colore ScegliMigliorColoreBOT(Giocatore *cpu) {
    int conteggio[4] = {0, 0, 0, 0};
    for (int i = 0; i < cpu->num_carte; i++) {
        if (cpu->mano[i].colore != NERO) conteggio[cpu->mano[i].colore]++;
    }
    int max = -1; Colore scelto = ROSSO;
    for (int i = 0; i < 4; i++) {
        if (conteggio[i] > max) { max = conteggio[i]; scelto = (Colore)i; }
    }
    return scelto;
}

void InitGame(Game *g) {
    g->player.num_carte = g->cpu.num_carte = 0;
    g->scegliColore = g->animFoca = g->game_over = false;
    g->msgTimer = 0;

    PreparaMazzo(g);

    for (int i = 0; i < 7; i++) { // Distribuzione iniziale
        g->player.mano[g->player.num_carte++] = Pesca(g);
        g->cpu.mano[g->cpu.num_carte++] = Pesca(g);
    }

    g->cima = Pesca(g);
    while(g->cima.colore == NERO) g->cima = Pesca(g); // La prima carta non può essere nera
    g->colore_attuale = g->cima.colore;
    g->turno = 0;
}

void HandleInput(Game *g) {
    if (g->game_over || g->turno != 0 || g->msgTimer > 0.5f) return;

    Vector2 m = GetMousePosition();

    // Input per scelta colore (dopo aver giocato una carta nera)
    if (g->scegliColore) {
        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(m, (Rectangle){ 300 + (float)i * 60, 250, 50, 50 }) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                g->colore_attuale = (Colore)i;
                g->scegliColore = false;
                g->animFoca = false;
                g->turno = g->prossimo_turno;
            }
        }
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Pesca dal mazzo
        if (CheckCollisionPointRec(m, (Rectangle){ 300, 250, 60, 90 })) {
            if (g->player.num_carte < MANO_MAX) {
                g->player.mano[g->player.num_carte++] = Pesca(g);
                g->turno = 1;
            }
            return;
        }

        // Gioca carta dalla mano
        int startX = 400 - (g->player.num_carte * 35);
        for (int i = 0; i < g->player.num_carte; i++) {
            float offset = (PuoGiocare(g->player.mano[i], g)) ? -20 : 0;
            if (CheckCollisionPointRec(m, (Rectangle){ (float)startX + i * 70, 450 + offset, 60, 90 })) {
                Carta c = g->player.mano[i];
                if (PuoGiocare(c, g)) {
                    g->cima = c;
                    for (int k = i; k < g->player.num_carte - 1; k++) g->player.mano[k] = g->player.mano[k+1];
                    g->player.num_carte--;

                    bool passaTurno = true;
                    if (c.tipo == SKIP || c.tipo == REVERSE || c.tipo == PLUS2 || c.tipo == PLUS4) passaTurno = false;

                    if (c.tipo == PLUS2) ApplicaMalus(g, 1, 2);
                    if (c.tipo == PLUS4) ApplicaMalus(g, 1, 4);

                    if (c.colore == NERO) {
                        g->scegliColore = true;
                        g->animFoca = true;
                        g->prossimo_turno = passaTurno ? 1 : 0;
                    } else {
                        g->colore_attuale = c.colore;
                        g->turno = passaTurno ? 1 : 0;
                    }
                    break;
                }
            }
        }
    }
}

void UpdateGame(Game *g) {
    if (g->game_over) return;
    if (g->msgTimer > 0) g->msgTimer -= GetFrameTime();

    // Logica decisionale del BOT
    if (g->turno == 1 && !g->scegliColore && g->msgTimer <= 0) {
        int indiceMossa = -1; int prioritaMax = -1;

        for (int i = 0; i < g->cpu.num_carte; i++) {
            if (PuoGiocare(g->cpu.mano[i], g)) {
                int p = 0; // Priorità: +4 > Altre Speciali > Colore Corrente
                if (g->cpu.mano[i].tipo == PLUS4) p = 5;
                else if (g->cpu.mano[i].tipo == PLUS2 || g->cpu.mano[i].tipo == SKIP || g->cpu.mano[i].tipo == REVERSE) p = 4;
                else if (g->cpu.mano[i].colore == g->colore_attuale) p = 3;
                if (p > prioritaMax) { prioritaMax = p; indiceMossa = i; }
            }
        }

        if (indiceMossa != -1) {
            Carta c = g->cpu.mano[indiceMossa];
            g->cima = c;
            bool passaTurno = true;
            if (c.tipo == SKIP || c.tipo == REVERSE || c.tipo == PLUS2 || c.tipo == PLUS4) passaTurno = false;

            if (c.tipo == PLUS2) { ApplicaMalus(g, 0, 2); ImpostaMessaggio(g, "BOT: +2 CARTE!"); }
            if (c.tipo == SKIP || c.tipo == REVERSE) ImpostaMessaggio(g, "BOT: TURNO BLOCCATO!");

            char buf[64];
            if (c.colore == NERO) {
                g->colore_attuale = ScegliMigliorColoreBOT(&g->cpu);
                if (c.tipo == PLUS4) {
                    ApplicaMalus(g, 0, 4);
                    sprintf(buf, "BOT: +4! COLORE SCELTO: %s", nomi_colori_ita[g->colore_attuale]);
                } else {
                    sprintf(buf, "BOT HA SCELTO IL COLORE: %s", nomi_colori_ita[g->colore_attuale]);
                }
                ImpostaMessaggio(g, buf);
            } else {
                g->colore_attuale = c.colore;
            }

            for (int k = indiceMossa; k < g->cpu.num_carte - 1; k++) g->cpu.mano[k] = g->cpu.mano[k+1];
            g->cpu.num_carte--;
            g->turno = passaTurno ? 0 : 1;
        } else {
            if (g->cpu.num_carte < MANO_MAX) g->cpu.mano[g->cpu.num_carte++] = Pesca(g);
            g->turno = 0;
        }

        if (g->player.num_carte == 0 || g->cpu.num_carte == 0) g->game_over = true;
    }
}