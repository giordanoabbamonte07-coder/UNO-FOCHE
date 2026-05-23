#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

#define CARTA_LARGHEZZA 100
#define CARTA_ALTEZZA 150

typedef struct {
    Texture2D sfondo;
    Texture2D foca;
    Texture2D deltaplano;
    Texture2D win_img;
    Texture2D defeat_img;
    Texture2D retro;
    Texture2D carte[5][6][13];

    Texture2D btn_sospendi;
} Grafica;

Texture2D CaricaSicuro(const char *path);
void InizializzaGrafica(Grafica *gfx);
void DisegnaPartita(Grafica *gfx, Partita *p);
void ScaricaGrafica(Grafica *gfx);
void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor);

#endif