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
    printf("ATTENZIONE: File mancante -> %s\n", path);
    return fallback;
}

void DisegnaCarta(Texture2D t, float cx, float cy, float rotazione) {
    Rectangle source = { 0, 0, (float)t.width, (float)t.height };
    Rectangle dest = { cx, cy, (float)CARTA_LARGHEZZA, (float)CARTA_ALTEZZA };
    Vector2 origin = { CARTA_LARGHEZZA / 2.0f, CARTA_ALTEZZA / 2.0f };
    DrawTexturePro(t, source, dest, origin, rotazione, WHITE);
}

void EvidenziaCarta(float cx, float cy, float rotazione, Color colore) {
    Rectangle rec = { cx, cy, CARTA_LARGHEZZA + 12, CARTA_ALTEZZA + 12 };
    Vector2 origin = { (CARTA_LARGHEZZA + 12) / 2.0f, (CARTA_ALTEZZA + 12) / 2.0f };
    DrawRectanglePro(rec, origin, rotazione, colore);
}

void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor) {
    int thick = (fontSize > 40) ? 3 : 2;
    for (int i = -thick; i <= thick; i++) {
        for (int j = -thick; j <= thick; j++) {
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
    gfx->btn_sospendi = CaricaSicuro("Pulsanti/sospendi.png");

    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) {
            char p[128];
            sprintf(p, "Carte/%s_%d.png", nomi_colore_gfx[c], v);
            gfx->carte[c][NUMERO][v] = CaricaSicuro(p);
        }
        char p1[128], p2[128], p3[128];
        sprintf(p1, "Carte/%s_stop.png", nomi_colore_gfx[c]);
        gfx->carte[c][SALTA][0] = CaricaSicuro(p1);
        sprintf(p2, "Carte/%s_rev.png", nomi_colore_gfx[c]);
        gfx->carte[c][INVERTI][0] = CaricaSicuro(p2);
        sprintf(p3, "Carte/%s_p2.png", nomi_colore_gfx[c]);
        gfx->carte[c][PIU_DUE][0] = CaricaSicuro(p3);
    }
    gfx->carte[NERO][CAMBIA_COLORE][0] = CaricaSicuro("Carte/wild.png");
    gfx->carte[NERO][PIU_QUATTRO][0] = CaricaSicuro("Carte/wild_4.png");
}

void DisegnaPartita(Grafica *gfx, Partita *p) {
    DrawTexturePro(gfx->sfondo, (Rectangle){0, 0, (float)gfx->sfondo.width, (float)gfx->sfondo.height},
                   (Rectangle){0, 0, 1200, 800}, (Vector2){0,0}, 0, WHITE);

    float mazzoX = 1200/2.0f - 110.0f;
    float mazzoY = 800/2.0f;

    if (p->turno == 0 && !GiocatoreHaMosse(p, 0) && !p->animando && !p->scegli_colore) {
        EvidenziaCarta(mazzoX, mazzoY, 0, GOLD);
    }

    DisegnaCarta(gfx->retro, mazzoX, mazzoY, 0);
    DisegnaCarta(OttieniCarta(gfx, p->cima), 1200/2.0f + 110.0f, 800 / 2.0f, 0);

    // ==========================================
    // GIOCATORE UMANO (In basso)
    // ==========================================
    int spacing = 65;
    int max_w = 1200 - 350;
    if (p->giocatori[0].num_carte * spacing > max_w) spacing = max_w / p->giocatori[0].num_carte;
    int startX = 600 - ((p->giocatori[0].num_carte - 1) * spacing) / 2;
    for (int i = 0; i < p->giocatori[0].num_carte; i++) {
        float dist = i - (p->giocatori[0].num_carte - 1)/2.0f;
        float rot = dist * 5.0f;
        float py = 800 - 100 + abs((int)dist) * 3;
        if (p->turno == 0 && !p->scegli_colore && PuoGiocare(p->giocatori[0].mano[i], p)) {
            py -= 25;
            EvidenziaCarta((float)startX + i*spacing, py, rot, GOLD);
        }
        DisegnaCarta(OttieniCarta(gfx, p->giocatori[0].mano[i]), (float)startX + i*spacing, py, rot);
    }

    // ==========================================
    // GESTIONE DEI BOT DINAMICA (1vs1 e 1vs3)
    // ==========================================
    int sp = 35;
    for (int b = 1; b < p->num_giocatori; b++) {
        Vector2 pos = OttieniPosizioneMazzoGiocatore(b, p->num_giocatori);
        int num_carte_bot = p->giocatori[b].num_carte;

        if (p->num_giocatori == 2) {
            int botX = (int)pos.x - ((num_carte_bot - 1) * sp) / 2;
            for (int i = 0; i < num_carte_bot; i++) {
                float dist = i - (num_carte_bot - 1)/2.0f;
                float py = pos.y - abs((int)dist) * 3;
                DisegnaCarta(gfx->retro, (float)botX + i*sp, py, -dist * 5.0f);
            }
        } else {
            if (b == 1) {
                int bot1Y = (int)pos.y - ((num_carte_bot - 1) * sp) / 2;
                for (int i = 0; i < num_carte_bot; i++) {
                    float dist = i - (num_carte_bot - 1)/2.0f;
                    DisegnaCarta(gfx->retro, pos.x - abs((int)dist)*3, (float)bot1Y + i*sp, 90.0f + dist * 5.0f);
                }
            } else if (b == 2) {
                int bot2X = (int)pos.x - ((num_carte_bot - 1) * sp) / 2;
                for (int i = 0; i < num_carte_bot; i++) {
                    float dist = i - (num_carte_bot - 1)/2.0f;
                    float py = pos.y - abs((int)dist) * 3;
                    DisegnaCarta(gfx->retro, (float)bot2X + i*sp, py, -dist * 5.0f);
                }
            } else if (b == 3) {
                int bot3Y = (int)pos.y - ((num_carte_bot - 1) * sp) / 2;
                for (int i = 0; i < num_carte_bot; i++) {
                    float dist = i - (num_carte_bot - 1)/2.0f;
                    DisegnaCarta(gfx->retro, pos.x + abs((int)dist)*3, (float)bot3Y + i*sp, -90.0f - dist * 5.0f);
                }
            }
        }
    }

    // ==========================================
    // LOGICA ANIMAZIONI E TESTI DI INTERFACCIA
    // ==========================================
    if (p->animando) {
        DisegnaCarta(OttieniCarta(gfx, p->carta_animata), p->posizione_animazione.x, p->posizione_animazione.y, p->timer_anim * 720.0f);
    }
    if (p->anima_deltaplano) {
        DrawTextureEx(gfx->deltaplano, (Vector2){p->pos_x_deltaplano, 150}, 0, 0.6f, WHITE);
    }

    if (p->timer_messaggio > 0) {
        DisegnaTestoPixelGrassetto(p->messaggio, 600 - MeasureText(p->messaggio, 50)/2, 220, 50, YELLOW);
    }

    // Finestra Selezione Colore
    if (p->scegli_colore) {
        DrawRectangle(0, 0, 1200, 800, Fade(BLACK, 0.5f));
        Color cols[] = {RED, YELLOW, GREEN, BLUE};
        for (int i=0; i<4; i++) {
            DrawRectangle(600-160 + i*85, 400-40, 75, 75, cols[i]);
        }

        // Spostata da 220 a 310 per abbassarla perfettamente sui quadratini
        DisegnaTestoPixelGrassetto("SCEGLI COLORE!", 600 - MeasureText("SCEGLI COLORE!", 40)/2, 310, 40, GOLD);

        if (p->anima_foca) {
            DrawTextureEx(gfx->foca, (Vector2){1200.0f/2 - (gfx->foca.width*0.45f)/2, 800.0f/2 - (gfx->foca.height*0.45f) - 120}, 0, 0.45f, WHITE);
        }
    }

    // GESTIONE BOTTONE SOSPENDI CON HOVER E CLICK SCOLORITO/SCURITO
    if (!p->is_guest && gfx->btn_sospendi.id > 0 && !p->partita_finita) {
        Vector2 mousePos = GetMousePosition();
        Rectangle rectSospendi = { 990, 700, 180, 70 };
        Color coloreTasto = WHITE;

        if (CheckCollisionPointRec(mousePos, rectSospendi)) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                coloreTasto = GRAY;            // Cliccato (Più scuro)
            } else {
                coloreTasto = Fade(WHITE, 0.6f); // Hover (Scolorito)
            }
        }

        DrawTexturePro(gfx->btn_sospendi, (Rectangle){ 0, 0, (float)gfx->btn_sospendi.width, (float)gfx->btn_sospendi.height },
                       rectSospendi, (Vector2){ 0, 0 }, 0, coloreTasto);
    }

    if (p->partita_finita) {
        DrawRectangle(0, 0, 1200, 800, Fade(BLACK, 0.85f));
        Texture2D finale = (p->vincitore == 0) ? gfx->win_img : gfx->defeat_img;
        static float animY = 800.0f;
        if (animY > 200.0f) {
            animY -= 5.0f;
        }
        DrawTextureEx(finale, (Vector2){600.0f - (finale.width*0.8f)/2, animY}, 0, 0.8f, WHITE);

        const char* t = (p->vincitore == 0) ? "VITTORIA FOCALIZZATA!" : "HAI PERSO CONTRO I BOT!";
        DisegnaTestoPixelGrassetto(t, 600 - MeasureText(t, 60)/2, 100, 60, (p->vincitore == 0) ? GOLD : RED);
    }
}

void ScaricaGrafica(Grafica *gfx) {
    UnloadTexture(gfx->sfondo);
    UnloadTexture(gfx->foca);
    UnloadTexture(gfx->deltaplano);
    UnloadTexture(gfx->win_img);
    UnloadTexture(gfx->defeat_img);
    UnloadTexture(gfx->retro);
    UnloadTexture(gfx->btn_sospendi);
    UnloadTexture(fallback);
    for (int c = 0; c < 5; c++)
        for (int t = 0; t < 6; t++)
            for (int v = 0; v < 13; v++)
                if (gfx->carte[c][t][v].id > 0) UnloadTexture(gfx->carte[c][t][v]);
}