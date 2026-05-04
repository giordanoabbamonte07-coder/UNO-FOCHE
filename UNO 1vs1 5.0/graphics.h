#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

// Struttura che contiene tutti i puntatori alle texture caricate
typedef struct {
    Texture2D sfondo;
    Texture2D foca;
    Texture2D foca_vittoria;
    Texture2D foca_sconfitta;
    Texture2D retro;
    Texture2D carte[5][6][13]; // Colore | Tipo | Valore
} Grafica;

void InizializzaGrafica(Grafica *gfx);
void DisegnaPartita(Grafica *gfx, Partita *p);
void ScaricaGrafica(Grafica *gfx);
Texture2D OttieniCarta(Grafica *gfx, Carta c);
Texture2D CreaFallback(void);
Texture2D CaricaSicuro(const char *path);
void DisegnaCarta(Texture2D t, float x, float y, float rotazione);
void EvidenziaCarta(float cx, float cy, float rotazione, Color colore);
void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor);

#endif