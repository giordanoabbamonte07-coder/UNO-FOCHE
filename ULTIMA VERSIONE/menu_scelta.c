#include "menu_scelta.h"
#include "audio.h"
#include <string.h>

#define SCHERMO_LARGHEZZA 1200.0f
#define SCHERMO_ALTEZZA 800.0f

static Sound suono_bottone;
static bool effettiMenuAttivi = true;

void InizializzaMenuScelta(MenuScelta *menu) {
    memset(menu, 0, sizeof(MenuScelta));

    menu->sfondo = LoadTexture("Sfondo/sfondo_pagina_main.png");
    menu->sfondoImpostazioni = LoadTexture("Sfondo/sfondo_impostazioni.png");
    menu->sfondoArchivio = LoadTexture("Sfondo/sfondo_storico.png");
    menu->sfondoRegole = LoadTexture("Sfondo/regole.png");

    menu->btn1vs1 = LoadTexture("Pulsanti/1vs1.png");
    menu->btnMulti = LoadTexture("Pulsanti/multigiocatore.png");
    menu->btnRegole = LoadTexture("Pulsanti/regole.png");
    menu->btnImpostazioni = LoadTexture("Pulsanti/impostazioni.png");
    menu->btnArchivio = LoadTexture("Pulsanti/archivio.png");
    suono_bottone = LoadSound("Musiche/suono_bottone.mp3");
    menu->btnIndietro = LoadTexture("Pulsanti/indietro.png");

    menu->btnOnMusica = LoadTexture("Pulsanti/on.png");
    menu->btnOnAudio = LoadTexture("Pulsanti/on.png");
    menu->btnOff = LoadTexture("Pulsanti/off.png");

    menu->musicaAttiva = true;
    menu->audioAttivo = true;
    effettiMenuAttivi = true;

    menu->nascondi_archivio = false;

    float larghPulsantiBase = 440.0f;
    float larghArchivio = 340.0f;
    float larghImpostazioni = 250.0f;

    menu->rect1vs1.width = larghPulsantiBase;
    menu->rect1vs1.height = (float)menu->btn1vs1.height * (larghPulsantiBase / (float)menu->btn1vs1.width);

    menu->rectMulti.width = larghPulsantiBase;
    menu->rectMulti.height = (float)menu->btnMulti.height * (larghPulsantiBase / (float)menu->btnMulti.width);

    menu->rectRegole.width = larghPulsantiBase;
    menu->rectRegole.height = (float)menu->btnRegole.height * (larghPulsantiBase / (float)menu->btnRegole.width);

    float hArch = (float)menu->btnArchivio.height * (larghArchivio / (float)menu->btnArchivio.width);
    float hImpo = (float)menu->btnImpostazioni.height * (larghImpostazioni / (float)menu->btnImpostazioni.width);

    float regola_1vs1_X = -20.0f;
    float regola_1vs1_Y = 0.0f;
    float regola_multi_X = -15.0f;
    float regola_multi_Y = 0.0f;
    float regola_regole_X = -10.0f;
    float regola_regole_Y = -18.0f;
    float regola_archivio_X = -20.0f;
    float regola_archivio_Y = -20.0f;
    float regola_impostazioni_X = 15.0f;
    float regola_impostazioni_Y = 0.0f;

    float centroSchermoX = SCHERMO_LARGHEZZA / 2.0f;
    float startY = 210.0f;
    float spaziatura = -90.0f;

    menu->rect1vs1.x = (centroSchermoX - (menu->rect1vs1.width / 2.0f)) + regola_1vs1_X;
    menu->rect1vs1.y = startY + regola_1vs1_Y;

    menu->rectMulti.x = (centroSchermoX - (menu->rectMulti.width / 2.0f)) + regola_multi_X;
    menu->rectMulti.y = (menu->rect1vs1.y + menu->rect1vs1.height + spaziatura) + regola_multi_Y;

    menu->rectRegole.x = (centroSchermoX - (menu->rectRegole.width / 2.0f)) + regola_regole_X;
    menu->rectRegole.y = (menu->rectMulti.y + menu->rectMulti.height + spaziatura) + regola_regole_Y;

    menu->rectArchivio.x = regola_archivio_X;
    menu->rectArchivio.y = regola_archivio_Y;
    menu->rectArchivio.width = larghArchivio;
    menu->rectArchivio.height = hArch;

    float margineBordoDestro = 20.0f;
    float rigaAltezzaTopImpostazioni = 20.0f;
    menu->rectImpostazioni.x = (SCHERMO_LARGHEZZA - larghImpostazioni - margineBordoDestro) + regola_impostazioni_X;
    menu->rectImpostazioni.y = rigaAltezzaTopImpostazioni + regola_impostazioni_Y;
    menu->rectImpostazioni.width = larghImpostazioni;
    menu->rectImpostazioni.height = hImpo;

    float scalaTasto = 0.10f;
    float larghezzaIndietro = (float)menu->btnIndietro.width * scalaTasto;
    float altezzaIndietro = (float)menu->btnIndietro.height * scalaTasto;
    menu->rectIndietro = (Rectangle){ 25.0f, SCHERMO_ALTEZZA - altezzaIndietro - 25.0f, larghezzaIndietro, altezzaIndietro };
}

int AggiornaDisegnaMenuScelta(MenuScelta *menu) {
    Vector2 mousePos = GetMousePosition();
    int statoSelezionato = STATO_MENU_SCELTA_CORRENTE;

    effettiMenuAttivi = menu->audioAttivo;

    if (CheckCollisionPointRec(mousePos, menu->rect1vs1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_1VS1;
    }
    if (CheckCollisionPointRec(mousePos, menu->rectMulti) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_MULTIGIOCATORE;
    }
    if (CheckCollisionPointRec(mousePos, menu->rectRegole) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_REGOLE;
    }
    if (CheckCollisionPointRec(mousePos, menu->rectImpostazioni) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_IMPOSTAZIONI;
    }
    if (!menu->nascondi_archivio) {
        if (CheckCollisionPointRec(mousePos, menu->rectArchivio) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (effettiMenuAttivi) PlaySound(suono_bottone);
            statoSelezionato = VAI_ARCHIVIO;
        }
    }
    if (CheckCollisionPointRec(mousePos, menu->rectIndietro) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (effettiMenuAttivi) PlaySound(suono_bottone);
        statoSelezionato = VAI_INDIETRO;
    }

    if (menu->sfondo.id > 0) {
        DrawTexturePro(menu->sfondo,
            (Rectangle){ 0, 0, (float)menu->sfondo.width, (float)menu->sfondo.height },
            (Rectangle){ 0, 0, SCHERMO_LARGHEZZA, SCHERMO_ALTEZZA },
            (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    Color c1vs1 = CheckCollisionPointRec(mousePos, menu->rect1vs1) ? LIGHTGRAY : WHITE;
    Color cMulti = CheckCollisionPointRec(mousePos, menu->rectMulti) ? LIGHTGRAY : WHITE;
    Color cRegole = CheckCollisionPointRec(mousePos, menu->rectRegole) ? LIGHTGRAY : WHITE;
    Color cImpostazioni = CheckCollisionPointRec(mousePos, menu->rectImpostazioni) ? LIGHTGRAY : WHITE;
    Color cIndietro = CheckCollisionPointRec(mousePos, menu->rectIndietro) ? LIGHTGRAY : WHITE;

    if (menu->btn1vs1.id > 0) DrawTexturePro(menu->btn1vs1, (Rectangle){0, 0, (float)menu->btn1vs1.width, (float)menu->btn1vs1.height}, menu->rect1vs1, (Vector2){0, 0}, 0.0f, c1vs1);
    if (menu->btnMulti.id > 0) DrawTexturePro(menu->btnMulti, (Rectangle){0, 0, (float)menu->btnMulti.width, (float)menu->btnMulti.height}, menu->rectMulti, (Vector2){0, 0}, 0.0f, cMulti);
    if (menu->btnRegole.id > 0) DrawTexturePro(menu->btnRegole, (Rectangle){0, 0, (float)menu->btnRegole.width, (float)menu->btnRegole.height}, menu->rectRegole, (Vector2){0, 0}, 0.0f, cRegole);
    if (menu->btnImpostazioni.id > 0) DrawTexturePro(menu->btnImpostazioni, (Rectangle){0, 0, (float)menu->btnImpostazioni.width, (float)menu->btnImpostazioni.height}, menu->rectImpostazioni, (Vector2){0, 0}, 0.0f, cImpostazioni);

    if (!menu->nascondi_archivio) {
        Color cArchivio = CheckCollisionPointRec(mousePos, menu->rectArchivio) ? LIGHTGRAY : WHITE;
        if (menu->btnArchivio.id > 0) DrawTexturePro(menu->btnArchivio, (Rectangle){0, 0, (float)menu->btnArchivio.width, (float)menu->btnArchivio.height}, menu->rectArchivio, (Vector2){0, 0}, 0.0f, cArchivio);
    }

    if (menu->btnIndietro.id > 0) {
        DrawTexturePro(menu->btnIndietro, (Rectangle){ 0, 0, (float)menu->btnIndietro.width, (float)menu->btnIndietro.height }, menu->rectIndietro, (Vector2){ 0, 0 }, 0.0f, cIndietro);
    }

    return statoSelezionato;
}

void DeinizializzaMenuScelta(MenuScelta *menu) {
    UnloadTexture(menu->sfondo);
    UnloadTexture(menu->sfondoImpostazioni);
    UnloadTexture(menu->sfondoArchivio);
    UnloadTexture(menu->sfondoRegole);
    UnloadTexture(menu->btn1vs1);
    UnloadTexture(menu->btnMulti);
    UnloadTexture(menu->btnRegole);
    UnloadTexture(menu->btnImpostazioni);
    UnloadTexture(menu->btnArchivio);
    UnloadTexture(menu->btnIndietro);
    UnloadTexture(menu->btnOnMusica);
    UnloadTexture(menu->btnOnAudio);
    UnloadTexture(menu->btnOff);
    UnloadSound(suono_bottone);
}
