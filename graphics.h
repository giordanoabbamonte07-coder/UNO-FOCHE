#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

// =======================================================================================
// COSTANTI DIMENSIONALI
// =======================================================================================
#define CARTA_LARGHEZZA 100
#define CARTA_ALTEZZA   150

// =======================================================================================
// STRUTTURA DATI: Grafica
// =======================================================================================
typedef struct {
    // --- ELEMENTI DI SFONDO E POPUP ANIMATI ---
    Texture2D sfondo;         // Texture del tavolo da gioco principale
    Texture2D foca;           // Sprite/Texture centrale per l'animazione della Foca
    Texture2D deltaplano;     // Sprite/Texture per l'evento o popup del deltaplano
    Texture2D win_img;        // Schermata grafica di vittoria (caricata da "Sfondo/win.mp3" logico o immagini)
    Texture2D defeat_img;     // Schermata grafica di sconfitta
    Texture2D retro;          // Texture del retro della carta per il mazzo di pesca e i Bot

    // --- MATRICE DI CACHING DELLE CARTE ---
    // Struttura tridimensionale per mappare i pattern delle carte del gioco:
    // [Indice Colore][Variante/Tipo][Valore Nominale o Funzione]
    Texture2D carte[5][6][13];

    // --- INTERFACCIA UTENTE (UI) IN-GAME ---
    Texture2D btn_sospendi;   // Pulsante per salvare e sospendere la partita corrente
} Grafica;

// =======================================================================================
// INTERFACCIA DELLE FUNZIONI (PROTOTIPI)
// =======================================================================================

/**
 * @brief Tenta il caricamento di una texture da file. Se fallisce, restituisce una texture di fallback vuota.
 * @param path Percorso relativo del file immagine (es. "Sfondo/sfondo_pagina_main.png")
 * @return Texture2D La risorsa caricata correttamente in VRAM o una texture di sicurezza.
 */
Texture2D CaricaSicuro(const char *path);

/**
 * @brief Alloca in memoria video tutte le risorse grafiche, i popup e popola l'array delle carte.
 * @param gfx Puntatore alla struttura Grafica da inizializzare.
 */
void InizializzaGrafica(Grafica *gfx);

/**
 * @brief Core di disegno: renderizza il tavolo, le mani dei bot, le carte del giocatore e i popup attivi.
 * @param gfx Puntatore alle risorse grafiche allocate.
 * @param p Puntatore allo stato logico della partita in corso.
 */
void DisegnaPartita(Grafica *gfx, Partita *p);

/**
 * @brief Scarica in modo sicuro ogni singola texture per prevenire memory leak nella scheda video (VRAM).
 * @param gfx Puntatore alla struttura Grafica da ripulire.
 */
void ScaricaGrafica(Grafica *gfx);

/**
 * @brief Disegna un testo con effetto finto-pixel art applicando un'ombra o un contorno in grassetto.
 * @param testo Stringa di caratteri da stampare a schermo.
 * @param x Coordinata X di partenza.
 * @param y Coordinata Y di partenza.
 * @param fontSize Dimensione scalata del font.
 * @param baseColor Colore del corpo del testo principale.
 */
void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor);

#endif // GRAPHICS_H