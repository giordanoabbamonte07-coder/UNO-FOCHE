#include "raylib.h"
#include "game.h"
#include "graphics.h"

int main() {
    InitWindow(1200, 800, "UNO FOCHE - Final");
    SetTargetFPS(60);

    // AUDIO
    InitAudioDevice();
    Music music = LoadMusicStream("assets_musica/musica.mp3");
    PlayMusicStream(music);
    SetMusicVolume(music, 0.5f);

    // GAME
    Game g;
    InitGame(&g);

    // GRAFICA
    CaricaAsset();

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);

        UpdateGameLogic(&g);
        UpdateAndDraw(&g);
    }

    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}