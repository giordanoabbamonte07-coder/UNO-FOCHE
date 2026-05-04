#include "audio.h"
#include "raylib.h"

static Music bgMusic;
static Sound cardSound;
static Sound winSound;
static Sound defeatSound;

void InitAudio(void) {
    InitAudioDevice();

    // Tutti i file ora vengono cercati esplicitamente dentro 'Musiche/'
    bgMusic = LoadMusicStream("Musiche/assets_musica.mp3");
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.4f);

    cardSound = LoadSound("Musiche/cardsound.mp3");
    SetSoundVolume(cardSound, 1.6f);

    winSound = LoadSound("Musiche/win.mp3");
    SetSoundVolume(winSound, 0.7f);

    defeatSound = LoadSound("Musiche/defeat.mp3");
    SetSoundVolume(defeatSound, 0.7f);
}

void UpdateAudio(void) {
    UpdateMusicStream(bgMusic);
}

void CloseAudio(void) {
    UnloadMusicStream(bgMusic);
    UnloadSound(cardSound);
    UnloadSound(winSound);
    UnloadSound(defeatSound);
    CloseAudioDevice();
}

void StopBackgroundMusic(void) {
    StopMusicStream(bgMusic);
}

void PlayCardSound(void) {
    PlaySound(cardSound);
}

void PlayWinSound(void) {
    PlaySound(winSound);
}

void PlayDefeatSound(void) {
    PlaySound(defeatSound);
}