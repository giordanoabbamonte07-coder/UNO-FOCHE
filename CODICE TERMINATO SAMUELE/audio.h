#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"

void InitAudio(void);
void UpdateAudio(void);
void CloseAudio(void);

void PlayBackgroundMusic(void);
void StopBackgroundMusic(void);

void PlayCardSound(void);
void PlayWinSound(void);
void PlayDefeatSound(void);

// --- AGGIUNGI QUESTE DUE RIGHE QUI SOTTO ---
void ImpostaStatoMusica(bool attiva);
void ImpostaStatoEffetti(bool attiva);

#endif