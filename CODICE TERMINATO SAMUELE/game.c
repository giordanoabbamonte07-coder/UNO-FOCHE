#include "game.h"
#include "audio.h"
#include "archivio.h" // <-- Incluso per poter chiamare SalvaRisultatoPartita
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char* nomi_colori_ita[] = {"ROSSO", "GIALLO", "VERDE", "BLU", "NERO"};

Vector2 OttieniPosizioneMazzoGiocatore(int index, int num_giocatori) {
    if (index == 0) return (Vector2){ 600.0f, 700.0f }; // Umano

    if (num_giocatori == 2) {
        if (index == 1) return (Vector2){ 600.0f, 115.0f }; // 1vs1: Bot Alto
    } else {
        if (index == 1) return (Vector2){ 120.0f, 400.0f };  // 1vs3: Bot Sinistra
        if (index == 2) return (Vector2){ 600.0f, 115.0f };  // 1vs3: Bot Alto
        if (index == 3) return (Vector2){ 1080.0f, 400.0f }; // 1vs3: Bot Destra
    }
    return (Vector2){ 600.0f, 400.0f };
}

static void MescolaArray(Carta *array, int n) {
    for (int i = 0; i < n; i++) {
        int r = GetRandomValue(0, n - 1);
        Carta tmp = array[i];
        array[i] = array[r];
        array[r] = tmp;
    }
}

static void RigeneraMazzo(Partita *p) {
    if (p->num_scarto > 0) {
        for (int i = 0; i < p->num_scarto; i++) p->mazzo[i] = p->scarto[i];
        p->indice_mazzo = 0;
        int conteggio = p->num_scarto;
        p->num_scarto = 0;
        MescolaArray(p->mazzo, conteggio);
    } else {
        int k = 0;
        for (int c = 0; c < 4; c++) {
            for (int v = 0; v <= 9; v++) p->mazzo[k++] = (Carta){(Colore)c, NUMERO, v};
        }
        p->indice_mazzo = 0;
        MescolaArray(p->mazzo, k);
    }
}

Carta Pesca(Partita *p) {
    if (p->indice_mazzo >= MAZZO_TOTALE) RigeneraMazzo(p);
    return p->mazzo[p->indice_mazzo++];
}

static void ImpostaMessaggio(Partita *p, const char* testo) {
    strncpy(p->messaggio, testo, 63);
    p->messaggio[63] = '\0';
    p->timer_messaggio = 3.5f;
}

static void ApplicaPenalita(Partita *p, int vittime, int quantita) {
    for (int i = 0; i < quantita; i++) {
        if (p->giocatori[vittime].num_carte < MANO_MAX) {
            p->giocatori[vittime].mano[p->giocatori[vittime].num_carte++] = Pesca(p);
        }
    }
    if (p->giocatori[vittime].num_carte > 1) p->giocatori[vittime].ha_notificato_ultima = false;
}

bool PuoGiocare(Carta c, Partita *p) {
    if (c.colore == NERO) return true;
    if (c.colore == p->colore_attuale) return true;
    if (c.tipo == NUMERO && p->cima.tipo == NUMERO && c.valore == p->cima.valore) return true;
    if (c.tipo != NUMERO && c.tipo == p->cima.tipo) return true;
    return false;
}

bool GiocatoreHaMosse(Partita *p, int id_giocatore) {
    for (int i = 0; i < p->giocatori[id_giocatore].num_carte; i++) {
        if (PuoGiocare(p->giocatori[id_giocatore].mano[i], p)) return true;
    }
    return false;
}

void InizializzaPartita(Partita *p, int num_giocatori) {
    char temp_user[12];
    bool temp_guest = p->is_guest;
    strncpy(temp_user, p->utente_corrente, 12);

    // Resetta la struttura Partita
    memset(p, 0, sizeof(Partita));

    p->pos_x_deltaplano = (float)SCHERMO_LARGHEZZA;
    p->is_guest = temp_guest;
    strncpy(p->utente_corrente, temp_user, 12);
    p->richiedi_uscita = false;

    p->num_giocatori = num_giocatori;
    p->direzione = 1;
    p->vincitore = -1;

    // --- ASSEGNAZIONE AUTOMATICA STRATEGIE IA ---
    // Il giocatore 0 è l'umano, gli altri sono bot.
    for (int j = 0; j < p->num_giocatori; j++) {
        if (j > 0) {
            // Assegnazione automatica: strategia 0 (Facile) o 1 (Difficile)
            p->giocatori[j].difficolta_ia = (j % 2 != 0) ? 1 : 0;
        }
    }

    // --- GENERAZIONE MAZZO ---
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

    // --- DISTRIBUZIONE CARTE ---
    for (int k = 0; k < 7; k++) {
        for(int j = 0; j < p->num_giocatori; j++) {
            p->giocatori[j].mano[p->giocatori[j].num_carte++] = Pesca(p);
        }
    }

    // --- CARTA INIZIALE ---
    p->cima = Pesca(p);
    while (p->cima.colore == NERO) p->cima = Pesca(p);
    p->colore_attuale = p->cima.colore;

    // --- GESTIONE EFFETTO CARTA INIZIALE ---
    if (p->cima.tipo == SALTA || p->cima.tipo == INVERTI) {
        p->turno = 1;
    } else if (p->cima.tipo == PIU_DUE) {
        ApplicaPenalita(p, 0, 2);
        p->turno = 1;
    }
}

void SalvaPartita(Partita *p) {
    if (p->is_guest || p->partita_finita) return;

    FILE *file = fopen("salvataggi_partite.dat", "ab");
    if (file != NULL) {
        RecordSalvataggio record;
        memset(&record, 0, sizeof(RecordSalvataggio));
        strncpy(record.nickname, p->utente_corrente, 11);

        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) strftime(record.data_ora, 24, "%d/%m/%Y %H:%M:%S", tm_info);

        record.dati_partita = *p;
        fwrite(&record, sizeof(RecordSalvataggio), 1, file);
        fclose(file);
    }
}

static void ProcessaCartaGiocata(Partita *p) {
    if (p->num_scarto < MAZZO_TOTALE) p->scarto[p->num_scarto++] = p->cima;
    Carta c = p->carta_animata;
    p->cima = c;
    PlayCardSound();

    int prossimo = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
    int vittima_successiva = prossimo;
    char msg[64];

    if (c.tipo == INVERTI) {
        if (p->num_giocatori == 2) {
            prossimo = p->turno;
        } else {
            p->direzione *= -1;
            prossimo = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            vittima_successiva = prossimo;
        }
        if (p->da_giocatore != 0) ImpostaMessaggio(p, "CAMBIO GIRO!");
    } else if (c.tipo == SALTA) {
        prossimo = (prossimo + p->direzione + p->num_giocatori) % p->num_giocatori;
        if (p->da_giocatore != 0) ImpostaMessaggio(p, "SALTO TURNO!");
    } else if (c.tipo == PIU_DUE) {
        ApplicaPenalita(p, vittima_successiva, 2);
        prossimo = (prossimo + p->direzione + p->num_giocatori) % p->num_giocatori;
        if (p->da_giocatore != 0) {
            sprintf(msg, "BOT %d HA LANCIATO UN +2!", p->da_giocatore);
            ImpostaMessaggio(p, msg);
        }
    }

    if (c.colore == NERO) {
        if (p->da_giocatore == 0) {
            p->scegli_colore = true;
            p->anima_foca = true;
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, vittima_successiva, 4);
                prossimo = (vittima_successiva + p->direzione + p->num_giocatori) % p->num_giocatori;
            }
            p->prossimo_turno = prossimo;
        } else {
            p->colore_attuale = GetRandomValue(0, 3);
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, vittima_successiva, 4);
                prossimo = (vittima_successiva + p->direzione + p->num_giocatori) % p->num_giocatori;
                sprintf(msg, "BOT %d: +4! COLORE: %s", p->da_giocatore, nomi_colori_ita[p->colore_attuale]);
            } else {
                sprintf(msg, "BOT %d HA SCELTO: %s", p->da_giocatore, nomi_colori_ita[p->colore_attuale]);
            }
            ImpostaMessaggio(p, msg);
            p->turno = prossimo;
        }
    } else {
        p->colore_attuale = c.colore;
        p->turno = prossimo;
    }
    p->puo_passare = false;
}

void GestisceInput(Partita *p) {
    if (p->partita_finita || p->timer_messaggio > 0 || p->anima_deltaplano) return;

    if (IsKeyPressed(KEY_ESCAPE)) {
        PlayCardSound();
        p->richiedi_uscita = true;
        return;
    }

    Vector2 m = GetMousePosition();
    if (!p->is_guest) {
        Rectangle btnSospendi = { 990, 700, 180, 70 };
        if (CheckCollisionPointRec(m, btnSospendi) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlayCardSound();
            p->richiedi_uscita = true;
            return;
        }
    }

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

    if (p->turno != 0 || p->animando) return;

    // --- CORREZIONE BUG 2: GESTIONE TURNO E PESCATA REGOLAMENTARE ---
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 160, SCHERMO_ALTEZZA/2.0f - 75, 100, 150 })) {
        if (p->puo_passare) {
            // Secondo click sul mazzo: l'utente decide di passare manualmente il turno
            p->turno = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            p->puo_passare = false;
            p->ha_pescato = false;
            PlayCardSound();
        } else if (!p->ha_pescato && p->giocatori[0].num_carte < MANO_MAX) {
            // Primo click: effettua l'unica pescata consentita per turno
            Carta pescata = Pesca(p);
            p->giocatori[0].mano[p->giocatori[0].num_carte++] = pescata;
            p->ha_pescato = true;

            if (!PuoGiocare(pescata, p)) {
                // La carta non è giocabile: l'utente è bloccato e al prossimo click passerà
                p->puo_passare = true;
            }
            PlayCardSound();
        }
    }

    int spacing = 65;
    int totalW = (p->giocatori[0].num_carte - 1) * spacing;
    int startX = SCHERMO_LARGHEZZA/2 - totalW / 2;
    for (int i = p->giocatori[0].num_carte - 1; i >= 0; i--) {
        Rectangle r = {startX + i*spacing - 50, SCHERMO_ALTEZZA - 175, 100, 150};
        if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (PuoGiocare(p->giocatori[0].mano[i], p)) {
                p->carta_animata = p->giocatori[0].mano[i];
                for (int k = i; k < p->giocatori[0].num_carte - 1; k++) p->giocatori[0].mano[k] = p->giocatori[0].mano[k+1];
                p->giocatori[0].num_carte--;
                p->animando = true;
                p->timer_anim = 0;
                p->durata_animazione = 0.3f;
                p->da_giocatore = 0;
                p->inizio_animazione = (Vector2){(float)startX + i*spacing, (float)SCHERMO_ALTEZZA - 100};
                p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};

                // Reset dei flag di pescata in quanto la mossa è stata completata con successo
                p->ha_pescato = false;
                p->puo_passare = false;
            }
            break;
        }
    }
}

static int SceltaStrategica(Partita *p, int id_giocatore) {
    Giocatore *bot = &p->giocatori[id_giocatore];
    // Priorità 1: Cerca una carta numerica (conserva le speciali)
    for (int i = 0; i < bot->num_carte; i++) {
        if (PuoGiocare(bot->mano[i], p) && bot->mano[i].tipo == NUMERO) return i;
    }
    // Priorità 2: Se non ha numeri, gioca una carta speciale
    for (int i = 0; i < bot->num_carte; i++) {
        if (PuoGiocare(bot->mano[i], p)) return i;
    }
    return -1;
}

void AggiornaPartita(Partita *p) {
    if (p->partita_finita) return;

    if (p->anima_deltaplano) {
        p->pos_x_deltaplano -= 650.0f * GetFrameTime();
        if (p->pos_x_deltaplano < -400.0f) {
            p->anima_deltaplano = false;
            p->pos_x_deltaplano = (float)SCHERMO_LARGHEZZA;
        }
        return;
    }

    if (!p->animando) {
        for(int i=0; i<p->num_giocatori; i++) {
            if (p->giocatori[i].num_carte == 1 && !p->giocatori[i].ha_notificato_ultima) {
                p->anima_deltaplano = true;
                p->giocatori[i].ha_notificato_ultima = true;
                if (i != 0) {
                    char msg[64];
                    sprintf(msg, "BOT %d HA UNA SOLA CARTA!", i);
                    ImpostaMessaggio(p, msg);
                }
                return;
            }
        }
    }

    if (p->timer_messaggio > 0) {
        p->timer_messaggio -= GetFrameTime();
        return;
    }

    if (p->animando) {
        p->timer_anim += GetFrameTime();
        float t = p->timer_anim / p->durata_animazione;
        if (t >= 1.0f) {
            p->animando = false;
            ProcessaCartaGiocata(p);
        } else {
            p->posizione_animazione.x = p->inizio_animazione.x + (p->fine_animazione.x - p->inizio_animazione.x) * t;
            p->posizione_animazione.y = p->inizio_animazione.y + (p->fine_animazione.y - p->inizio_animazione.y) * t;
        }
        return;
    }

    static float accumulatore_tempo_bot = 0.0f;
    if (p->turno != 0 && !p->scegli_colore) {
        accumulatore_tempo_bot += GetFrameTime();

        if (accumulatore_tempo_bot >= 0.8f) {
            accumulatore_tempo_bot = 0.0f;
            int b_idx = p->turno;
            int idx = -1;

            // SELEZIONE AUTOMATICA STRATEGIA
            if (p->giocatori[b_idx].difficolta_ia == 1) {
                idx = SceltaStrategica(p, b_idx);
            } else {
                for (int i = 0; i < p->giocatori[b_idx].num_carte; i++) {
                    if (PuoGiocare(p->giocatori[b_idx].mano[i], p)) { idx = i; break; }
                }
            }

            if (idx != -1) {
                p->carta_animata = p->giocatori[b_idx].mano[idx];
                for (int k = idx; k < p->giocatori[b_idx].num_carte - 1; k++) p->giocatori[b_idx].mano[k] = p->giocatori[b_idx].mano[k+1];
                p->giocatori[b_idx].num_carte--;
                p->animando = true;
                p->timer_anim = 0;
                p->durata_animazione = 0.4f;
                p->da_giocatore = b_idx;
                p->inizio_animazione = OttieniPosizioneMazzoGiocatore(b_idx, p->num_giocatori);
                p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};
            } else {
                p->giocatori[b_idx].mano[p->giocatori[b_idx].num_carte++] = Pesca(p);
                p->turno = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            }
        }
    } else {
        accumulatore_tempo_bot = 0.0f;
    }

    // --- FINE PARTITA ---
    for(int i=0; i<p->num_giocatori; i++) {
        if (p->giocatori[i].num_carte == 0) {
            p->partita_finita = true;
            p->vincitore = i;
            if (!p->suono_fine_giocato) {
                StopBackgroundMusic();
                if (i == 0) { PlayWinSound(); SalvaRisultatoPartita(p->utente_corrente, true); }
                else { PlayDefeatSound(); SalvaRisultatoPartita(p->utente_corrente, false); }
                p->suono_fine_giocato = true;
            }
            break;
        }
    }
}