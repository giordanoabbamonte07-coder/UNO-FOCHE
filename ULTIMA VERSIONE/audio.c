#include "audio.h"

static Music bg;
static Sound sCard, sWin, sDef;

// Flag di stato locali per la gestione nativa del muto
static bool musicaAbilitata = true;
static bool effettiAbilitati = true;

void InitAudio(void) {
    InitAudioDevice();
    bg = LoadMusicStream("Musiche/musica.mp3");
    sCard = LoadSound("Musiche/suono_carta.mp3");
    sWin = LoadSound("Musiche/vittoria.mp3");
    sDef = LoadSound("Musiche/sconfitta.mp3");

    musicaAbilitata = true;
    effettiAbilitati = true;
}

void PlayBackgroundMusic(void) {
    if (musicaAbilitata && !IsMusicStreamPlaying(bg)) {
        PlayMusicStream(bg);
    }
}

void UpdateAudio(void) {
    if (musicaAbilitata) {
        UpdateMusicStream(bg);
    }
}

void StopBackgroundMusic(void) {
    StopMusicStream(bg);
}

void PlayCardSound(void) {
    if (effettiAbilitati) PlaySound(sCard);
}

void PlayWinSound(void) {
    if (effettiAbilitati) PlaySound(sWin);
}

void PlayDefeatSound(void) {
    if (effettiAbilitati) PlaySound(sDef);
}

void ImpostaStatoMusica(bool attiva) {
    musicaAbilitata = attiva;
    if (!musicaAbilitata) {
        StopBackgroundMusic(); // Se disattivata, la interrompiamo subito
    }
    // Rimosso PlayBackgroundMusic da qui: la musica NON deve partire nei menu!
}

void ImpostaStatoEffetti(bool attiva) {
    effettiAbilitati = attiva;
}

void CloseAudio(void) {
    UnloadMusicStream(bg);
    UnloadSound(sCard);
    UnloadSound(sWin);
    UnloadSound(sDef);
    CloseAudioDevice();
}