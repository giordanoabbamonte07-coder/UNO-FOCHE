#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef enum {
    STATO_MENU,
    STATO_ACCEDI,
    STATO_REGISTRATI,
    STATO_DEBUG,
    STATO_MENU_SCELTA,
    STATO_GIOCO,
    STATO_REGOLE,
    STATO_IMPOSTAZIONI,
    STATO_ARCHIVIO,
    STATO_STATISTICHE
} StatoGioco;

typedef struct {
    Texture2D sfondo;
    Texture2D btn_accedi;
    Texture2D btn_registrati;
    Texture2D btn_ospite;
    Texture2D sfondo_accedi;
    Texture2D sfondo_registrati;
    Texture2D tex_utente;
    Texture2D tex_password;
    char nickname[12];
    char password[12];
    int campo_attivo;
    int frame_counter;
} MenuGrafica;

// Struttura dati unificata e pulita per il database utenti
typedef struct {
    char nickname[11];
    char password[11];
    char ultimo_accesso[24];
    int ultima_azione; // 0 = Registrazione, 1 = Login
} UtenteRegistrato;

void InizializzaMenu(MenuGrafica *mg);
StatoGioco AggiornaDisegnaMenu(MenuGrafica *mg);
StatoGioco GestisciSchermataAuth(MenuGrafica *mg, StatoGioco stato);
void ScaricaMenu(MenuGrafica *mg);

#endif