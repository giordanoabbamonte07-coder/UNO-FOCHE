#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"

int main() {
    InitWindow(800, 600, "UNO FOCA - EDIZIONE DEFINITIVA");
    SetTargetFPS(60);

    // Inizializza l'audio subito dopo la finestra
    InitAudio();

    Game game;
    Graphics gfx;

    InitGame(&game);
    InitGraphics(&gfx);

    while (!WindowShouldClose()) {
        UpdateAudio(); // Deve essere chiamato ad ogni frame per lo streaming musicale

        HandleInput(&game);
        UpdateGame(&game);
        DrawGame(&gfx, &game);
    }

    UnloadGraphics(&gfx);
    CloseAudio(); // Chiudi la musica prima della finestra
    CloseWindow();

    return 0;
}