#include "graphics.h"
#include <math.h>

// ================= TEXTURE =================
Texture2D cardTextures[4][10];
Texture2D cardBack, sfondoTex;

// ================= COSTANTI TAVOLO =================
#define MAZZO_X 500
#define MAZZO_Y 300

#define SCARTI_X 700
#define SCARTI_Y 300

// ================= CARICAMENTO =================
void CaricaAsset() {
    const char* nomi[] = {"rosso", "giallo", "verde", "blu"};

    for(int c = 0; c < 4; c++) {
        for(int v = 0; v < 10; v++) {
            cardTextures[c][v] = LoadTexture(
                TextFormat("Carte/%s_%d.png", nomi[c], v)
            );
        }
    }

    cardBack = LoadTexture("Carte/back_card.png");
    sfondoTex = LoadTexture("Carte/sfondo.png");
}

// ================= TEXTURE CARTA =================
Texture2D GetCardTexture(Carta c) {
    if (c.colore >= 0 && c.colore < 4 && c.valore >= 0 && c.valore < 10)
        return cardTextures[c.colore][c.valore];

    return cardBack;
}

// ================= ANIMAZIONE =================
void DrawCardAnim(Carta *c, Texture2D tex, Vector2 target, float rot, float scale, float dt) {
    c->pos.x += (target.x - c->pos.x) * 10.0f * dt;
    c->pos.y += (target.y - c->pos.y) * 10.0f * dt;
    c->rot += (rot - c->rot) * 8.0f * dt;

    Vector2 origin = {50 * scale, 75 * scale};

    DrawTexturePro(
        tex,
        (Rectangle){0,0,tex.width, tex.height},
        (Rectangle){c->pos.x, c->pos.y, 100*scale, 150*scale},
        origin,
        c->rot,
        WHITE
    );
}

// ================= CLICK CARTA =================
bool CheckCardClick(Rectangle rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect) &&
           IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ================= UPDATE + DRAW =================
void UpdateAndDraw(Game *g) {
    float dt = GetFrameTime();

    BeginDrawing();
    ClearBackground(BLACK);

    // ===== SFONDO =====
    if (sfondoTex.id > 0) {
        DrawTexturePro(
            sfondoTex,
            (Rectangle){0,0,sfondoTex.width,sfondoTex.height},
            (Rectangle){0,0,1200,800},
            (Vector2){0,0},
            0,
            WHITE
        );
    }

    DrawRectangle(0,0,1200,800, Fade(BLACK, 0.3f));

    DrawText("UNO FOCHE", 480, 20, 30, WHITE);

    // ===== MAZZO (SINISTRA) =====
    DrawTexturePro(
        cardBack,
        (Rectangle){0,0,cardBack.width, cardBack.height},
        (Rectangle){MAZZO_X, MAZZO_Y, 100, 150},
        (Vector2){50,75},
        0,
        WHITE
    );

    // ===== SCARTI (DESTRA) =====
    Texture2D centerTex = GetCardTexture(g->cima);

    DrawTexturePro(
        centerTex,
        (Rectangle){0,0,centerTex.width, centerTex.height},
        (Rectangle){SCARTI_X, SCARTI_Y, 100, 150},
        (Vector2){50,75},
        sinf(GetTime()*2)*5,
        WHITE
    );

    // ===== CPU =====
    float spacingCPU = fminf(40, (1200 - 300) / (float)(g->cpu.num_carte > 0 ? g->cpu.num_carte : 1));
    float startCPU = 600 - ((g->cpu.num_carte-1)*spacingCPU)/2;

    for(int i=0; i<g->cpu.num_carte; i++) {
        Vector2 target = { startCPU + i*spacingCPU, 80 };
        DrawCardAnim(&g->cpu.mano[i], cardBack, target, 180, 0.7f, dt);
    }

    // ===== PLAYER =====
    float spacing = fminf(70, (1200 - 200) / (float)(g->player.num_carte > 0 ? g->player.num_carte : 1));
    float start = 600 - ((g->player.num_carte-1)*spacing)/2;

    for(int i=0; i<g->player.num_carte; i++) {
        Vector2 target = { start + i*spacing, 700 };
        Texture2D tex = GetCardTexture(g->player.mano[i]);

        DrawCardAnim(&g->player.mano[i], tex,
                     target,
                     (i - g->player.num_carte/2.0f)*5,
                     1.0f,
                     dt);

        // ===== CLICK =====
        Rectangle rect = { target.x-50, target.y-75, 100, 150 };

        if (CheckCardClick(rect) && g->turno == 0) {
            if (PuoGiocare(g->player.mano[i], g)) {

                // aggiorna carta centrale
                g->cima = g->player.mano[i];
                g->colore_attuale = g->cima.colore;

                // rimuovi carta dalla mano
                for (int k = i; k < g->player.num_carte - 1; k++)
                    g->player.mano[k] = g->player.mano[k + 1];

                g->player.num_carte--;

                // effetto
                ApplicaEffetto(g, g->cima);
            }
        }
    }

    // ===== CLICK MAZZO (PESCA) =====
    Rectangle mazzoRect = {MAZZO_X-50, MAZZO_Y-75, 100, 150};

    if (CheckCardClick(mazzoRect) && g->turno == 0) {
        g->player.mano[g->player.num_carte++] = Pesca(g);
        g->turno = 1;
    }

    EndDrawing();
}