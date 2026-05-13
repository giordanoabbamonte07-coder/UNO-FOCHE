#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

typedef struct {
    Texture2D sfondo;
    Texture2D foca;
    Texture2D deltaplano;
    Texture2D win_img;
    Texture2D defeat_img;
    Texture2D retro;
    Texture2D carte[5][6][13];
} Grafica;

void InizializzaGrafica(Grafica *gfx);
void DisegnaPartita(Grafica *gfx, Partita *p);
void ScaricaGrafica(Grafica *gfx);
void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor);

#endif