#include "audio.h"

static Music bg;
static Sound sCard, sWin, sDef;

void InitAudio(void) {
    InitAudioDevice();
    bg = LoadMusicStream("Musiche/musica.mp3");
    sCard = LoadSound("Musiche/suono_carta.mp3");
    sWin = LoadSound("Musiche/vittoria.mp3");
    sDef = LoadSound("Musiche/sconfitta.mp3");
    PlayMusicStream(bg);
}

void UpdateAudio(void) { UpdateMusicStream(bg); }
void StopBackgroundMusic(void) { StopMusicStream(bg); }
void PlayCardSound(void) { PlaySound(sCard); }
void PlayWinSound(void) { PlaySound(sWin); }
void PlayDefeatSound(void) { PlaySound(sDef); }
void CloseAudio(void) {
    UnloadMusicStream(bg); UnloadSound(sCard);
    UnloadSound(sWin); UnloadSound(sDef);
    CloseAudioDevice();
}