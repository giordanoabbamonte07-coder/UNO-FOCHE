#include "graphics.h"
#include <stdio.h>
#include <math.h>

const char* nomi_col_gfx[] = {"rosso", "giallo", "verde", "blu", "nero"};
static Texture2D fallback;

Texture2D CreateFallback() {
    Image img = GenImageColor(60, 90, DARKGRAY);
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}

Texture2D LoadSafe(const char *path) {
    if (FileExists(path)) return LoadTexture(path);
    return fallback;
}

void DrawCarta(Texture2D t, int x, int y) {
    DrawTexturePro(t, (Rectangle){0,0,(float)t.width,(float)t.height},
                   (Rectangle){(float)x,(float)y,60,90}, (Vector2){0,0}, 0, WHITE);
}

Texture2D GetCarta(Graphics *gfx, Carta c) {
    if (c.tipo == NUMERO) return gfx->carte[c.colore][NUMERO][c.valore];
    return gfx->carte[c.colore][c.tipo][0];
}

void InitGraphics(Graphics *gfx) {
    fallback = CreateFallback();
    gfx->sfondo = LoadSafe("Carte/sfondo.png");
    gfx->foca   = LoadSafe("Carte/foca_popup.png");
    gfx->retro  = LoadSafe("Carte/back_card.png");

    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) {
            char path[128];
            sprintf(path, "Carte/%s_%d.png", nomi_col_gfx[c], v);
            gfx->carte[c][NUMERO][v] = LoadSafe(path);
        }
        char p[128];
        sprintf(p, "Carte/%s_stop.png", nomi_col_gfx[c]); gfx->carte[c][SKIP][0] = LoadSafe(p);
        sprintf(p, "Carte/%s_rev.png", nomi_col_gfx[c]);  gfx->carte[c][REVERSE][0] = LoadSafe(p);
        sprintf(p, "Carte/%s_p2.png", nomi_col_gfx[c]);   gfx->carte[c][PLUS2][0] = LoadSafe(p);
    }
    gfx->carte[NERO][CAMBIO_COLORE][0] = LoadSafe("Carte/wild.png");
    gfx->carte[NERO][PLUS4][0] = LoadSafe("Carte/wild_4.png");
}

void DrawGame(Graphics *gfx, Game *g) {
    BeginDrawing();
    ClearBackground(BLACK);

    // Sfondo
    DrawTexturePro(gfx->sfondo, (Rectangle){0,0,(float)gfx->sfondo.width,(float)gfx->sfondo.height},
                   (Rectangle){0,0,800,600}, (Vector2){0,0}, 0, WHITE);

    DrawCarta(gfx->retro, 300, 250);
    DrawCarta(GetCarta(gfx, g->cima), 420, 250);

    // Mano Player
    int startX = 400 - (g->player.num_carte * 35);
    for (int i = 0; i < g->player.num_carte; i++) {
        int posY = 450;
        if (g->turno == 0 && !g->scegliColore && PuoGiocare(g->player.mano[i], g)) {
            posY -= 20;
            DrawRectangle(startX + i * 70 - 2, posY - 2, 64, 94, Fade(YELLOW, 0.5f));
        }
        DrawCarta(GetCarta(gfx, g->player.mano[i]), startX + i * 70, posY);
    }

    // Mano CPU
    int cpuStart = 400 - (g->cpu.num_carte * 10);
    for (int i = 0; i < g->cpu.num_carte; i++) DrawCarta(gfx->retro, cpuStart + i * 20, 50);

    // Scelta Colore
    if (g->scegliColore) {
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));
        Color cols[] = {RED, YELLOW, GREEN, BLUE};
        for (int i = 0; i < 4; i++) {
            DrawRectangle(300 + i * 60, 250, 50, 50, cols[i]);
            DrawRectangleLines(300 + i * 60, 250, 50, 50, WHITE);
        }
        DrawText("SCEGLI IL COLORE!", 280, 210, 24, RAYWHITE);
    }

    // Messaggi della CPU
    if (g->msgTimer > 0) {
        int tw = MeasureText(g->messaggio, 25);
        DrawRectangle(400 - tw/2 - 15, 175, tw + 30, 40, Fade(BLACK, 0.8f));
        DrawText(g->messaggio, 400 - tw/2, 182, 25, GOLD);
    }

    // Animazione Foca
    if (g->animFoca) {
        float s = g->scegliColore ? 0.35f : 0.3f + 0.05f * sinf(GetTime() * 6);
        DrawTextureEx(gfx->foca, (Vector2){320, 80}, 0, s, WHITE);
    }

    // Schermata di Fine Partita
    if (g->game_over) {
        DrawRectangle(0,0,800,600, Fade(BLACK, 0.85f));
        if (g->player.num_carte == 0) {
            DrawText("VITTORIA FOCALIZZATA!", 120, 260, 48, GOLD);
        } else {
            DrawText("LA FOCA TI HA BATTUTO...", 140, 260, 40, RED);
        }
    }
    EndDrawing();
}

void UnloadGraphics(Graphics *gfx) {
    UnloadTexture(gfx->sfondo); UnloadTexture(gfx->foca); UnloadTexture(gfx->retro); UnloadTexture(fallback);
    for (int c=0; c<5; c++) for (int t=0; t<6; t++) for (int v=0; v<13; v++)
        if (gfx->carte[c][t][v].id > 0) UnloadTexture(gfx->carte[c][t][v]);
}