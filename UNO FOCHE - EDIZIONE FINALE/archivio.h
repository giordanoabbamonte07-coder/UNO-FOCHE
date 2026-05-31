#ifndef ARCHIVIO_H
#define ARCHIVIO_H

#include "raylib.h"
#include "game.h"

// Numero massimo di slot di salvataggio consentiti per ciascun profilo utente registrato
#define MAX_SALVATAGGI_PER_UTENTE 5

// =======================================================================================
// RISORSE GRAFICHE E DI INPUT CONDIVISE (Dichiarate esterne, definite in archivio.c)
// =======================================================================================
extern Texture2D btnAvanti;          // Texture freccia per navigare alla pagina successiva
extern Texture2D btnIndietro;        // Texture freccia per navigare alla pagina precedente
extern Texture2D sfondoStatistiche;  // Texture di sfondo nativa per la schermata dell'archivio

extern Rectangle rectAvanti;         // Area di collisione del mouse per il pulsante "Avanti"
extern Rectangle rectIndietro;       // Area di collisione del mouse per il pulsante "Indietro"

// =======================================================================================
// PROTOTIPI DELLE FUNZIONE DI GESTIONE RISORSE E PERSISTENZA
// =======================================================================================

/**
 * @brief Carica in memoria VRAM le texture e i file grafici necessari alla schermata storico.
 * Da invocare una sola volta all'avvio del programma principale.
 */
void InizializzaRisorseArchivio(void);

/**
 * @brief Rilascia le risorse grafiche precedentemente caricate nella scheda video.
 * Da invocare tassativamente prima della chiusura definitiva dell'eseguibile per prevenire memory leak.
 */
void ScaricaRisorseArchivio(void);

/**
 * @brief Cattura lo stato corrente di gioco e lo scrive nel file binario, timbrandolo con data e ora reali.
 * @param partitaCorrente Puntatore alla struttura Partita da congelare su disco.
 */
void SalvaPartitaSospesa(Partita *partitaCorrente);

/**
 * @brief Scansiona il file binario ed estrae le partite sospese appartenenti a un utente, memorizzandole in un array.
 * @param username Stringa contenente il nickname dell'utente corrente.
 * @param elencoPartite Array di output in cui verranno depositati i dati delle partite caricate.
 * @return int Il numero totale di record validi trovati e caricati in memoria per quell'utente.
 */
int CaricaPartiteSospeseUtente(const char *username, Partita elencoPartite[]);

/**
 * @brief Rimuove definitivamente una specifica partita dall'archivio (es: dopo che è stata ripresa o sovrascritta).
 * @param username Stringa contenente il nickname dell'utente proprietario del salvataggio.
 * @param indiceDaRimuovere Posizione numerica del record da eliminare all'interno del database.
 */
void EliminaPartitaDaArchivio(const char *username, int indiceDaRimuovere);

/**
 * @brief Monitora l'input del mouse e renderizza l'interfaccia dello storico con impaginazione dinamica.
 * @param username Stringa contenente il nickname dell'utente di cui mostrare i salvataggi.
 * @param mousePos Coordinate cartesiane X,Y correnti del cursore del mouse ottenute da Raylib.
 * @param partitaSelezionataIndice Puntatore di output: assumerà l'indice della partita cliccata dall'utente (oppure -1).
 */
void DisegnaArchivioStorico(const char *username, Vector2 mousePos, int *partitaSelezionataIndice);

/**
 * @brief Calcola il totale storico di vittorie e sconfitte accumulate dall'utente dall'inizio della sua iscrizione.
 * Viene usata per popolare la scheda riassuntiva del profilo a schermo.
 * @param username Stringa contenente il nickname dell'utente da analizzare.
 * @param vittorie Puntatore alla variabile intera in cui salvare il conteggio delle partite vinte.
 * @param sconfitte Puntatore alla variabile intera in cui salvare il conteggio delle partite perse.
 */
void OttieniStatisticheUtente(const char *username, int *vittorie, int *sconfitte);

/**
 * @brief Incrementa permanentemente il contatore delle vittorie o delle sconfitte al termine di un match.
 * Invocata direttamente all'interno della routine di game over in game.c.
 * @param username Stringa contenente il nickname del giocatore umano.
 * @param haVinto Flag booleano: inserire 'true' se ha vinto l'umano, 'false' se ha vinto un bot.
 */
void SalvaRisultatoPartita(const char *username, bool haVinto);

/**
 * @brief Effettua un pre-caricamento iniziale dei file di salvataggio nella memoria RAM del computer.
 * Ottimizza le prestazioni riducendo drasticamente le letture sincrone dal disco fisso durante il ciclo di rendering.
 * @param username Stringa contenente il nickname dell'utente di cui pre-caricare lo storico.
 */
void PreparaArchivioStorico(const char *username);

#endif // ARCHIVIO_H