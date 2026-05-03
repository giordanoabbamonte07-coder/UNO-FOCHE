#include "game.h"
#include "audio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern const char* nomi_colori_ita[];
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
    Giocatore *bersaglio = (vittima == 0) ? &p->giocatore : &p->cpu;
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
    p->timer_messaggio = 2.5f;
}

static Colore ScegliColoreMiglioreCPU(Giocatore *cpu) {
    int conteggio[4] = {0, 0, 0, 0};
    for (int i = 0; i < cpu->num_carte; i++) {
        if (cpu->mano[i].colore != NERO) conteggio[cpu->mano[i].colore]++;
    }
    int max = -1; Colore scelto = ROSSO;
    for (int i = 0; i < 4; i++) {
        if (conteggio[i] > max) { max = conteggio[i]; scelto = (Colore)i; }
    }
    return scelto;
}

void InizializzaPartita(Partita *p) {
    p->giocatore.num_carte = p->cpu.num_carte = 0;
    p->scegli_colore = p->anima_foca = p->partita_finita = false;
    p->suono_fine_giocato = false;
    p->timer_messaggio = 0;
    p->turno = 0;
    p->prossimo_turno = 0;
    p->timer_animazione = 0;
    p->animando = false; // Importante inizializzarlo
    memset(p->messaggio, 0, sizeof(p->messaggio));

    PreparaMazzo(p);

    for (int i = 0; i < 7; i++) {
        p->giocatore.mano[p->giocatore.num_carte++] = Pesca(p);
        p->cpu.mano[p->cpu.num_carte++] = Pesca(p);
    }

    p->cima = Pesca(p);
    while (p->cima.colore == NERO) p->cima = Pesca(p);
    p->colore_attuale = p->cima.colore;
}

// Questa funzione viene chiamata SOLO QUANDO L'ANIMAZIONE È FINITA
// Così la foca e la scelta dei colori non appaiono prima che la carta sia arrivata!
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
        if (giocatore == 1) ImpostaMessaggio(p, "CPU: +2 CARTE!");
    }
    if (giocatore == 1 && (c.tipo == SALTA || c.tipo == INVERTI)) ImpostaMessaggio(p, "CPU: TURNO BLOCCATO!");

    if (c.colore == NERO) {
        if (giocatore == 0) {
            p->scegli_colore = true;
            p->anima_foca = true;
            p->prossimo_turno = passaTurno ? 1 : 0;
        } else {
            p->colore_attuale = ScegliColoreMiglioreCPU(&p->cpu);
            char buf[64];
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, 0, 4);
                sprintf(buf, "CPU: +4! COLORE: %s", nomi_colori_ita[p->colore_attuale]);
            } else {
                sprintf(buf, "CPU COLORE: %s", nomi_colori_ita[p->colore_attuale]);
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
    // Se c'è un'animazione in corso blocchiamo gli input!
    if (p->partita_finita || p->turno != 0 || p->timer_messaggio > 0.5f || p->animando) return;

    Vector2 m = GetMousePosition();

    if (p->scegli_colore) {
        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 160 + (float)i * 85, SCHERMO_ALTEZZA/2.0f - 40, 75, 75 }) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                p->colore_attuale = (Colore)i;
                p->scegli_colore = false;
                p->anima_foca = false;
                p->turno = p->prossimo_turno;
            }
        }
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 110 - 50, SCHERMO_ALTEZZA/2.0f - 75, 100, 150 })) {
            if (p->giocatore.num_carte < MANO_MAX) {
                p->giocatore.mano[p->giocatore.num_carte++] = Pesca(p);
                p->turno = 1;
            }
            return;
        }

        // Calcolo hitbox basato sullo stesso calcolo usato nella grafica (spacing dinamico)
        int max_width = SCHERMO_LARGHEZZA - 400;
        int spacing = 65;
        if (p->giocatore.num_carte > 0 && p->giocatore.num_carte * spacing > max_width) {
            spacing = max_width / p->giocatore.num_carte;
        }
        int startX = SCHERMO_LARGHEZZA/2 - ((p->giocatore.num_carte - 1) * spacing) / 2;

        for (int i = p->giocatore.num_carte - 1; i >= 0; i--) { // Ciclo inverso per cliccare quelle sopra
            // Calcolo posY come in DrawGame
            float posY = SCHERMO_ALTEZZA - 100; // Centro Y della carta
            float middle_index = (p->giocatore.num_carte - 1) / 2.0f;
            float distFromCenter = i - middle_index;
            posY += abs((int)distFromCenter) * 3; // Le carte ai lati scendono un pochino
            if (p->turno == 0 && PuoGiocare(p->giocatore.mano[i], p)) {
                posY -= 20; // Alza le carte giocabili
            }

            int x = startX + i * spacing - CARTA_LARGHEZZA/2;
            if (CheckCollisionPointRec(m, (Rectangle){ (float)x, posY - CARTA_ALTEZZA/2, CARTA_LARGHEZZA, CARTA_ALTEZZA })) {
                Carta c = p->giocatore.mano[i];
                if (PuoGiocare(c, p)) {
                    // 1. Rimuovi carta dalla mano
                    for (int k = i; k < p->giocatore.num_carte - 1; k++) p->giocatore.mano[k] = p->giocatore.mano[k+1];
                    p->giocatore.num_carte--;

                    // 2. Avvia Animazione verso il centro
                    p->animando = true;
                    p->carta_animata = c;
                    p->inizio_animazione = (Vector2){ (float)x + CARTA_LARGHEZZA/2, posY }; // Coordinate centro carta
                    p->fine_animazione = (Vector2){ SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f }; // Centro del mazzo di scarto
                    p->posizione_animazione = p->inizio_animazione;
                    p->timer_anim = 0;
                    p->durata_animazione = 0.35f; // Durata del volo (0.35 secondi)
                    p->da_giocatore = 0;  // Giocata dal player
                    break;
                }
            }
        }
    }
}

void AggiornaPartita(Partita *p) {
    if (p->partita_finita) return;

    // GESTIONE ANIMAZIONE IN VOLO (Stato di blocco)
    if (p->animando) {
        p->timer_anim += GetFrameTime();
        float t = p->timer_anim / p->durata_animazione;
        
        if (t >= 1.0f) {
            p->animando = false;
            ProcessaCartaGiocata(p); // Quando arriva, applica finalmente l'effetto!
        } else {
            // Formula interpolazione lineare (lerp) per creare il movimento
            p->posizione_animazione.x = p->inizio_animazione.x + (p->fine_animazione.x - p->inizio_animazione.x) * t;
            p->posizione_animazione.y = p->inizio_animazione.y + (p->fine_animazione.y - p->inizio_animazione.y) * t;
        }
        return; // Blocca la CPU dal fare altro mentre la carta vola!
    }

    if (p->timer_messaggio > 0) p->timer_messaggio -= GetFrameTime();

    // TURNO CPU
    if (p->turno == 1 && !p->scegli_colore && p->timer_messaggio <= 0) {
        int indiceMossa = -1; int prioritaMax = -1;

        for (int i = 0; i < p->cpu.num_carte; i++) {
            if (PuoGiocare(p->cpu.mano[i], p)) {
                int pr = 0;
                if (p->cpu.mano[i].tipo == PIU_QUATTRO) pr = 5;
                else if (p->cpu.mano[i].tipo == PIU_DUE || p->cpu.mano[i].tipo == SALTA || p->cpu.mano[i].tipo == INVERTI) pr = 4;
                else if (p->cpu.mano[i].colore == p->colore_attuale) pr = 3;
                if (pr > prioritaMax) { prioritaMax = pr; indiceMossa = i; }
            }
        }

        if (indiceMossa != -1) {
            Carta c = p->cpu.mano[indiceMossa];
            
            // 1. Rimuovi carta
            for (int k = indiceMossa; k < p->cpu.num_carte - 1; k++) p->cpu.mano[k] = p->cpu.mano[k+1];
            p->cpu.num_carte--;

            // 2. Avvia animazione dal bot al mazzo
            int max_width = SCHERMO_LARGHEZZA - 400;
            int spacingBot = 35;
            if (p->cpu.num_carte > 0 && p->cpu.num_carte * spacingBot > max_width) spacingBot = max_width / p->cpu.num_carte;
            int cpuStart = SCHERMO_LARGHEZZA/2 - ((p->cpu.num_carte - 1) * spacingBot) / 2;

            p->animando = true;
            p->carta_animata = c;
            p->inizio_animazione = (Vector2){ (float)cpuStart + indiceMossa * spacingBot + CARTA_LARGHEZZA/2, 20 + CARTA_ALTEZZA/2 };
            p->fine_animazione = (Vector2){ SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f };
            p->posizione_animazione = p->inizio_animazione;
            p->timer_anim = 0;
            p->durata_animazione = 0.35f;
            p->da_giocatore = 1; // Giocata dal Bot

        } else {
            if (p->cpu.num_carte < MANO_MAX) p->cpu.mano[p->cpu.num_carte++] = Pesca(p);
            p->turno = 0;
        }
    }

    // CONTROLLO VITTORIA (Avviene costantemente)
    if (p->giocatore.num_carte == 0 || p->cpu.num_carte == 0) {
        p->partita_finita = true;
        if (!p->suono_fine_giocato) {
            StopBackgroundMusic();
            if (p->giocatore.num_carte == 0) PlayWinSound();
            else PlayDefeatSound();
            p->suono_fine_giocato = true;
        }
    }
}