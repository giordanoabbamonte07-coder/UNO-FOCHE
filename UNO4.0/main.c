#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"

int main(void) {
    // Inizializzazione finestra e impostazioni video
    InitWindow(SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, "UNO FOCA - EDIZIONE DEFINITIVA");
    SetTargetFPS(60);

    InitAudio(); // Carica musica e inizializza driver audio

    Partita partita;
    Grafica grafica;

    InizializzaPartita(&partita);     // Setup logica di gioco e mazzo
    InizializzaGrafica(&grafica);  // Caricamento texture e asset visivi

    // Loop principale di gioco
    while (!WindowShouldClose()) {
        UpdateAudio();   // Gestione streaming musicale
        GestisciInput(&partita); // Gestione click mouse e input utente
        AggiornaPartita(&partita);  // Logica BOT e controllo regole
        DisegnaPartita(&grafica, &partita); // Rendering a video
    }

    // Pulizia risorse prima della chiusura
    ScaricaGrafica(&grafica);
    CloseAudio();
    CloseWindow();

    return 0;
}