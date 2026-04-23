#include "raylib.h"

Music bgMusic;

void InitAudio() {
    InitAudioDevice();
    bgMusic = LoadMusicStream("assets/musica.mp3");
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.5f);
}

void UpdateAudio() {
    UpdateMusicStream(bgMusic);
}

void CloseAudio() {
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
}