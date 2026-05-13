#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define MANO_MAX 50
#define MAZZO_TOTALE 108
#define SCHERMO_LARGHEZZA 1200
#define SCHERMO_ALTEZZA 800
#define CARTA_LARGHEZZA 100
#define CARTA_ALTEZZA 150

typedef enum { ROSSO, GIALLO, VERDE, BLU, NERO } Colore;
typedef enum { NUMERO, SALTA, INVERTI, PIU_DUE, PIU_QUATTRO, CAMBIA_COLORE } Tipo;

typedef struct {
    Colore colore;
    Tipo tipo;
    int valore;
} Carta;

typedef struct {
    Carta mano[MANO_MAX];
    int num_carte;
    bool ha_notificato_ultima;
} Giocatore;

typedef struct {
    Giocatore giocatore;
    Giocatore bot;
    Carta mazzo[MAZZO_TOTALE];
    Carta scarto[MAZZO_TOTALE];
    int indice_mazzo;
    int num_scarto;

    Carta cima;
    Colore colore_attuale;
    int turno;
    bool partita_finita;

    bool animando;
    Carta carta_animata;
    Vector2 posizione_animazione;
    Vector2 inizio_animazione;
    Vector2 fine_animazione;
    float timer_anim;
    float durata_animazione;
    int da_giocatore;

    char messaggio[64];
    float timer_messaggio;
    bool scegli_colore;
    bool anima_foca;
    int prossimo_turno;

    bool anima_deltaplano;
    float pos_x_deltaplano;
    bool suono_fine_giocato;

    bool puo_passare;
} Partita;

// Esportazione dell'array dei nomi colori per graphics.c
extern const char* nomi_colori_ita[];

void InizializzaPartita(Partita *p);
void AggiornaPartita(Partita *p);
void GestisceInput(Partita *p);
bool PuoGiocare(Carta c, Partita *p);
bool GiocatoreHaMosse(Partita *p); // Dichiarazione aggiunta
Carta Pesca(Partita *p);

#endif