#ifndef MAZZO_H
#define MAZZO_H

#include "carte.h"

#define MAZZO_SIZE 108

void inizializzaMazzo(Carta mazzo[], int *top);
void mescola(Carta mazzo[], int n);
Carta pesca(Carta mazzo[], int *top);

#endif