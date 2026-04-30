#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"

int main(void) {
    // Inizializzazione finestra e impostazioni video
    InitWindow(800, 600, "UNO FOCA - EDIZIONE DEFINITIVA");
    SetTargetFPS(60);

    InitAudio(); // Carica musica e inizializza driver audio

    Game game;
    Graphics gfx;

    InitGame(&game);     // Setup logica di gioco e mazzo
    InitGraphics(&gfx);  // Caricamento texture e asset visivi

    // Loop principale di gioco
    while (!WindowShouldClose()) {
        UpdateAudio();   // Gestione streaming musicale
        HandleInput(&game); // Gestione click mouse e input utente
        UpdateGame(&game);  // Logica BOT e controllo regole
        DrawGame(&gfx, &game); // Rendering a video
    }

    // Pulizia risorse prima della chiusura
    UnloadGraphics(&gfx);
    CloseAudio();
    CloseWindow();

    return 0;
}