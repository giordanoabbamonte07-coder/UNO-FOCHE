#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MANO_MAX 50
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define CARD_WIDTH 100
#define CARD_HEIGHT 150

// Coordinate fisse per il tavolo
#define MAZZO_X (SCREEN_WIDTH/2.0f - 110.0f)
#define MAZZO_Y (SCREEN_HEIGHT/2.0f)
#define SCARTI_X (SCREEN_WIDTH/2.0f + 110.0f)
#define SCARTI_Y (SCREEN_HEIGHT/2.0f)

// ================= STRUTTURE =================
typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SKIP, REVERSE, PLUS2, PLUS4, CAMBIO_COLORE } Tipo;
typedef enum { STATUS_PLAYING, STATUS_SELECTING_COLOR, STATUS_GAME_OVER } GameStatus;

typedef struct {
    Colore colore; Tipo tipo; int valore;
    Vector2 pos; float rot; float targetRot;
} Carta;

typedef struct { Carta mano[MANO_MAX]; int num_carte; } Giocatore;

typedef struct {
    Carta mazzo[108]; int indice_mazzo;
    Carta cima; Carta vecchiaCima; 
    Giocatore player, cpu;
    Colore colore_attuale;
    int turno; GameStatus status;
    float cpuTimer;
    char msg[32]; float msgTimer;
    float focaTimer; float focaY;
    float shakeTimer; // Per l'effetto scossa sul +4
} Game;

Texture2D cardTextures[5][15]; 
Texture2D cardBack, sfondoTex, focaTex;
Music bgMusic;

// ================= CARICAMENTO =================
void CaricaAsset() {
    const char* col_nomi[] = {"rosso", "giallo", "verde", "blu", "nero"};
    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) cardTextures[c][v] = LoadTexture(TextFormat("assets/%s_%d.png", col_nomi[c], v));
        cardTextures[c][10] = LoadTexture(TextFormat("assets/%s_stop.png", col_nomi[c]));
        cardTextures[c][11] = LoadTexture(TextFormat("assets/%s_rev.png", col_nomi[c]));
        cardTextures[c][12] = LoadTexture(TextFormat("assets/%s_p2.png", col_nomi[c]));
    }
    cardTextures[NERO][13] = LoadTexture("assets/wild.png");
    cardTextures[NERO][14] = LoadTexture("assets/wild_4.png");
    cardBack = LoadTexture("assets/back_card.png");
    sfondoTex = LoadTexture("assets/sfondo.png");
    focaTex = LoadTexture("assets/foca_popup.png");
}

Texture2D GetTex(Carta c) {
    if (c.colore == NERO) return (c.tipo == PLUS4) ? cardTextures[NERO][14] : cardTextures[NERO][13];
    int idx = (c.tipo == NUMERO) ? c.valore : (c.tipo == SKIP ? 10 : (c.tipo == REVERSE ? 11 : 12));
    return cardTextures[c.colore][idx];
}

// ================= LOGICA CORE =================
void PreparaMazzo(Game *g) {
    int i = 0;
    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) g->mazzo[i++] = (Carta){c, NUMERO, v, {MAZZO_X, MAZZO_Y}, 0};
        for (int k = 0; k < 2; k++) {
            g->mazzo[i++] = (Carta){c, SKIP, 0, {MAZZO_X, MAZZO_Y}, 0};
            g->mazzo[i++] = (Carta){c, REVERSE, 0, {MAZZO_X, MAZZO_Y}, 0};
            g->mazzo[i++] = (Carta){c, PLUS2, 0, {MAZZO_X, MAZZO_Y}, 0};
        }
    }
    for (int j = 0; j < 4; j++) {
        g->mazzo[i++] = (Carta){NERO, PLUS4, 0, {MAZZO_X, MAZZO_Y}, 0};
        g->mazzo[i++] = (Carta){NERO, CAMBIO_COLORE, 0, {MAZZO_X, MAZZO_Y}, 0};
    }
    g->indice_mazzo = i;
    for (int j = 0; j < i; j++) {
        int r = GetRandomValue(0, i - 1);
        Carta tmp = g->mazzo[j]; g->mazzo[j] = g->mazzo[r]; g->mazzo[r] = tmp;
    }
}

Carta Pesca(Game *g) {
    if (g->indice_mazzo <= 1) PreparaMazzo(g);
    Carta c = g->mazzo[--g->indice_mazzo];
    c.pos = (Vector2){MAZZO_X, MAZZO_Y}; 
    return c;
}

bool PuoGiocare(Carta c, Game *g) {
    if (c.colore == NERO) return true;
    if (c.colore == g->colore_attuale) return true;
    if (c.tipo == g->cima.tipo && c.tipo != NUMERO) return true;
    if (c.tipo == NUMERO && g->cima.tipo == NUMERO && c.valore == g->cima.valore) return true;
    return false;
}

void ApplicaEffetto(Game *g, Carta c) {
    bool ancoraIo = false;
    if (c.tipo == SKIP || c.tipo == REVERSE) { sprintf(g->msg, "SKIP!"); ancoraIo = true; }
    else if (c.tipo == PLUS2) {
        Giocatore *opp = (g->turno == 0) ? &g->cpu : &g->player;
        for(int i=0; i<2; i++) opp->mano[opp->num_carte++] = Pesca(g);
        sprintf(g->msg, "+2!"); ancoraIo = true;
    } else if (c.tipo == PLUS4) {
        Giocatore *opp = (g->turno == 0) ? &g->cpu : &g->player;
        for(int i=0; i<4; i++) opp->mano[opp->num_carte++] = Pesca(g);
        sprintf(g->msg, "+4!"); ancoraIo = true;
        g->focaTimer = 2.5f; g->shakeTimer = 0.4f;
    } else if (c.tipo == CAMBIO_COLORE) {
        sprintf(g->msg, "JOLLY!"); g->focaTimer = 1.5f;
    }

    g->msgTimer = 1.3f;
    if (c.colore == NERO) {
        g->status = STATUS_SELECTING_COLOR;
        if (g->turno == 1) { 
            g->colore_attuale = GetRandomValue(0, 3);
            g->status = STATUS_PLAYING;
            if (!ancoraIo) g->turno = 0;
        }
    } else { if (!ancoraIo) g->turno = (g->turno + 1) % 2; }
}

// ================= DISEGNO E ANIMAZIONI =================
void DrawCard(Carta *c, Vector2 target, float targetRot, float scale, Color tint, float dt) {
    c->pos.x += (target.x - c->pos.x) * 10.0f * dt;
    c->pos.y += (target.y - c->pos.y) * 10.0f * dt;
    c->rot += (targetRot - c->rot) * 8.0f * dt;
    Texture2D tex = GetTex(*c);
    Vector2 origin = { (CARD_WIDTH * scale)/2, (CARD_HEIGHT * scale)/2 };
    DrawTexturePro(tex, (Rectangle){0,0,tex.width, tex.height}, (Rectangle){c->pos.x+4, c->pos.y+4, CARD_WIDTH*scale, CARD_HEIGHT*scale}, origin, c->rot, Fade(BLACK, 0.3f));
    DrawTexturePro(tex, (Rectangle){0,0,tex.width, tex.height}, (Rectangle){c->pos.x, c->pos.y, CARD_WIDTH*scale, CARD_HEIGHT*scale}, origin, c->rot, tint);
}

void DrawCardRetro(Vector2 pos, float rot, float scale, float dt) {
    Vector2 origin = { (CARD_WIDTH * scale)/2, (CARD_HEIGHT * scale)/2 };
    DrawTexturePro(cardBack, (Rectangle){0,0,cardBack.width, cardBack.height}, (Rectangle){pos.x, pos.y, CARD_WIDTH*scale, CARD_HEIGHT*scale}, origin, rot, WHITE);
}

void UpdateAndDraw(Game *g) {
    float dt = GetFrameTime();
    if (IsAudioDeviceReady()) UpdateMusicStream(bgMusic);
    if (g->msgTimer > 0) g->msgTimer -= dt;
    if (g->focaTimer > 0) g->focaTimer -= dt;
    if (g->shakeTimer > 0) g->shakeTimer -= dt;

    BeginDrawing();
    ClearBackground(BLACK);
    
    // Shake screen effect
    if (g->shakeTimer > 0) {
        float rX = GetRandomValue(-10, 10);
        float rY = GetRandomValue(-10, 10);
        BeginMode2D((Camera2D){.offset = (Vector2){rX, rY}, .zoom = 1.0f});
    }

    // 1. SFONDO
    if (sfondoTex.id > 0) DrawTexturePro(sfondoTex, (Rectangle){0,0,sfondoTex.width, sfondoTex.height}, (Rectangle){0,0,SCREEN_WIDTH,SCREEN_HEIGHT}, (Vector2){0,0}, 0, WHITE);
    else DrawCircleGradient(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 800, ColorFromHSV(140, 0.7, 0.2), BLACK);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.3f));

    // 2. TAVOLO
    DrawCardRetro((Vector2){MAZZO_X, MAZZO_Y}, 0, 1.0f, dt);
    DrawCard(&g->vecchiaCima, (Vector2){SCARTI_X, SCARTI_Y}, g->vecchiaCima.rot, 1.0f, WHITE, dt);
    DrawCard(&g->cima, (Vector2){SCARTI_X, SCARTI_Y}, (sinf(GetTime()*2)*3), 1.0f, WHITE, dt); 
    Color cVis = (g->colore_attuale == ROSSO) ? RED : (g->colore_attuale == GIALLO) ? YELLOW : (g->colore_attuale == VERDE) ? LIME : BLUE;
    DrawRectangleLinesEx((Rectangle){SCARTI_X-55, SCARTI_Y-80, 110, 160}, 5, cVis);

    // 3. MANO CPU
    float spCPU = fminf(35, (SCREEN_WIDTH - 400) / (float)(g->cpu.num_carte > 0 ? g->cpu.num_carte : 1));
    float stCPU = SCREEN_WIDTH/2 - ((g->cpu.num_carte-1)*spCPU)/2;
    for(int i=0; i<g->cpu.num_carte; i++) {
        g->cpu.mano[i].pos.x += (stCPU + i*spCPU - g->cpu.mano[i].pos.x) * 8.0f * dt;
        g->cpu.mano[i].pos.y += (20 - g->cpu.mano[i].pos.y) * 8.0f * dt;
        DrawCardRetro(g->cpu.mano[i].pos, 180 + (i - g->cpu.num_carte/2.0f)*2.0f, 0.75f, dt);
    }

    // 4. MANO PLAYER
    float sp = fminf(65, (SCREEN_WIDTH-300)/(float)(g->player.num_carte>0?g->player.num_carte:1));
    float st = SCREEN_WIDTH/2 - ((g->player.num_carte-1)*sp)/2;
    int hov = -1;
    for(int i=g->player.num_carte-1; i>=0; i--) {
        if(CheckCollisionPointRec(GetMousePosition(), (Rectangle){st+i*sp-45, SCREEN_HEIGHT-170, 90, 140}) && g->status == STATUS_PLAYING) { hov=i; break; }
    }
    for(int i=0; i<g->player.num_carte; i++) {
        Vector2 t = { st + i*sp, SCREEN_HEIGHT-100 };
        float r = (i - g->player.num_carte/2.0f)*3.0f;
        float s = 1.0f;
        if(i == hov && g->turno == 0) { t.y -= 45; r = 0; s = 1.15f; }
        Color tint = (g->turno == 0 && PuoGiocare(g->player.mano[i], g)) ? WHITE : GRAY;
        DrawCard(&g->player.mano[i], t, r, s, tint, dt);
        if(i == hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && g->turno == 0 && g->status == STATUS_PLAYING && PuoGiocare(g->player.mano[i], g)) {
            g->vecchiaCima = g->cima; g->cima = g->player.mano[i];
            if(g->cima.colore != NERO) g->colore_attuale = g->cima.colore;
            for(int k=i; k<g->player.num_carte-1; k++) g->player.mano[k] = g->player.mano[k+1];
            g->player.num_carte--; ApplicaEffetto(g, g->cima);
            if(g->player.num_carte == 0) g->status = STATUS_GAME_OVER;
        }
    }

    // 5. SELEZIONE COLORE (LAYER SOPRA CARTE)
    if (g->status == STATUS_SELECTING_COLOR) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, Fade(BLACK, 0.6f));
        Colore cols[] = {ROSSO, GIALLO, VERDE, BLU}; Color vCols[] = {RED, GOLD, LIME, BLUE};
        for(int i=0; i<4; i++) {
            Rectangle b = { SCREEN_WIDTH/2 - 160 + i*85, SCREEN_HEIGHT/2 - 40, 75, 75 };
            DrawRectangleRec(b, CheckCollisionPointRec(GetMousePosition(), b) ? ColorBrightness(vCols[i], 0.2f) : vCols[i]);
            DrawRectangleLinesEx(b, 3, WHITE);
            if(CheckCollisionPointRec(GetMousePosition(), b) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                g->colore_attuale = cols[i]; g->status = STATUS_PLAYING;
                if(g->cima.tipo == CAMBIO_COLORE) g->turno = (g->turno + 1) % 2;
            }
        }
    }

    // 6. FOCA POPUP (LAYER SOPRA TUTTO!)
    if (focaTex.id > 0) {
        float tarY = (g->focaTimer > 0) ? SCREEN_HEIGHT - focaTex.height : SCREEN_HEIGHT + 200;
        g->focaY += (tarY - g->focaY) * 12.0f * dt;
        DrawTextureEx(focaTex, (Vector2){20, g->focaY}, 0, 1.2f, WHITE);
    }

    // 7. MESSAGGI E UI
    if (g->msgTimer > 0) DrawText(g->msg, SCREEN_WIDTH/2 - MeasureText(g->msg, 60)/2, SCREEN_HEIGHT/2 - 220, 60, GOLD);
    if (g->turno == 0 && g->status == STATUS_PLAYING) DrawText("TUO TURNO", 30, SCREEN_HEIGHT - 40, 25, YELLOW);
    
    // Pesca
    if(g->turno == 0 && g->status == STATUS_PLAYING && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){MAZZO_X-50, MAZZO_Y-75, 100, 150})) {
        g->player.mano[g->player.num_carte++] = Pesca(g); g->turno = 1;
    }

    // CPU
    if (g->turno == 1 && g->status == STATUS_PLAYING) {
        g->cpuTimer += dt;
        if(g->cpuTimer > 1.4f) {
            int m = -1; for(int i=0; i<g->cpu.num_carte; i++) if(PuoGiocare(g->cpu.mano[i], g)) { m=i; break; }
            if(m != -1) {
                g->vecchiaCima = g->cima; g->cima = g->cpu.mano[m];
                if(g->cima.colore != NERO) g->colore_attuale = g->cima.colore;
                for(int k=m; k<g->cpu.num_carte-1; k++) g->cpu.mano[k] = g->cpu.mano[k+1];
                g->cpu.num_carte--; ApplicaEffetto(g, g->cima);
                if(g->cpu.num_carte == 0) g->status = STATUS_GAME_OVER;
            } else { g->cpu.mano[g->cpu.num_carte++] = Pesca(g); g->turno = 0; }
            g->cpuTimer = 0;
        }
    }

    if (g->shakeTimer > 0) EndMode2D();

    if (g->status == STATUS_GAME_OVER) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, Fade(BLACK, 0.85f));
        const char* v = (g->player.num_carte == 0) ? "VITTORIA FOCALIZZATA!" : "CPU TRIONFANTE!";
        DrawText(v, SCREEN_WIDTH/2-MeasureText(v, 50)/2, SCREEN_HEIGHT/2-30, 50, GOLD);
        DrawText("Premi R per la rivincita", SCREEN_WIDTH/2-140, SCREEN_HEIGHT/2+60, 25, WHITE);
        if(IsKeyPressed(KEY_R)) {
            g->player.num_carte = g->cpu.num_carte = 0; PreparaMazzo(g);
            for(int i=0; i<7; i++) { g->player.mano[g->player.num_carte++] = Pesca(g); g->cpu.mano[g->cpu.num_carte++] = Pesca(g); }
            g->vecchiaCima = (Carta){0}; g->cima = Pesca(g); g->cima.pos = (Vector2){SCARTI_X, SCARTI_Y};
            g->colore_attuale = (g->cima.colore == NERO) ? ROSSO : g->cima.colore;
            g->status = STATUS_PLAYING; g->turno = 0;
        }
    }
    EndDrawing();
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT); InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "UNO FOCHE - Ultra Edition");
    InitAudioDevice(); SetTargetFPS(60); CaricaAsset();
    bgMusic = LoadMusicStream("assets/musica.mp3");
    if(bgMusic.stream.buffer != NULL) { PlayMusicStream(bgMusic); SetMusicVolume(bgMusic, 0.5f); }
    srand(time(NULL));
    Game g = { .status = STATUS_PLAYING, .focaY = SCREEN_HEIGHT + 200 };
    PreparaMazzo(&g);
    for(int i=0; i<7; i++) { g.player.mano[g.player.num_carte++] = Pesca(&g); g.cpu.mano[g.cpu.num_carte++] = Pesca(&g); }
    g.cima = Pesca(&g); g.cima.pos = (Vector2){SCARTI_X, SCARTI_Y};
    g.colore_attuale = (g.cima.colore == NERO) ? ROSSO : g.cima.colore;
    while (!WindowShouldClose()) UpdateAndDraw(&g);
    UnloadMusicStream(bgMusic); CloseAudioDevice(); CloseWindow();
    return 0;
}
