#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"

// =======================================================================================
// FUNZIONI DI GESTIONE DEL SOTTOSISTEMA AUDIO (CICLO DI VITA)
// =======================================================================================

/**
 * @brief Inizializza il dispositivo hardware audio e carica in memoria RAM tutti i file sonori.
 * Carica la musica di sottofondo dalla cartella "Musiche/" e gli effetti d'ambiente.
 * Da invocare una sola volta all'avvio del gioco (nel main, subito dopo InitWindow).
 */
void InitAudio(void);

/**
 * @brief Aggiorna i buffer di streaming della musica di sottofondo.
 * Essendo la musica un flusso continuo (stream) e non un suono statico, questa funzione
 * deve essere invocata tassativamente ad ogni frame all'interno del ciclo principale (game loop).
 */
void UpdateAudio(void);

/**
 * @brief Scarica i file audio dalla RAM e chiude in sicurezza il dispositivo di output di Raylib.
 * Da invocare alla chiusura definitiva dell'applicazione per evitare memory leak audio.
 */
void CloseAudio(void);

// =======================================================================================
// FUNZIONI DI CONTROLLO DELLA MUSICA DI SFONDO (STREAMING)
// =======================================================================================

/**
 * @brief Avvia la riproduzione in loop della traccia musicale principale.
 * Se la musica è disattivata dalle impostazioni, la funzione gestisce internamente il blocco.
 */
void PlayBackgroundMusic(void);

/**
 * @brief Interrompe immediatamente la riproduzione della musica di sottofondo e ne resetta la posizione.
 */
void StopBackgroundMusic(void);

// =======================================================================================
// FUNZIONI DI RIPRODUZIONE DEGLI EFFETTI SONORI (SOUND EFFECTS)
// =======================================================================================

/**
 * @brief Riproduce il file audio associato al movimento o al lancio di una carta sul tavolo.
 */
void PlayCardSound(void);

/**
 * @brief Riproduce il tema audio celebrativo di vittoria ("win.mp3").
 * Utilizza internamente un controllo sul flag della struttura Partita per evitare l'avvio in loop sovrapposto.
 */
void PlayWinSound(void);

/**
 * @brief Riproduce il tema audio di sconfitta ("sconfitta.mp3") quando un bot azzera la mano prima dell'utente.
 */
void PlayDefeatSound(void);

// =======================================================================================
// FUNZIONI DI STATO E CONFIGURAZIONE (MENU OPZIONI / INTERFACCIA)
// =======================================================================================

/**
 * @brief Abilita o disabilita globalmente la musica di sottofondo.
 * Se impostata su false, azzera il volume o interrompe lo streaming corrente.
 * @param attiva Impostare su 'true' per sentire la musica, 'false' per mutarla.
 */
void ImpostaStatoMusica(bool attiva);

/**
 * @brief Abilita o disabilita la riproduzione di tutti gli effetti sonori secondari (carte, pop-up, click).
 * @param attiva Impostare su 'true' per sentire gli effetti, 'false' per mutarli.
 */
void ImpostaStatoEffetti(bool attiva);

#endif // AUDIO_H