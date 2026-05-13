#include "graphics.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
    printf("ERRORE: Impossibile trovare l'asset in: %s\n", path);
    return fallback;
}

void DisegnaCarta(Texture2D t, float cx, float cy, float rotazione) {
    Rectangle source = {0, 0, (float)t.width, (float)t.height};
    Rectangle dest = {cx, cy, (float)CARTA_LARGHEZZA, (float)CARTA_ALTEZZA};
    Vector2 origin = {CARTA_LARGHEZZA/2.0f, CARTA_ALTEZZA/2.0f};
    DrawTexturePro(t, source, dest, origin, rotazione, WHITE);
}

void EvidenziaCarta(float cx, float cy, float rotazione, Color colore) {
    Rectangle rec = { cx, cy, CARTA_LARGHEZZA + 12, CARTA_ALTEZZA + 12 };
    Vector2 origin = {(CARTA_LARGHEZZA+12)/2.0f, (CARTA_ALTEZZA+12)/2.0f};
    DrawRectanglePro(rec, origin, rotazione, colore);
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
    gfx->sfondo = CaricaSicuro("Sfondo/sfondo_partita.png");
    gfx->foca = CaricaSicuro("Animazioni/foca_popup.png");
    gfx->deltaplano = CaricaSicuro("Animazioni/popup_deltaplano.png");
    gfx->win_img = CaricaSicuro("Animazioni/win.png");
    gfx->defeat_img = CaricaSicuro("Animazioni/defeat.png");
    gfx->retro = CaricaSicuro("Carte/back_card.png");

    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) {
            char p[128]; sprintf(p, "Carte/%s_%d.png", nomi_colore_gfx[c], v);
            gfx->carte[c][NUMERO][v] = CaricaSicuro(p);
        }
        char p1[128], p2[128], p3[128];
        sprintf(p1, "Carte/%s_stop.png", nomi_colore_gfx[c]); gfx->carte[c][SALTA][0] = CaricaSicuro(p1);
        sprintf(p2, "Carte/%s_rev.png", nomi_colore_gfx[c]); gfx->carte[c][INVERTI][0] = CaricaSicuro(p2);
        sprintf(p3, "Carte/%s_p2.png", nomi_colore_gfx[c]); gfx->carte[c][PIU_DUE][0] = CaricaSicuro(p3);
    }
    gfx->carte[NERO][CAMBIA_COLORE][0] = CaricaSicuro("Carte/wild.png");
    gfx->carte[NERO][PIU_QUATTRO][0] = CaricaSicuro("Carte/wild_4.png");
}

void DisegnaPartita(Grafica *gfx, Partita *p) {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(gfx->sfondo, (Rectangle){0, 0, (float)gfx->sfondo.width, (float)gfx->sfondo.height},
                  (Rectangle){0, 0, (float)SCHERMO_LARGHEZZA, (float)SCHERMO_ALTEZZA}, (Vector2){0,0}, 0, WHITE);

    float mazzoX = SCHERMO_LARGHEZZA/2.0f - 110.0f;
    float mazzoY = SCHERMO_ALTEZZA/2.0f;
    if (p->turno == 0 && !GiocatoreHaMosse(p) && !p->animando && !p->scegli_colore) {
        EvidenziaCarta(mazzoX, mazzoY, 0, GOLD);
    }
    DisegnaCarta(gfx->retro, mazzoX, mazzoY, 0);
    DisegnaCarta(OttieniCarta(gfx, p->cima), SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f, 0);

    int spacing = 65;
    int max_w = SCHERMO_LARGHEZZA - 350;
    if (p->giocatore.num_carte * spacing > max_w) spacing = max_w / p->giocatore.num_carte;
    int startX = SCHERMO_LARGHEZZA/2 - ((p->giocatore.num_carte - 1) * spacing) / 2;

    for (int i = 0; i < p->giocatore.num_carte; i++) {
        float dist = i - (p->giocatore.num_carte - 1) / 2.0f;
        float rot = dist * 5.0f;
        float py = SCHERMO_ALTEZZA - 100 + abs((int)dist) * 3;
        if (p->turno == 0 && !p->scegli_colore && PuoGiocare(p->giocatore.mano[i], p)) {
            py -= 25;
            EvidenziaCarta((float)startX + i*spacing, py, rot, GOLD);
        }
        DisegnaCarta(OttieniCarta(gfx, p->giocatore.mano[i]), (float)startX + i*spacing, py, rot);
    }

    int spB = 35;
    int botX = SCHERMO_LARGHEZZA/2 - ((p->bot.num_carte - 1) * spB) / 2;
    for (int i = 0; i < p->bot.num_carte; i++) {
        float dist = i - (p->bot.num_carte - 1) / 2.0f;
        DisegnaCarta(gfx->retro, (float)botX + i*spB, 90.0f + abs((int)dist) * 3, dist * 5.0f);
    }

    if (p->animando) {
        DisegnaCarta(OttieniCarta(gfx, p->carta_animata), p->posizione_animazione.x, p->posizione_animazione.y, p->timer_anim * 720.0f);
    }

    if (p->anima_deltaplano) {
        DrawTextureEx(gfx->deltaplano, (Vector2){p->pos_x_deltaplano, 150}, 0, 0.6f, WHITE);
    }

    if (p->timer_messaggio > 0) {
        int fSize = 50;
        int tw = MeasureText(p->messaggio, fSize);
        DisegnaTestoPixelGrassetto(p->messaggio, SCHERMO_LARGHEZZA/2 - tw/2, SCHERMO_ALTEZZA/2 - 160, fSize, YELLOW);
    }

    if (p->scegli_colore) {
        DrawRectangle(0, 0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, Fade(BLACK, 0.5f));
        Color cols[] = {RED, YELLOW, GREEN, BLUE};
        for (int i = 0; i < 4; i++) {
            DrawRectangle(SCHERMO_LARGHEZZA/2 - 160 + i*85, SCHERMO_ALTEZZA/2 - 40, 75, 75, cols[i]);
        }
        int tw = MeasureText("SCEGLI COLORE!", 40);
        DisegnaTestoPixelGrassetto("SCEGLI COLORE!", SCHERMO_LARGHEZZA/2 - tw/2, SCHERMO_ALTEZZA/2 - 100, 40, GOLD);
    }

    if (p->anima_foca) DrawTextureEx(gfx->foca, (Vector2){30, 100}, 0, 0.45f, WHITE);

    if (p->partita_finita) {
        DrawRectangle(0, 0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, Fade(BLACK, 0.85f));
        Texture2D finale = (p->giocatore.num_carte == 0) ? gfx->win_img : gfx->defeat_img;
        float scale = 0.8f;
        float fW = finale.width * scale;
        float fH = finale.height * scale;
        static float animY = 800.0f;
        float targetY = SCHERMO_ALTEZZA - fH;
        if (animY > targetY) animY -= 450.0f * GetFrameTime();
        DrawTextureEx(finale, (Vector2){SCHERMO_LARGHEZZA/2.0f - fW/2.0f, animY}, 0, scale, WHITE);
        const char* titolo = (p->giocatore.num_carte == 0) ? "VITTORIA FOCALIZZATA!" : "HAI PERSO CONTRO LA FOCA!";
        DisegnaTestoPixelGrassetto(titolo, SCHERMO_LARGHEZZA/2 - MeasureText(titolo, 60)/2, 100, 60, (p->giocatore.num_carte == 0) ? GOLD : RED);
    }

    EndDrawing();
}

void ScaricaGrafica(Grafica *gfx) {
    UnloadTexture(gfx->sfondo); UnloadTexture(gfx->foca); UnloadTexture(gfx->deltaplano);
    UnloadTexture(gfx->win_img); UnloadTexture(gfx->defeat_img); UnloadTexture(gfx->retro); UnloadTexture(fallback);
    for (int c = 0; c < 5; c++)
        for (int t = 0; t < 6; t++)
            for (int v = 0; v < 13; v++)
                if (gfx->carte[c][t][v].id > 0) UnloadTexture(gfx->carte[c][t][v]);
}