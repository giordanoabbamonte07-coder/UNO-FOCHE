#include "mazzo.h"
#include <stdlib.h>

void inizializzaMazzo(Carta mazzo[], int *top) {
    int index = 0;
    for (int colore = 0; colore < 4; colore++) {
        // Numeri da 0 a 9
        for (int v = 0; v <= 9; v++) {
            mazzo[index++] = creaCarta(colore, v);
            if (v != 0) mazzo[index++] = creaCarta(colore, v); // Due copie per 1-9
        }
        // Speciali: 2 per tipo per ogni colore
        for (int i = 0; i < 2; i++) {
            mazzo[index++] = creaCarta(colore, SKIP);
            mazzo[index++] = creaCarta(colore, REVERSE);
            mazzo[index++] = creaCarta(colore, PIU2);
        }
    }
    // Jolly: 4 normali e 4 +4
    for (int i = 0; i < 4; i++) {
        mazzo[index++] = creaCarta(JOLLY, JOLLY_CARD);
        mazzo[index++] = creaCarta(JOLLY, PIU4);
    }
    *top = index;
}

void mescola(Carta mazzo[], int n) {
    for (int i = 0; i < n; i++) {
        int j = rand() % n;
        Carta temp = mazzo[i];
        mazzo[i] = mazzo[j];
        mazzo[j] = temp;
    }
}

Carta pesca(Carta mazzo[], int *top) {
    if (*top > 0) return mazzo[--(*top)];
    return mazzo[0];
}