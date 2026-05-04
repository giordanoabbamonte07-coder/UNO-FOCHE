#include "game.h"
#include "audio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char* nomi_colori_ita[] = {"ROSSO", "GIALLO", "VERDE", "BLU", "NERO"};

static void MescolaMazzo(Partita *p) {
    for (int j = 0; j < MAZZO_TOTALE; j++) {
        int r = GetRandomValue(0, MAZZO_TOTALE - 1);
        Carta tmp = p->mazzo[j];
        p->mazzo[j] = p->mazzo[r];
        p->mazzo[r] = tmp;
    }
}

static void PreparaMazzo(Partita *p) {
    int i = 0;
    for (int c = 0; c < 4; c++) {
        p->mazzo[i++] = (Carta){(Colore)c, NUMERO, 0};
        for (int v = 1; v <= 9; v++) {
            p->mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
            p->mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
        }
        for (int k = 0; k < 2; k++) {
            p->mazzo[i++] = (Carta){(Colore)c, SALTA, 0};
            p->mazzo[i++] = (Carta){(Colore)c, INVERTI, 0};
            p->mazzo[i++] = (Carta){(Colore)c, PIU_DUE, 0};
        }
    }
    for (int j = 0; j < 4; j++) {
        p->mazzo[i++] = (Carta){NERO, PIU_QUATTRO, 0};
        p->mazzo[i++] = (Carta){NERO, CAMBIA_COLORE, 0};
    }
    p->indice_mazzo = 0;
    MescolaMazzo(p);
}

Carta Pesca(Partita *p) {
    if (p->indice_mazzo >= MAZZO_TOTALE) {
        p->indice_mazzo = 0;
        MescolaMazzo(p);
    }
    return p->mazzo[p->indice_mazzo++];
}

static void ApplicaPenalita(Partita *p, int vittima, int quantita) {
    Giocatore *bersaglio = (vittima == 0) ? &p->giocatore : &p->bot;
    for (int i = 0; i < quantita; i++) {
        if (bersaglio->num_carte < MANO_MAX) {
            bersaglio->mano[bersaglio->num_carte++] = Pesca(p);
        }
    }
}

bool PuoGiocare(Carta c, Partita *p) {
    if (c.colore == NERO) return true;
    if (c.colore == p->colore_attuale) return true;
    if (c.tipo == NUMERO && p->cima.tipo == NUMERO && c.valore == p->cima.valore) return true;
    if (c.tipo != NUMERO && c.tipo == p->cima.tipo) return true;
    return false;
}

static void ImpostaMessaggio(Partita *p, const char* testo) {
    strncpy(p->messaggio, testo, 63);
    p->messaggio[63] = '\0';
    p->timer_messaggio = 2.0f;
}

static Colore ScegliColoreMiglioreBot(Giocatore *bot) {
    int conteggio[4] = {0, 0, 0, 0};
    for (int i = 0; i < bot->num_carte; i++) {
        if (bot->mano[i].colore != NERO) conteggio[bot->mano[i].colore]++;
    }
    int max = -1; Colore scelto = ROSSO;
    for (int i = 0; i < 4; i++) {
        if (conteggio[i] > max) { max = conteggio[i]; scelto = (Colore)i; }
    }
    return scelto;
}

void InizializzaPartita(Partita *p) {
    p->giocatore.num_carte = p->bot.num_carte = 0;
    p->scegli_colore = p->anima_foca = p->partita_finita = false;
    p->suono_fine_giocato = false;
    p->timer_messaggio = 0;
    p->turno = 0;
    p->prossimo_turno = 0;
    p->animando = false;
    memset(p->messaggio, 0, sizeof(p->messaggio));
    PreparaMazzo(p);
    for (int i = 0; i < 7; i++) {
        p->giocatore.mano[p->giocatore.num_carte++] = Pesca(p);
        p->bot.mano[p->bot.num_carte++] = Pesca(p);
    }
    p->cima = Pesca(p);
    while (p->cima.colore == NERO) p->cima = Pesca(p);
    p->colore_attuale = p->cima.colore;
}

static void ProcessaCartaGiocata(Partita *p) {
    Carta c = p->carta_animata;
    p->cima = c;
    PlayCardSound();
    bool passaTurno = true;
    if (c.tipo == SALTA || c.tipo == INVERTI || c.tipo == PIU_DUE || c.tipo == PIU_QUATTRO) passaTurno = false;

    int giocatore = p->da_giocatore;
    int vittima = (giocatore == 0) ? 1 : 0;

    if (c.tipo == PIU_DUE) {
        ApplicaPenalita(p, vittima, 2);
        if (giocatore == 1) ImpostaMessaggio(p, "BOT: +2 CARTE!");
    }
    if (giocatore == 1 && c.tipo == SALTA) ImpostaMessaggio(p, "BOT: TURNO BLOCCATO!");
    if (giocatore == 1 && c.tipo == INVERTI) ImpostaMessaggio(p, "BOT: CAMBIO GIRO!");

    if (c.colore == NERO) {
        if (giocatore == 0) {
            p->scegli_colore = true;
            p->anima_foca = true;
            p->prossimo_turno = passaTurno ? 1 : 0;
        } else {
            p->colore_attuale = ScegliColoreMiglioreBot(&p->bot);
            char buf[64];
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, 0, 4);
                sprintf(buf, "BOT: +4! COLORE: %s", nomi_colori_ita[p->colore_attuale]);
            } else {
                sprintf(buf, "BOT: COLORE %s", nomi_colori_ita[p->colore_attuale]);
            }
            ImpostaMessaggio(p, buf);
            p->turno = passaTurno ? 0 : 1;
        }
    } else {
        p->colore_attuale = c.colore;
        p->turno = passaTurno ? (giocatore == 0 ? 1 : 0) : giocatore;
    }
}

void GestisciInput(Partita *p) {
    if (p->partita_finita || p->turno != 0 || p->animando) return;
    Vector2 m = GetMousePosition();
    if (p->scegli_colore) {
        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 160 + (float)i * 85, SCHERMO_ALTEZZA/2.0f-40, 75, 75 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                p->colore_attuale = (Colore)i;
                p->scegli_colore = false;
                p->anima_foca = false;
                p->turno = p->prossimo_turno;
            }
        }
        return;
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 160, SCHERMO_ALTEZZA/2.0f - 75, 100, 150 })) {
            if (p->giocatore.num_carte < MANO_MAX) {
                p->giocatore.mano[p->giocatore.num_carte++] = Pesca(p);
                p->turno = 1;
                return;
            }
        }
        int spacing = 65;
        int max_w = SCHERMO_LARGHEZZA - 400;
        if (p->giocatore.num_carte * spacing > max_w) spacing = max_w / p->giocatore.num_carte;
        int startX = SCHERMO_LARGHEZZA/2 - ((p->giocatore.num_carte - 1) * spacing) / 2;
        for (int i = p->giocatore.num_carte - 1; i >= 0; i--) {
            float dist = i - (p->giocatore.num_carte - 1) / 2.0f;
            float py = SCHERMO_ALTEZZA - 100 + abs((int)dist) * 3;
            if (CheckCollisionPointRec(m, (Rectangle){ (float)startX + i * spacing - 50, py - 75, 100, 150 })) {
                if (PuoGiocare(p->giocatore.mano[i], p)) {
                    Carta c = p->giocatore.mano[i];
                    for (int k = i; k < p->giocatore.num_carte - 1; k++) p->giocatore.mano[k] = p->giocatore.mano[k+1];
                    p->giocatore.num_carte--;
                    p->animando = true; p->carta_animata = c; p->timer_anim = 0; p->durata_animazione = 0.3f;
                    p->inizio_animazione = (Vector2){(float)startX + i * spacing, py};
                    p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};
                    p->da_giocatore = 0;
                    break;
                }
            }
        }
    }
}

void AggiornaPartita(Partita *p) {
    if (p->partita_finita) return;
    if (p->animando) {
        p->timer_anim += GetFrameTime();
        float t = p->timer_anim / p->durata_animazione;
        if (t >= 1.0f) { p->animando = false; ProcessaCartaGiocata(p); }
        else {
            p->posizione_animazione.x = p->inizio_animazione.x + (p->fine_animazione.x - p->inizio_animazione.x) * t;
            p->posizione_animazione.y = p->inizio_animazione.y + (p->fine_animazione.y - p->inizio_animazione.y) * t;
        }
        return;
    }
    if (p->timer_messaggio > 0) p->timer_messaggio -= GetFrameTime();
    if (p->turno == 1 && !p->scegli_colore && p->timer_messaggio <= 0) {
        int idx = -1;
        for (int i = 0; i < p->bot.num_carte; i++) {
            if (PuoGiocare(p->bot.mano[i], p)) { idx = i; break; }
        }
        if (idx != -1) {
            Carta c = p->bot.mano[idx];
            for (int k = idx; k < p->bot.num_carte - 1; k++) p->bot.mano[k] = p->bot.mano[k+1];
            p->bot.num_carte--;
            p->animando = true; p->carta_animata = c; p->timer_anim = 0; p->durata_animazione = 0.3f;
            p->inizio_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f, 100};
            p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};
            p->da_giocatore = 1;
        } else {
            if (p->bot.num_carte < MANO_MAX) p->bot.mano[p->bot.num_carte++] = Pesca(p);
            p->turno = 0;
        }
    }
    if (p->giocatore.num_carte == 0 || p->bot.num_carte == 0) {
        p->partita_finita = true;
        if (!p->suono_fine_giocato) {
            StopBackgroundMusic();
            if (p->giocatore.num_carte == 0) PlayWinSound(); else PlayDefeatSound();
            p->suono_fine_giocato = true;
        }
    }
}