#ifndef MENU_SCELTA_H
#define MENU_SCELTA_H

#include "raylib.h"

// Definizioni degli stati di ritorno del menu scelta
#define STATO_MENU_SCELTA_CORRENTE 0
#define VAI_1VS1 1
#define VAI_MULTIGIOCATORE 2
#define VAI_REGOLE 3
#define VAI_IMPOSTAZIONI 4
#define VAI_ARCHIVIO 5
#define VAI_INDIETRO 6

typedef struct {
    Texture2D sfondo;
    Texture2D sfondoImpostazioni;
    Texture2D sfondoArchivio;
    Texture2D sfondoRegole; // <-- Nuova texture per lo sfondo delle regole
    Texture2D btn1vs1;
    Texture2D btnMulti;
    Texture2D btnRegole;
    Texture2D btnImpostazioni;
    Texture2D btnArchivio;
    Texture2D btnIndietro;

    Texture2D btnOnMusica;
    Texture2D btnOnAudio;
    Texture2D btnOff;

    Rectangle rectOnMusica;
    Rectangle rectOnAudio;

    Rectangle rect1vs1;
    Rectangle rectMulti;
    Rectangle rectRegole;
    Rectangle rectImpostazioni;
    Rectangle rectArchivio;
    Rectangle rectIndietro;
    bool nascondi_archivio;

    bool musicaAttiva;
    bool audioAttivo;
} MenuScelta;

void InizializzaMenuScelta(MenuScelta *menu);
int AggiornaDisegnaMenuScelta(MenuScelta *menu);
void DeinizializzaMenuScelta(MenuScelta *menu);

#endif