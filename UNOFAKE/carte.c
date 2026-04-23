#include "carte.h"
#include <stdio.h>

Texture2D caricaTexture(int colore, int valore) {
    char path[100];
    const char *colori[] = {"rosso", "blu", "giallo", "verde"};

    // Gestione Jolly
    if (colore == JOLLY) {
        if (valore == PIU4)
            sprintf(path, "Carte/jolly_piu4.png");
        else
            sprintf(path, "Carte/jolly.png");
    } else {
        // Gestione carte colorate
        if (valore <= 9)
            sprintf(path, "Carte/%s_%d.png", colori[colore], valore);
        else if (valore == SKIP)
            sprintf(path, "Carte/%s_skip.png", colori[colore]);
        else if (valore == REVERSE)
            sprintf(path, "Carte/%s_reverse.png", colori[colore]);
        else if (valore == PIU2)
            sprintf(path, "Carte/%s_piu2.png", colori[colore]);
    }

    Texture2D tex = LoadTexture(path);

    // Controllo Debug: se la texture ha ID 0, il file non è stato trovato
    if (tex.id == 0) {
        printf("ERRORE: Impossibile trovare il file: %s\n", path);
    }

    return tex;
}

Carta creaCarta(int colore, int valore) {
    Carta c;
    c.colore = colore;
    c.valore = valore;
    c.texture = caricaTexture(colore, valore);
    return c;
}