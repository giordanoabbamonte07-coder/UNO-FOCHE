#ifndef ARCHIVIO_H
#define ARCHIVIO_H

#include "raylib.h"
#include "game.h"

#define MAX_SALVATAGGI_PER_UTENTE 5

// --- RISORSE CONDIVISE CON IL MAIN ---
// Usiamo i nomi esatti del tuo file archivio.c
extern Texture2D btnAvanti;
extern Texture2D btnIndietro;
extern Texture2D sfondoStatistiche; // <-- Con la 's' minuscola come nel tuo .c

extern Rectangle rectAvanti;
extern Rectangle rectIndietro;

// Carica in memoria le risorse grafiche dello storico
void InizializzaRisorseArchivio(void);

// Rilascia le textures dalla memoria VRAM alla chiusura del gioco
void ScaricaRisorseArchivio(void);

// Registra la partita corrente inserendo automaticamente data e ora correnti
void SalvaPartitaSospesa(Partita *partitaCorrente);

// Estrae i salvataggi abbinati all'utente specifico ordinandoli dal più recente
int CaricaPartiteSospeseUtente(const char *username, Partita elencoPartite[]);

// Cancella il record selezionato dal database binario dopo essere stato caricato
void EliminaPartitaDaArchivio(const char *username, int indiceDaRimuovere);

// Gestisce logica, disegno e collisioni della schermata dell'archivio basata sulla foto reale
void DisegnaArchivioStorico(const char *username, Vector2 mousePos, int *partitaSelezionataIndice);

// Ottiene il numero di vittorie e sconfitte di un determinato utente
void OttieniStatisticheUtente(const char *username, int *vittorie, int *sconfitte);

// Aggiungi questa riga in fondo ad archivio.h
void SalvaRisultatoPartita(const char *username, bool haVinto);

// Precarica i salvataggi in memoria per evitare letture continue dal disco
void PreparaArchivioStorico(const char *username);

#endif // ARCHIVIO_H