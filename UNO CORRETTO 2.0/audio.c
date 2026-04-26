#include "audio.h"
#include "raylib.h"

static Music bgMusic;

void InitAudio(void) {
    InitAudioDevice();
    // Carica il file musicale indicato
    bgMusic = LoadMusicStream("assets_musica.mp3");
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.4f); // Volume piacevole di sottofondo
}

void UpdateAudio(void) {
    UpdateMusicStream(bgMusic);
}

void CloseAudio(void) {
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
}