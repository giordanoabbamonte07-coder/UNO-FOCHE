#include "game.h"
#include "audio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char* nomi_colori_ita[] = {"ROSSO", "GIALLO", "VERDE", "BLU", "NERO"};

// --- FUNZIONI DI SERVIZIO ---

static void MescolaArray(Carta *array, int n) {
    for (int i = 0; i < n; i++) {
        int r = GetRandomValue(0, n - 1);
        Carta tmp = array[i];
        array[i] = array[r];
        array[r] = tmp;
    }
}

static void RigeneraMazzo(Partita *p) {
    // Portiamo gli scarti nel mazzo (p->cima NON viene toccata)
    if (p->num_scarto > 0) {
        for (int i = 0; i < p->num_scarto; i++) {
            p->mazzo[i] = p->scarto[i];
        }
        p->indice_mazzo = 0;
        int conteggio = p->num_scarto;
        p->num_scarto = 0;
        MescolaArray(p->mazzo, conteggio);
    } else {
        // Fallback di sicurezza se il mazzo è vuoto e non ci sono scarti
        int k = 0;
        for (int c = 0; c < 4; c++) {
            for (int v = 0; v <= 9; v++) p->mazzo[k++] = (Carta){(Colore)c, NUMERO, v};
        }
        p->indice_mazzo = 0;
        MescolaArray(p->mazzo, k);
    }
}

Carta Pesca(Partita *p) {
    if (p->indice_mazzo >= MAZZO_TOTALE) {
        RigeneraMazzo(p);
    }
    return p->mazzo[p->indice_mazzo++];
}

static void ImpostaMessaggio(Partita *p, const char* testo) {
    strncpy(p->messaggio, testo, 63);
    p->messaggio[63] = '\0';
    p->timer_messaggio = 3.5f;
}

static void ApplicaPenalita(Partita *p, int vittima, int quantita) {
    Giocatore *bersaglio = (vittima == 0) ? &p->giocatore : &p->bot;
    for (int i = 0; i < quantita; i++) {
        if (bersaglio->num_carte < MANO_MAX) {
            bersaglio->mano[bersaglio->num_carte++] = Pesca(p);
        }
    }
    if (bersaglio->num_carte > 1) bersaglio->ha_notificato_ultima = false;
}

// --- LOGICA DI GIOCO ---

bool PuoGiocare(Carta c, Partita *p) {
    if (c.colore == NERO) return true;
    if (c.colore == p->colore_attuale) return true;
    if (c.tipo == NUMERO && p->cima.tipo == NUMERO && c.valore == p->cima.valore) return true;
    if (c.tipo != NUMERO && c.tipo == p->cima.tipo) return true;
    return false;
}

bool GiocatoreHaMosse(Partita *p) {
    for (int i = 0; i < p->giocatore.num_carte; i++) {
        if (PuoGiocare(p->giocatore.mano[i], p)) return true;
    }
    return false;
}

void InizializzaPartita(Partita *p) {
    memset(p, 0, sizeof(Partita));
    p->pos_x_deltaplano = (float)SCHERMO_LARGHEZZA;

    int i = 0;
    for (int c = 0; c < 4; c++) {
        p->mazzo[i++] = (Carta){(Colore)c, NUMERO, 0};
        for (int v = 1; v <= 9; v++) {
            p->mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
            p->mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
        }
        for (int v = 0; v < 2; v++) {
            p->mazzo[i++] = (Carta){(Colore)c, SALTA, 0};
            p->mazzo[i++] = (Carta){(Colore)c, INVERTI, 0};
            p->mazzo[i++] = (Carta){(Colore)c, PIU_DUE, 0};
        }
    }
    for (int j = 0; j < 4; j++) {
        p->mazzo[i++] = (Carta){NERO, PIU_QUATTRO, 0};
        p->mazzo[i++] = (Carta){NERO, CAMBIA_COLORE, 0};
    }

    MescolaArray(p->mazzo, MAZZO_TOTALE);

    for (int k = 0; k < 7; k++) {
        p->giocatore.mano[p->giocatore.num_carte++] = Pesca(p);
        p->bot.mano[p->bot.num_carte++] = Pesca(p);
    }

    p->cima = Pesca(p);
    while (p->cima.colore == NERO) p->cima = Pesca(p);
    p->colore_attuale = p->cima.colore;

    // Regola UNO: Effetto prima carta
    if (p->cima.tipo == SALTA || p->cima.tipo == INVERTI) {
        p->turno = 1;
    } else if (p->cima.tipo == PIU_DUE) {
        ApplicaPenalita(p, 0, 2);
        p->turno = 1;
    }
}

static void ProcessaCartaGiocata(Partita *p) {
    if (p->num_scarto < MAZZO_TOTALE) {
        p->scarto[p->num_scarto++] = p->cima;
    }

    Carta c = p->carta_animata;
    p->cima = c;
    PlayCardSound();

    bool passaTurno = true;
    int vittima = (p->da_giocatore == 0) ? 1 : 0;

    if (c.tipo == SALTA || c.tipo == INVERTI) {
        passaTurno = false;
        if (p->da_giocatore == 1) {
            ImpostaMessaggio(p, (c.tipo == SALTA) ? "IL BOT TI HA BLOCCATO!" : "IL BOT HA CAMBIATO GIRO!");
        }
    } else if (c.tipo == PIU_DUE) {
        ApplicaPenalita(p, vittima, 2);
        passaTurno = false;
        if (p->da_giocatore == 1) ImpostaMessaggio(p, "IL BOT TI HA DATO UN +2!");
    }

    if (c.colore == NERO) {
        if (p->da_giocatore == 0) {
            p->scegli_colore = true;
            p->anima_foca = true;
            if (c.tipo == PIU_QUATTRO) ApplicaPenalita(p, 1, 4);
            p->prossimo_turno = passaTurno ? 1 : 0;
        } else {
            p->colore_attuale = GetRandomValue(0, 3);
            char msg[64];
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, 0, 4);
                sprintf(msg, "IL BOT HA LANCIATO UN +4! COLORE: %s", nomi_colori_ita[p->colore_attuale]);
            } else {
                sprintf(msg, "IL BOT HA SCELTO IL COLORE: %s", nomi_colori_ita[p->colore_attuale]);
            }
            ImpostaMessaggio(p, msg);
            p->turno = passaTurno ? 0 : 1;
        }
    } else {
        p->colore_attuale = c.colore;
        p->turno = passaTurno ? (p->da_giocatore == 0 ? 1 : 0) : p->da_giocatore;
    }
    p->puo_passare = false;
}

void GestisceInput(Partita *p) {
    // BLOCCO TOTALE INPUT durante animazioni o deltaplano
    if (p->partita_finita || p->turno != 0 || p->animando || p->timer_messaggio > 0 || p->anima_deltaplano) return;

    Vector2 m = GetMousePosition();

    if (p->scegli_colore) {
        for (int i = 0; i < 4; i++) {
            Rectangle r = { SCHERMO_LARGHEZZA/2.0f - 160 + i*85, SCHERMO_ALTEZZA/2.0f - 40, 75, 75 };
            if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                p->colore_attuale = (Colore)i;
                p->scegli_colore = false;
                p->anima_foca = false;
                p->turno = p->prossimo_turno;
            }
        }
        return;
    }

    // Pesca
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 160, SCHERMO_ALTEZZA/2.0f - 75, 100, 150 })) {
        if (p->giocatore.num_carte < MANO_MAX) {
            Carta pescata = Pesca(p);
            p->giocatore.mano[p->giocatore.num_carte++] = pescata;
            if (!PuoGiocare(pescata, p)) p->turno = 1;
            else p->puo_passare = true;
        }
    }

    // Gioca carta
    int spacing = 65;
    int totalW = (p->giocatore.num_carte - 1) * spacing;
    int startX = SCHERMO_LARGHEZZA/2 - totalW / 2;
    for (int i = p->giocatore.num_carte - 1; i >= 0; i--) {
        Rectangle r = {startX + i*spacing - 50, SCHERMO_ALTEZZA - 175, 100, 150};
        if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (PuoGiocare(p->giocatore.mano[i], p)) {
                p->carta_animata = p->giocatore.mano[i];
                for (int k = i; k < p->giocatore.num_carte - 1; k++) p->giocatore.mano[k] = p->giocatore.mano[k+1];
                p->giocatore.num_carte--;
                p->animando = true;
                p->timer_anim = 0;
                p->durata_animazione = 0.3f;
                p->da_giocatore = 0;
                p->inizio_animazione = (Vector2){(float)startX + i*spacing, (float)SCHERMO_ALTEZZA - 100};
                p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};
            }
            break;
        }
    }
}

void AggiornaPartita(Partita *p) {
    if (p->partita_finita) return;

    // 1. GESTIONE DELTAPLANO (Priorità assoluta: congela tutto il resto)
    if (p->anima_deltaplano) {
        p->pos_x_deltaplano -= 650.0f * GetFrameTime();
        if (p->pos_x_deltaplano < -400.0f) {
            p->anima_deltaplano = false;
            p->pos_x_deltaplano = (float)SCHERMO_LARGHEZZA;
        }
        return;
    }

    // 2. TRIGGER UNO (Solo se non ci sono animazioni di carte attive)
    if (!p->animando) {
        if ((p->giocatore.num_carte == 1 && !p->giocatore.ha_notificato_ultima) ||
            (p->bot.num_carte == 1 && !p->bot.ha_notificato_ultima)) {
            p->anima_deltaplano = true;
            if (p->giocatore.num_carte == 1) p->giocatore.ha_notificato_ultima = true;
            if (p->bot.num_carte == 1) {
                p->bot.ha_notificato_ultima = true;
                ImpostaMessaggio(p, "IL BOT HA UNA SOLA CARTA!");
            }
            return;
        }
    }

    if (p->timer_messaggio > 0) {
        p->timer_messaggio -= GetFrameTime();
        return;
    }

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

    // Turno IA BOT
    if (p->turno == 1 && !p->scegli_colore) {
        int idx = -1;
        for (int i = 0; i < p->bot.num_carte; i++) {
            if (PuoGiocare(p->bot.mano[i], p)) { idx = i; break; }
        }

        if (idx != -1) {
            p->carta_animata = p->bot.mano[idx];
            for (int k = idx; k < p->bot.num_carte - 1; k++) p->bot.mano[k] = p->bot.mano[k+1];
            p->bot.num_carte--;
            p->animando = true;
            p->timer_anim = 0;
            p->durata_animazione = 0.4f;
            p->da_giocatore = 1;
            p->inizio_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f, 100.0f};
            p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};
        } else {
            p->bot.mano[p->bot.num_carte++] = Pesca(p);
            p->turno = 0;
        }
    }

    // Condizione Vittoria
    if (p->giocatore.num_carte == 0 || p->bot.num_carte == 0) {
        p->partita_finita = true;
        if (!p->suono_fine_giocato) {
            StopBackgroundMusic();
            if (p->giocatore.num_carte == 0) PlayWinSound(); else PlayDefeatSound();
            p->suono_fine_giocato = true;
        }
    }
}