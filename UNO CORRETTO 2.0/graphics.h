#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

typedef struct {
    Texture2D sfondo;
    Texture2D foca;
    Texture2D retro;
    Texture2D carte[5][6][13];
} Graphics;

void InitGraphics(Graphics *gfx);
void DrawGame(Graphics *gfx, Game *g);
void UnloadGraphics(Graphics *gfx);
Texture2D GetCarta(Graphics *gfx, Carta c);
void DrawCarta(Texture2D t, int x, int y);

#endif