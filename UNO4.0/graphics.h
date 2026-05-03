#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

#define CARTA_LARGHEZZA 100
#define CARTA_ALTEZZA 150
#define SCHERMO_LARGHEZZA 1200
#define SCHERMO_ALTEZZA 800

// Struttura che contiene tutti i puntatori alle texture caricate
typedef struct {
    Texture2D sfondo;
    Texture2D foca;
    Texture2D foca_vittoria;     // Foca per la vittoria
    Texture2D foca_sconfitta;  // Foca per la sconfitta
    Texture2D retro;
    Texture2D carte[5][6][13]; // Colore | Tipo | Valore
} Grafica;

void InizializzaGrafica(Grafica *gfx);
void DisegnaPartita(Grafica *gfx, Partita *p);
void ScaricaGrafica(Grafica *gfx);
Texture2D OttieniCarta(Grafica *gfx, Carta c);
Texture2D CreaFallback(void);
Texture2D CaricaSicuro(const char *path);

// Aggiunto parametro per la rotazione (serve per il ventaglio e le animazioni)
void DisegnaCarta(Texture2D t, float x, float y, float rotazione);

#endif