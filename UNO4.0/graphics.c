#include "graphics.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h> // Aggiunto per risolvere l'errore di abs()

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

// Disegna la carta dal centro, permettendo rotazione
void DisegnaCarta(Texture2D t, float cx, float cy, float rotazione) {
    DrawTexturePro(t, (Rectangle){0,0,(float)t.width,(float)t.height},
                   (Rectangle){cx, cy, CARTA_LARGHEZZA, CARTA_ALTEZZA}, 
                   (Vector2){CARTA_LARGHEZZA/2, CARTA_ALTEZZA/2}, // Origine impostata al centro della carta per ruotare bene
                   rotazione, WHITE);
}

Texture2D OttieniCarta(Grafica *gfx, Carta c) {
    if (c.tipo == NUMERO) return gfx->carte[c.colore][NUMERO][c.valore];
    return gfx->carte[c.colore][c.tipo][0];
}

void InizializzaGrafica(Grafica *gfx) {
    fallback = CreaFallback();
    gfx->sfondo = CaricaSicuro("Carte/sfondo.png");
    gfx->foca   = CaricaSicuro("Carte/foca_popup.png");
    gfx->foca_vittoria = CaricaSicuro("Carte/win.png");       // Carica la foca vittoriosa
    gfx->foca_sconfitta = CaricaSicuro("Carte/defeat.png"); // Carica la foca triste
    gfx->retro  = CaricaSicuro("Carte/back_card.png");

    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) {
            char path[128];
            sprintf(path, "Carte/%s_%d.png", nomi_colore_gfx[c], v);
            gfx->carte[c][NUMERO][v] = CaricaSicuro(path);
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

    DrawTexturePro(gfx->sfondo, (Rectangle){0,0,(float)gfx->sfondo.width,(float)gfx->sfondo.height},
                   (Rectangle){0,0,SCHERMO_LARGHEZZA,SCHERMO_ALTEZZA}, (Vector2){0,0}, 0, WHITE);

    // Coordinate passate sono il CENTRO della carta (x+50, y+75)
    DisegnaCarta(gfx->retro, SCHERMO_LARGHEZZA/2.0f - 110.0f, SCHERMO_ALTEZZA/2.0f, 0);
    DisegnaCarta(OttieniCarta(gfx, p->cima), SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f, 0);

    // --- DISEGNO MANO GIOCATORE CON OVERFLOW FIX & VENTAGLIO ---
    int max_width = SCHERMO_LARGHEZZA - 400; // Spazio massimo usabile a schermo per le carte
    int spacing = 65;    // Spazio standard tra le carte
    
    // Se le carte occupano troppo spazio, comprimiamo lo spacing! (Overflow Fix)
    if (p->giocatore.num_carte > 0 && p->giocatore.num_carte * spacing > max_width) {
        spacing = max_width / p->giocatore.num_carte; 
    }
    
    int startX = SCHERMO_LARGHEZZA/2 - ((p->giocatore.num_carte - 1) * spacing) / 2;

    for (int i = 0; i < p->giocatore.num_carte; i++) {
        float posY = SCHERMO_ALTEZZA - 100; // Centro Y della carta
        
        // Calcolo EFFETTO VENTAGLIO
        float middle_index = (p->giocatore.num_carte - 1) / 2.0f;
        float distFromCenter = i - middle_index;
        float rot = distFromCenter * 5.0f; // 5 gradi di inclinazione per ogni carta dal centro
        posY += abs((int)distFromCenter) * 3; // Le carte ai lati scendono un pochino

        if (p->turno == 0 && !p->scegli_colore && !p->animando && PuoGiocare(p->giocatore.mano[i], p)) {
            posY -= 20; // Alza le carte giocabili
        }
        DisegnaCarta(OttieniCarta(gfx, p->giocatore.mano[i]), startX + i * spacing, posY, rot);
    }

    // --- DISEGNO MANO CPU (Anche qui Overflow fix) ---
    int spacingBot = 35;
    int max_width_cpu = SCHERMO_LARGHEZZA - 400;
    if (p->cpu.num_carte > 0 && p->cpu.num_carte * spacingBot > max_width_cpu) {
        spacingBot = max_width_cpu / p->cpu.num_carte;
    }
    int cpuStart = SCHERMO_LARGHEZZA/2 - ((p->cpu.num_carte - 1) * spacingBot) / 2;
    for (int i = 0; i < p->cpu.num_carte; i++) {
        DisegnaCarta(gfx->retro, cpuStart + i * spacingBot, 20, 0);
    }

    // --- DISEGNO ANIMAZIONE CARTA IN VOLO ---
    if (p->animando) {
        // La carta ruota mentre vola per un effetto carino (anim_timer * 720)
        DisegnaCarta(OttieniCarta(gfx, p->carta_animata), p->posizione_animazione.x, p->posizione_animazione.y, p->timer_anim * 720.0f);
    }

    if (p->scegli_colore) {
        DrawRectangle(0, 0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, Fade(BLACK, 0.5f));
        Color cols[] = {RED, YELLOW, GREEN, BLUE};
        for (int i = 0; i < 4; i++) {
            DrawRectangle(SCHERMO_LARGHEZZA/2 - 160 + i * 85, SCHERMO_ALTEZZA/2 - 40, 75, 75, cols[i]);
            DrawRectangleLines(SCHERMO_LARGHEZZA/2 - 160 + i * 85, SCHERMO_ALTEZZA/2 - 40, 75, 75, WHITE);
        }
        DrawText("SCEGLI IL COLORE!", SCHERMO_LARGHEZZA/2 - MeasureText("SCEGLI IL COLORE!", 24)/2, SCHERMO_ALTEZZA/2 - 80, 24, RAYWHITE);
    }

    if (p->timer_messaggio > 0) {
        int tw = MeasureText(p->messaggio, 25);
        DrawRectangle(SCHERMO_LARGHEZZA/2 - tw / 2 - 15, 175, tw + 30, 40, Fade(BLACK, 0.8f));
        DrawText(p->messaggio, SCHERMO_LARGHEZZA/2 - tw / 2, 182, 25, GOLD);
    }

    if (p->anima_foca) {
        float s = p->scegli_colore ? 0.35f : 0.3f + 0.05f * sinf(GetTime() * 6);
        DrawTextureEx(gfx->foca, (Vector2){20, 80}, 0, s, WHITE);
    }

    // Variabile statica per tenere traccia del tempo passato dalla fine della partita
    static float endgameAnimTimer = 0.0f;
    
    if (!p->partita_finita) {
        endgameAnimTimer = 0.0f; // Resetta se stiamo giocando
    } else {
        endgameAnimTimer += GetFrameTime();
        if (endgameAnimTimer > 1.0f) endgameAnimTimer = 1.0f; 
        
        DrawRectangle(0,0,SCHERMO_LARGHEZZA,SCHERMO_ALTEZZA, Fade(BLACK, 0.85f));
        
        // Ho preso le misure originali 1380x752 e le ho scalate mantenendo le proporzioni
        float larghezzaFoca = 550.0f; 
        float altezzaFoca = 300.0f;   

        // La Y parte da 600 (fuori schermo) e sale.
        // La X calcola la metà schermo meno metà della larghezza foca, per centrarla
        float currentY = SCHERMO_ALTEZZA - (altezzaFoca * endgameAnimTimer);
        float currentX = SCHERMO_LARGHEZZA/2.0f - (larghezzaFoca / 2.0f); 
        
        // Rettangolo di destinazione dove verrà incollata l'immagine
        Rectangle destRect = {currentX, currentY, larghezzaFoca, altezzaFoca};

        if (p->giocatore.num_carte == 0) {
            // Il rettangolo sorgente è la texture originale per intero
            Rectangle sourceRect = {0, 0, (float)gfx->foca_vittoria.width, (float)gfx->foca_vittoria.height};
            DrawTexturePro(gfx->foca_vittoria, sourceRect, destRect, (Vector2){0,0}, 0, WHITE);
                   
            DrawText("VITTORIA FOCALIZZATA!", 180, 180, 48, GOLD);
        } else {
            // Logica identica per il game over
            Rectangle sourceRect = {0, 0, (float)gfx->foca_sconfitta.width, (float)gfx->foca_sconfitta.height};
            DrawTexturePro(gfx->foca_sconfitta, sourceRect, destRect, (Vector2){0,0}, 0, WHITE);
                   
            DrawText("LA FOCA TI HA BATTUTO...", 210, 180, 40, RED);
        }
    }
    EndDrawing();
}

void ScaricaGrafica(Grafica *gfx) {
    UnloadTexture(gfx->sfondo);
    UnloadTexture(gfx->foca);
    UnloadTexture(gfx->foca_vittoria);     // Libera foca_vittoria
    UnloadTexture(gfx->foca_sconfitta);  // Libera foca_sconfitta
    UnloadTexture(gfx->retro);
    UnloadTexture(fallback);
    for (int c = 0; c < 5; c++) {
        for (int t = 0; t < 6; t++) {
            for (int v = 0; v < 13; v++) {
                if (gfx->carte[c][t][v].id > 0) UnloadTexture(gfx->carte[c][t][v]);
            }
        }
    }
}