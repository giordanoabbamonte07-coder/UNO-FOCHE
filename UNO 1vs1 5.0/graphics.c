#include "graphics.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h> // <--- AGGIUNTA FONDAMENTALE PER RISOLVERE IL BUG

const char* nomi_colore_gfx[] = {"rosso", "giallo", "verde", "blu", "nero"};
static Texture2D fallback;

Texture2D CreaFallback(void) {
    Image img = GenImageColor(60, 90, DARKGRAY);
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}

Texture2D CaricaSicuro(const char *path) {
    if (FileExists(path)) return LoadTexture(path);
    return fallback;
}

void DisegnaCarta(Texture2D t, float cx, float cy, float rotazione) {
    DrawTexturePro(t, (Rectangle){0, 0, (float)t.width, (float)t.height},
                   (Rectangle){cx, cy, CARTA_LARGHEZZA, CARTA_ALTEZZA},
                   (Vector2){CARTA_LARGHEZZA/2.0f, CARTA_ALTEZZA/2.0f}, rotazione, WHITE);
}

void EvidenziaCarta(float cx, float cy, float rotazione, Color colore) {
    Rectangle rec = { cx, cy, CARTA_LARGHEZZA + 12, CARTA_ALTEZZA + 12 };
    DrawRectanglePro(rec, (Vector2){(CARTA_LARGHEZZA+12)/2.0f, (CARTA_ALTEZZA+12)/2.0f}, rotazione, colore);
}

void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor) {
    int thick = (fontSize > 40) ? 3 : 2;
    for(int i = -thick; i <= thick; i++) {
        for(int j = -thick; j <= thick; j++) {
            if (i != 0 || j != 0) DrawText(testo, x + i, y + j, fontSize, BLACK);
        }
    }
    DrawText(testo, x, y, fontSize, baseColor);
}

Texture2D OttieniCarta(Grafica *gfx, Carta c) {
    if (c.tipo == NUMERO) return gfx->carte[c.colore][NUMERO][c.valore];
    return gfx->carte[c.colore][c.tipo][0];
}

void InizializzaGrafica(Grafica *gfx) {
    fallback = CreaFallback();
    gfx->sfondo = CaricaSicuro("Carte/sfondo.png");
    gfx->foca = CaricaSicuro("Carte/foca_popup.png");
    gfx->foca_vittoria = CaricaSicuro("Carte/win.png");
    gfx->foca_sconfitta = CaricaSicuro("Carte/defeat.png");
    gfx->retro = CaricaSicuro("Carte/back_card.png");
    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) {
            char p[128]; sprintf(p, "Carte/%s_%d.png", nomi_colore_gfx[c], v);
            gfx->carte[c][NUMERO][v] = CaricaSicuro(p);
        }
        char p[128];
        sprintf(p, "Carte/%s_stop.png", nomi_colore_gfx[c]); gfx->carte[c][SALTA][0] = CaricaSicuro(p);
        sprintf(p, "Carte/%s_rev.png", nomi_colore_gfx[c]);  gfx->carte[c][INVERTI][0] = CaricaSicuro(p);
        sprintf(p, "Carte/%s_p2.png", nomi_colore_gfx[c]);   gfx->carte[c][PIU_DUE][0] = CaricaSicuro(p);
    }
    gfx->carte[NERO][CAMBIA_COLORE][0] = CaricaSicuro("Carte/wild.png");
    gfx->carte[NERO][PIU_QUATTRO][0] = CaricaSicuro("Carte/wild_4.png");
}

void DisegnaPartita(Grafica *gfx, Partita *p) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(gfx->sfondo, (Rectangle){0,0, (float)gfx->sfondo.width, (float)gfx->sfondo.height}, (Rectangle){0,0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA}, (Vector2){0,0}, 0, WHITE);

    DisegnaCarta(gfx->retro, SCHERMO_LARGHEZZA/2.0f - 110.0f, SCHERMO_ALTEZZA/2.0f, 0);
    DisegnaCarta(OttieniCarta(gfx, p->cima), SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f, 0);

    // Mano Giocatore
    int spacing = 65; int max_w = SCHERMO_LARGHEZZA - 350;
    if (p->giocatore.num_carte * spacing > max_w) spacing = max_w / p->giocatore.num_carte;
    int startX = SCHERMO_LARGHEZZA/2 - ((p->giocatore.num_carte - 1) * spacing) / 2;
    for (int i = 0; i < p->giocatore.num_carte; i++) {
        float dist = i - (p->giocatore.num_carte - 1) / 2.0f;
        float rot = dist * 5.0f;
        float py = SCHERMO_ALTEZZA - 100 + abs((int)dist) * 3; // abs() ora riconosciuto!
        if (p->turno == 0 && !p->scegli_colore && PuoGiocare(p->giocatore.mano[i], p)) {
            py -= 25; EvidenziaCarta(startX + i * spacing, py, rot, GOLD);
        }
        DisegnaCarta(OttieniCarta(gfx, p->giocatore.mano[i]), startX + i * spacing, py, rot);
    }

    // Mano BOT
    int spB = 35; int botX = SCHERMO_LARGHEZZA/2 - ((p->bot.num_carte - 1) * spB) / 2;
    for (int i = 0; i < p->bot.num_carte; i++) {
        float dist = i - (p->bot.num_carte - 1) / 2.0f;
        DisegnaCarta(gfx->retro, botX + i * spB, 90 - abs((int)dist)*3, dist * 5.0f);
    }

    if (p->animando) DisegnaCarta(OttieniCarta(gfx, p->carta_animata), p->posizione_animazione.x, p->posizione_animazione.y, p->timer_anim * 720.0f);

    // Messaggi centrali
    if (p->timer_messaggio > 0) {
        int fSize = 50; int tw = MeasureText(p->messaggio, fSize);
        DisegnaTestoPixelGrassetto(p->messaggio, SCHERMO_LARGHEZZA/2 - tw/2, SCHERMO_ALTEZZA/2 - 160, fSize, YELLOW);
    }

    if (p->scegli_colore) {
        DrawRectangle(0,0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, Fade(BLACK, 0.5f));
        Color cols[] = {RED, YELLOW, GREEN, BLUE};
        for (int i = 0; i < 4; i++) DrawRectangle(SCHERMO_LARGHEZZA/2 - 160 + i*85, SCHERMO_ALTEZZA/2 - 40, 75, 75, cols[i]);
        int tw = MeasureText("SCEGLI COLORE!", 40);
        DisegnaTestoPixelGrassetto("SCEGLI COLORE!", SCHERMO_LARGHEZZA/2 - tw/2, SCHERMO_ALTEZZA/2 - 100, 40, GOLD);
    }

    if (p->anima_foca) DrawTextureEx(gfx->foca, (Vector2){30, 100}, 0, 0.45f, WHITE);

    if (p->partita_finita) {
        DrawRectangle(0, 0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, Fade(BLACK, 0.85f));
        float fW = 750.0f; float fH = 500.0f;
        float fX = SCHERMO_LARGHEZZA/2 - fW/2;
        float fY = SCHERMO_ALTEZZA/2 - fH/2 + 60;

        if (p->giocatore.num_carte == 0) {
            DrawTexturePro(gfx->foca_vittoria, (Rectangle){0,0, (float)gfx->foca_vittoria.width, (float)gfx->foca_vittoria.height}, (Rectangle){fX, fY, fW, fH}, (Vector2){0,0}, 0, WHITE);
            int tw = MeasureText("VITTORIA FOCALIZZATA!", 65);
            DisegnaTestoPixelGrassetto("VITTORIA FOCALIZZATA!", SCHERMO_LARGHEZZA/2 - tw/2, 130, 65, GOLD);
        } else {
            DrawTexturePro(gfx->foca_sconfitta, (Rectangle){0,0, (float)gfx->foca_sconfitta.width, (float)gfx->foca_sconfitta.height}, (Rectangle){fX, fY, fW, fH}, (Vector2){0,0}, 0, WHITE);
            int tw = MeasureText("HAI PERSO CONTRO LA FOCA!", 65);
            DisegnaTestoPixelGrassetto("HAI PERSO CONTRO LA FOCA!", SCHERMO_LARGHEZZA/2 - tw/2, 130, 65, RED);
        }
    }
    EndDrawing();
}

void ScaricaGrafica(Grafica *gfx) {
    UnloadTexture(gfx->sfondo); UnloadTexture(gfx->foca); UnloadTexture(gfx->foca_vittoria);
    UnloadTexture(gfx->foca_sconfitta); UnloadTexture(gfx->retro); UnloadTexture(fallback);
    for (int c = 0; c < 5; c++) for (int t = 0; t < 6; t++) for (int v = 0; v < 13; v++) if (gfx->carte[c][t][v].id > 0) UnloadTexture(gfx->carte[c][t][v]);
}