#include "audio.h"

// =======================================================================================
// VARIABILI DI STATO MULTIMEDIALI INTERNE (STATIC / FILE-SCOPE)
// =======================================================================================
static Music bg;          // Descrittore Raylib per lo streaming audio di sottofondo (lunga durata)
static Sound sCard;       // Buffer audio per l'effetto sonoro del lancio/pesca della carta
static Sound sWin;        // Buffer audio per il jingle celebrativo di vittoria ("vittoria.mp3")
static Sound sDef;        // Buffer audio per il jingle di sconfitta ("sconfitta.mp3")

// Flag booleani di controllo per la gestione nativa del muto (on/off) richiamabili dalle opzioni
static bool musicaAbilitata = true;
static bool effettiAbilitati = true;

/**
 * Inizializza l'hardware audio del PC e alloca i flussi e i campioni sonori dai file multimediali
 */
void InitAudio(void) {
    InitAudioDevice(); // Inizializza il contesto audio del sistema operativo e i canali di output

    // Caricamento selettivo delle risorse dalle cartelle dedicate del progetto
    bg = LoadMusicStream("Musiche/musica.mp3"); // Carica come stream (lettura parcellizzata da disco)
    sCard = LoadSound("Musiche/suono_carta.mp3"); // Carica interamente in RAM (riproduzione immediata)
    sWin = LoadSound("Musiche/vittoria.mp3");
    sDef = LoadSound("Musiche/sconfitta.mp3");

    // Imposta lo stato iniziale dei volumi a regime attivo
    musicaAbilitata = true;
    effettiAbilitati = true;
}

/**
 * Avvia l'esecuzione del flusso musicale di sottofondo se le condizioni di configurazione lo permettono
 */
void PlayBackgroundMusic(void) {
    // Avvia la riproduzione solo se l'utente non ha mutato la musica e se non è già in esecuzione
    if (musicaAbilitata && !IsMusicStreamPlaying(bg)) {
        PlayMusicStream(bg);
    }
}

/**
 * Alimenta i buffer dello stream della musica. Deve risiedere nel ciclo principale dell'applicazione
 */
void UpdateAudio(void) {
    // Se la musica è attiva, ricarica ciclicamente i blocchi di byte audio decodificati da Raylib
    if (musicaAbilitata) {
        UpdateMusicStream(bg);
    }
}

/**
 * Interrompe istantaneamente la riproduzione della traccia di sottofondo riposizionando l'indice all'inizio
 */
void StopBackgroundMusic(void) {
    StopMusicStream(bg);
}

/**
 * Riproduce l'effetto sonoro della carta in movimento, se gli effetti globali sono abilitati
 */
void PlayCardSound(void) {
    if (effettiAbilitati) {
        PlaySound(sCard);
    }
}

/**
 * Riproduce il suono di vittoria a fine partita, se gli effetti globali sono abilitati
 */
void PlayWinSound(void) {
    if (effettiAbilitati) {
        PlaySound(sWin);
    }
}

/**
 * Riproduce il suono di sconfitta a fine partita, se gli effetti globali sono abilitati
 */
void PlayDefeatSound(void) {
    if (effettiAbilitati) {
        PlaySound(sDef);
    }
}

/**
 * Cambia lo stato di attivazione della musica. Interrompe subito il flusso se impostato su Muto
 */
void ImpostaStatoMusica(bool attiva) {
    musicaAbilitata = attiva;

    // Se l'utente disattiva la musica dalle opzioni, spegne immediatamente lo streaming attivo
    if (!musicaAbilitata) {
        StopBackgroundMusic();
    }
    // NOTA DI SICUREZZA: Non invochiamo PlayBackgroundMusic() nel ramo 'else' per evitare
    // l'avvio incontrollato e indesiderato della traccia musicale durante la navigazione nei menu.
}

/**
 * Cambia lo stato di attivazione degli effetti sonori del tavolo di gioco e della UI
 */
void ImpostaStatoEffetti(bool attiva) {
    effettiAbilitati = attiva;
}

/**
 * Libera tutta la memoria allocata per l'audio e chiude i descrittori di sistema prima del termine del gioco
 */
void CloseAudio(void) {
    UnloadMusicStream(bg);  // Dealloca lo stream della musica di sottofondo
    UnloadSound(sCard);     // Rilascia il campione dell'effetto carta
    UnloadSound(sWin);      // Rilascia il campione di vittoria
    UnloadSound(sDef);      // Rilascia il campione di sconfitta

    CloseAudioDevice();     // Chiude il dispositivo hardware audio precedentemente aperto
}