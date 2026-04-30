#include "audio.h"
#include "raylib.h"

static Music bgMusic;

void InitAudio(void) {
    InitAudioDevice();
    bgMusic = LoadMusicStream("assets_musica.mp3");
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.4f);
}

void UpdateAudio(void) {
    UpdateMusicStream(bgMusic); // Necessario per far avanzare il buffer audio
}

void CloseAudio(void) {
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
}