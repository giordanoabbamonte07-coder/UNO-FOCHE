#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"

void InitAudio(void);
void UpdateAudio(void);
void CloseAudio(void);
void PlayCardSound(void);
void PlayWinSound(void);
void PlayDefeatSound(void);
void StopBackgroundMusic(void);

#endif