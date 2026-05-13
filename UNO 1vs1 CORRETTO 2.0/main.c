#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"

int main(void) {
    InitWindow(SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA, "UNO FOCA - EDIZIONE DEFINITIVA");
    SetTargetFPS(60);
    InitAudio();

    Partita partita;
    Grafica grafica;

    InizializzaPartita(&partita);
    InizializzaGrafica(&grafica);

    while (!WindowShouldClose()) {
        UpdateAudio();
        GestisceInput(&partita);
        if (!partita.animando && partita.timer_messaggio <= 0) {
            GestisceInput(&partita);
        }
        AggiornaPartita(&partita);
        DisegnaPartita(&grafica, &partita);
    }

    ScaricaGrafica(&grafica);
    CloseAudio();
    CloseWindow();
    return 0;
}