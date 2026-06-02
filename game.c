#include "game.h"
#include "audio.h"
#include "archivio.h" // Incluso per richiamare la persistenza delle statistiche (SalvaRisultatoPartita)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Array globale di stringhe per la traduzione a schermo dei colori di gioco
const char* nomi_colori_ita[] = {"ROSSO", "GIALLO", "VERDE", "BLU", "NERO"};

/**
 * Calcola l'origine geometrica X,Y dei mazzi di ciascun giocatore per ancorare le animazioni delle carte
 */
Vector2 OttieniPosizioneMazzoGiocatore(int index, int num_giocatori) {
    if (index == 0) return (Vector2){ 600.0f, 700.0f }; // Giocatore Umano (In basso al centro)

    // Configurazione della stanza in modalità 1vs1 (2 Giocatori totali)
    if (num_giocatori == 2) {
        if (index == 1) return (Vector2){ 600.0f, 115.0f }; // Bot avversario posizionato in Alto al centro
    } else {
        // Configurazione della stanza in modalità Tavolo Completo (4 Giocatori totali)
        if (index == 1) return (Vector2){ 120.0f, 400.0f };  // Bot 1: Posizionato a Sinistra
        if (index == 2) return (Vector2){ 600.0f, 115.0f };  // Bot 2: Posizionato in Alto
        if (index == 3) return (Vector2){ 1080.0f, 400.0f }; // Bot 3: Posizionato a Destra
    }
    return (Vector2){ 600.0f, 400.0f }; // Fallback di sicurezza al centro esatto dello schermo
}

/**
 * Algoritmo di rimescolamento (basato sul principio di Fisher-Yates semplificato tramite Raylib)
 */
static void MescolaArray(Carta *array, int n) {
    for (int i = 0; i < n; i++) {
        int r = GetRandomValue(0, n - 1); // Estrae un indice casuale all'interno del mazzo
        Carta tmp = array[i];            // Scambio dei puntatori di memoria dei due oggetti Carta
        array[i] = array[r];
        array[r] = tmp;
    }
}

/**
 * Genera sequenzialmente le 108 carte del mazzo regolamentare di UNO
 */
static int CreaMazzoCompleto(Carta *mazzo) {
    int i = 0;
    // Scansione dei 4 colori standard (Rosso = 0, Giallo = 1, Verde = 2, Blu = 3)
    for (int c = 0; c < 4; c++) {
        // Ogni colore possiede una singola carta con valore numerico 0
        mazzo[i++] = (Carta){(Colore)c, NUMERO, 0};
        // Ogni colore possiede due copie delle carte numeriche da 1 a 9
        for (int v = 1; v <= 9; v++) {
            mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
            mazzo[i++] = (Carta){(Colore)c, NUMERO, v};
        }
        // Ogni colore possiede due copie di ciascuna carta d'effetto (Salto, Inverti, +2)
        for (int v = 0; v < 2; v++) {
            mazzo[i++] = (Carta){(Colore)c, SALTA, 0};
            mazzo[i++] = (Carta){(Colore)c, INVERTI, 0};
            mazzo[i++] = (Carta){(Colore)c, PIU_DUE, 0};
        }
    }
    // Inserimento delle 8 carte speciali nere (4 Cambia Colore e 4 Pesca Quattro "+4")
    for (int j = 0; j < 4; j++) {
        mazzo[i++] = (Carta){NERO, PIU_QUATTRO, 0};
        mazzo[i++] = (Carta){NERO, CAMBIA_COLORE, 0};
    }
    return i; // Restituisce il numero esatto di carte caricate (108)
}

/**
 * Rigenera il mazzo prendendo le carte dalla pila degli scarti qualora dovesse esaurirsi durante il gioco
 */
static void RigeneraMazzo(Partita *p) {
    if (p->num_scarto > 0) {
        // Travasa l'intero array delle carte scartate all'interno del mazzo di pesca
        for (int i = 0; i < p->num_scarto; i++) p->mazzo[i] = p->scarto[i];
        p->indice_mazzo = 0;              // Resetta l'indice di lettura delle pescate
        p->carte_mazzo = p->num_scarto;   // Il contatore delle carte disponibili assume la dimensione degli scarti
        p->num_scarto = 0;                // Svuota la pila degli scarti sul tavolo
        MescolaArray(p->mazzo, p->carte_mazzo); // Mescola il mazzo rigenerato
    } else {
        // Procedura di emergenza se anche la pila degli scarti è vuota: ricrea un mazzo nuovo da zero
        p->carte_mazzo = CreaMazzoCompleto(p->mazzo);
        p->indice_mazzo = 0;
        MescolaArray(p->mazzo, p->carte_mazzo);
    }
}

/**
 * Estrae e restituisce la prima carta utile in cima al mazzo di pesca
 */
Carta Pesca(Partita *p) {
    // Se il mazzo è esaurito o l'indice ha superato il limite massimo, avvia la rigenerazione
    if (p->carte_mazzo <= 0 || p->indice_mazzo >= p->carte_mazzo) RigeneraMazzo(p);
    return p->mazzo[p->indice_mazzo++]; // Restituisce la carta incrementando l'indice per la lettura successiva
}

/**
 * Formatta e attiva il banner di testo dei messaggi di sistema a schermo
 */
static void ImpostaMessaggio(Partita *p, const char* testo) {
    strncpy(p->messaggio, testo, 63); // Copia in sicurezza nel buffer della stringa (max 63 caratteri)
    p->messaggio[63] = '\0';          // Forza la chiusura della stringa per evitare sovrascritture di memoria
    p->timer_messaggio = 3.5f;        // Imposta la durata di persistenza del testo a 3.5 secondi
}

/**
 * Distribuisce forzatamente un quantitativo di carte a un giocatore come penalità (+2 o +4)
 */
static void ApplicaPenalita(Partita *p, int vittime, int quantita) {
    for (int i = 0; i < quantita; i++) {
        // Inserisce una nuova carta in mano alla vittima solo se non ha superato la capacità massima dell'array
        if (p->giocatori[vittime].num_carte < MANO_MAX) {
            p->giocatori[vittime].mano[p->giocatori[vittime].num_carte++] = Pesca(p);
        }
    }
    // Avendo pescato delle carte, il giocatore perde l'eventuale notifica di ultima carta ("UNO") precedentemente acquisita
    if (p->giocatori[vittime].num_carte > 1) p->giocatori[vittime].ha_notificato_ultima = false;
}

/**
 * Verifica se una determinata carta della mano è compatibile con quella presente in cima al tavolo
 */
bool PuoGiocare(Carta c, Partita *p) {
    if (c.colore == NERO) return true; // Le carte jolly nere possono essere giocate sopra qualsiasi carta
    if (c.colore == p->colore_attuale) return true; // Corrispondenza per colore
    // Se è un numero, controlla se coincide il valore numerico con la carta a terra
    if (c.tipo == NUMERO && p->cima.tipo == NUMERO && c.valore == p->cima.valore) return true;
    // Se è una speciale (Salto, Inverti, +2), controlla se coincide la stessa tipologia di effetto
    if (c.tipo != NUMERO && c.tipo == p->cima.tipo) return true;
    return false; // Carta non compatibile secondo il regolamento
}

/**
 * Effettua un controllo preventivo per capire se il giocatore possiede almeno una mossa valida in mano
 */
bool GiocatoreHaMosse(Partita *p, int id_giocatore) {
    for (int i = 0; i < p->giocatori[id_giocatore].num_carte; i++) {
        if (PuoGiocare(p->giocatori[id_giocatore].mano[i], p)) return true; // Trovata carta giocabile
    }
    return false; // Nessuna carta valida, il giocatore sarà obbligato a pescare
}

/**
 * Setup iniziale della partita: resetta i dati, genera il mazzo e distribuisce 7 carte a testa
 */
void InizializzaPartita(Partita *p, int num_giocatori) {
    char temp_user[12];
    bool temp_guest = p->is_guest;
    strncpy(temp_user, p->utente_corrente, 12); // Preserva lo username dell'account per il salvataggio statistico

    // Svuota completamente l'area di memoria occupata dalla struttura Partita per azzerare residui di vecchi match
    memset(p, 0, sizeof(Partita));

    // Impostazioni di default dei parametri grafici e di sessione
    p->pos_x_deltaplano = (float)SCHERMO_LARGHEZZA;
    p->is_guest = temp_guest;
    strncpy(p->utente_corrente, temp_user, 12);
    p->richiedi_uscita = false;

    p->num_giocatori = num_giocatori;
    p->direzione = 1;  // Giro iniziale impostato in senso orario (1)
    p->vincitore = -1; // -1 indica che la partita è ancora in corso e non vi sono vincitori

    // ASGNAZIONE DELLE STRATEGIE COMPORTAMENTALI DEI BOT (INTELLIGENZA ARTIFICIALE)
    for (int j = 0; j < p->num_giocatori; j++) {
        if (j > 0) {
            // Assegnazione alternata: i bot dispari usano la strategia 1 (Difficile), i pari la 0 (Facile)
            p->giocatori[j].difficolta_ia = (j % 2 != 0) ? 1 : 0;
        }
    }

    // Costruzione e rimescolamento del mazzo iniziale di gioco
    p->carte_mazzo = CreaMazzoCompleto(p->mazzo);
    MescolaArray(p->mazzo, p->carte_mazzo);

    // Distribuzione ciclica regolamentare di 7 carte per ciascun partecipante al tavolo
    for (int k = 0; k < 7; k++) {
        for(int j = 0; j < p->num_giocatori; j++) {
            p->giocatori[j].mano[p->giocatori[j].num_carte++] = Pesca(p);
        }
    }

    // Estrazione della prima carta del tavolo per avviare la pila degli scarti
    p->cima = Pesca(p);
    // Se la prima carta estratta è un jolly nero, continua a pescare finché non esce una carta colorata
    while (p->cima.colore == NERO) p->cima = Pesca(p);
    p->colore_attuale = p->cima.colore; // Sincronizza il colore attivo con la carta scoperta

    // VALUTAZIONE EFFETTO CARTA DI APERTURA DEL MATCH
    if (p->cima.tipo == SALTA || p->cima.tipo == INVERTI) {
        p->turno = 1; // Salta il giocatore umano (Indice 0) trasferendo direttamente il turno al Bot 1
    } else if (p->cima.tipo == PIU_DUE) {
        ApplicaPenalita(p, 0, 2); // Il giocatore umano subisce la penalità di inizio partita prendendo 2 carte
        p->turno = 1;             // Il turno passa al Bot 1
    }
}

/**
 * Salva lo stato interno della partita corrente all'interno del file binario per la ripresa successiva
 */
void SalvaPartita(Partita *p) {
    // I salvataggi sono disabilitati se l'utente gioca come Ospite o se il match è già terminato
    if (p->is_guest || p->partita_finita) return;

    // Apre il file in modalità scrittura binaria aggiuntiva (append binario)
    FILE *file = fopen("salvataggi_partite.dat", "ab");
    if (file != NULL) {
        RecordSalvataggio record;
        memset(&record, 0, sizeof(RecordSalvataggio));
        strncpy(record.nickname, p->utente_corrente, 11);

        // Cattura l'istante di tempo esatto del salvataggio
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        if (tm_info) strftime(record.data_ora, 24, "%d/%m/%Y %H:%M:%S", tm_info);

        record.dati_partita = *p; // Esegue una copia bit a bit (deep dump) dell'intera struttura di gioco
        fwrite(&record, sizeof(RecordSalvataggio), 1, file); // Scrive il blocco sul disco fisso
        fclose(file);
    }
}

/**
 * Elabora le conseguenze e gli effetti speciali applicati dalle carte dopo il termine della loro animazione
 */
static void ProcessaCartaGiocata(Partita *p) {
    // Sposta la vecchia carta in cima all'interno dello storico della pila degli scarti
    if (p->num_scarto < MAZZO_TOTALE) p->scarto[p->num_scarto++] = p->cima;
    Carta c = p->carta_animata;
    p->cima = c;         // Sovrascrive la cima del tavolo con la carta appena atterrata
    PlayCardSound();     // Riproduce l'effetto sonoro del piazzamento carta

    // Calcolo modulare del turno successivo standard in base alla direzione di rotazione attuale
    int prossimo = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
    int vittima_successiva = prossimo;
    char msg[64];

    // GESTIONE LOGICA DELLE CARTE SPECIALI COLORATE
    if (c.tipo == INVERTI) {
        if (p->num_giocatori == 2) {
            prossimo = p->turno; // Nel gioco a due giocatori, la carta Inverti assume la valenza di un Salta Turno
        } else {
            p->direzione *= -1;  // Inverte l'ordine algebrico del giro di turni (da 1 a -1 o viceversa)
            prossimo = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            vittima_successiva = prossimo;
        }
        if (p->da_giocatore != 0) ImpostaMessaggio(p, "CAMBIO GIRO!");
    } else if (c.tipo == SALTA) {
        // Applica un ulteriore sbalzo saltando l'avversario immediatamente consecutivo
        prossimo = (prossimo + p->direzione + p->num_giocatori) % p->num_giocatori;
        if (p->da_giocatore != 0) ImpostaMessaggio(p, "SALTO TURNO!");
    } else if (c.tipo == PIU_DUE) {
        ApplicaPenalita(p, vittima_successiva, 2); // Assegna 2 carte forzate al bersaglio di turno
        prossimo = (prossimo + p->direzione + p->num_giocatori) % p->num_giocatori; // Salta il turno della vittima
        if (p->da_giocatore != 0) {
            sprintf(msg, "BOT %d HA LANCIATO UN +2!", p->da_giocatore);
            ImpostaMessaggio(p, msg);
        }
    }

    // GESTIONE LOGICA DELLE CARTE SPECIALI JOLLY NERE
    if (c.colore == NERO) {
        if (p->da_giocatore == 0) {
            // Se la carta è stata scagliata dall'utente umano, blocca il gioco e attiva i pannelli di scelta colore
            p->scegli_colore = true;
            p->anima_foca = true; // Mostra il pop-up grafico della foca centralmente sopra la scelta
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, vittima_successiva, 4); // Assegna le 4 carte di penalità
                prossimo = (vittima_successiva + p->direzione + p->num_giocatori) % p->num_giocatori; // Salta il turno
            }
            p->prossimo_turno = prossimo; // Memorizza temporaneamente lo stato del turno per riprenderlo dopo la scelta
        } else {
            // Se la carta nera è mossa da un Bot, seleziona un colore in modo casuale
            p->colore_attuale = GetRandomValue(0, 3);
            if (c.tipo == PIU_QUATTRO) {
                ApplicaPenalita(p, vittima_successiva, 4);
                prossimo = (vittima_successiva + p->direzione + p->num_giocatori) % p->num_giocatori;
                sprintf(msg, "BOT %d: +4! COLORE: %s", p->da_giocatore, nomi_colori_ita[p->colore_attuale]);
            } else {
                sprintf(msg, "BOT %d HA SCELTO: %s", p->da_giocatore, nomi_colori_ita[p->colore_attuale]);
            }
            ImpostaMessaggio(p, msg);
            p->turno = prossimo; // Il bot assegna direttamente il turno senza fasi di attesa
        }
    } else {
        p->colore_attuale = c.colore; // Aggiorna il colore ufficiale del tavolo con quello della carta giocata
        p->turno = prossimo;          // Passa il controllo del gioco al giocatore successivo
    }
    p->puo_passare = false; // Resetta i flag di controllo dell'input per il turno entrante
}

/**
 * Intercetta l'input del mouse e della tastiera per il controllo del giocatore umano (ID 0)
 */
void GestisceInput(Partita *p) {
    // Interrompe l'ascolto se la partita è finita o se sono in corso animazioni e scritte bloccanti
    if (p->partita_finita || p->timer_messaggio > 0 || p->anima_deltaplano) return;

    // Pressione del tasto ESCAPE: richiede l'abbandono immediato del match per ritornare ai menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        PlayCardSound();
        p->richiedi_uscita = true;
        return;
    }

    Vector2 m = GetMousePosition();
    // Se l'utente non è un ospite, controlla i click sulla hit-box del pulsante grafico "Sospendi"
    if (!p->is_guest) {
        Rectangle btnSospendi = { 990, 700, 180, 70 };
        if (CheckCollisionPointRec(m, btnSospendi) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlayCardSound();
            p->richiedi_uscita = true;
            return;
        }
    }

    // SELEZIONE DEL COLORE (POP-UP ATTIVO PER CARTE JOLLY)
    if (p->scegli_colore) {
        for (int i = 0; i < 4; i++) {
            // Calcola la posizione rettangolare dei 4 bottoni dei colori affiancati orizzontalmente
            Rectangle r = { SCHERMO_LARGHEZZA/2.0f - 160 + i*85, SCHERMO_ALTEZZA/2.0f - 40, 75, 75 };
            if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                p->colore_attuale = (Colore)i; // Applica il colore selezionato al tavolo
                p->scegli_colore = false;     // Chiude il pannello modale
                p->anima_foca = false;        // Disattiva il popup della foca animata
                p->turno = p->prossimo_turno;  // Ripristina il flusso dei turni calcolato in precedenza
            }
        }
        return;
    }

    // Se non è il turno dell'utente umano (0) o se c'è un'animazione in corso, rifiuta qualsiasi interazione
    if (p->turno != 0 || p->animando) return;

    // GESTIONE REQUISITI DI PESCATA REGOLAMENTARE DA MAZZO
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, (Rectangle){ SCHERMO_LARGHEZZA/2.0f - 160, SCHERMO_ALTEZZA/2.0f - 75, 100, 150 })) {
        if (p->puo_passare) {
            // SECONDO CLICK SUL MAZZO: L'utente ha già pescato una carta utile ma decide liberamente di non giocarla e passare
            p->turno = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            p->puo_passare = false;
            p->ha_pescato = false;
            PlayCardSound();
        } else if (!p->ha_pescato && p->giocatori[0].num_carte < MANO_MAX) {
            // PRIMO CLICK SUL MAZZO: Pesca l'unica carta concessa per questo turno
            Carta pescata = Pesca(p);
            p->giocatori[0].mano[p->giocatori[0].num_carte++] = pescata;
            p->ha_pescato = true;

            if (PuoGiocare(pescata, p)) {
                // Se la carta appena pescata è compatibile con il tavolo, sblocca il diritto di decidere se giocarla o passare
                p->puo_passare = true;
            } else {
                // Se non è accoppiabile, il turno viene saltato e trasferito d'ufficio all'avversario
                p->turno = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
                p->ha_pescato = false;
                p->puo_passare = false;
            }
            PlayCardSound();
        }
    }

    // INTERCETTAZIONE DEI CLICK SULLE CARTE DELLA PROPRIA MANO
    int spacing = 65; // Spazio di sovrapposizione laterale tra le carte grafiche per compattarle
    int totalW = (p->giocatori[0].num_carte - 1) * spacing;
    int startX = SCHERMO_LARGHEZZA/2 - totalW / 2; // Calcola il punto di origine X per mantenere l'intera mano centrata

    // Scansiona l'array dall'ultima carta (quella visivamente in primo piano) fino alla prima per rispettare i livelli di rendering
    for (int i = p->giocatori[0].num_carte - 1; i >= 0; i--) {
        Rectangle r = {startX + i*spacing - 50, SCHERMO_ALTEZZA - 175, 100, 150};
        if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (PuoGiocare(p->giocatori[0].mano[i], p)) {
                // Configura e innesca la struttura dei vettori dell'animazione
                p->carta_animata = p->giocatori[0].mano[i];
                // Compatta l'array della mano eliminando l'elemento giocato tramite scorrimento a sinistra delle carte successive
                for (int k = i; k < p->giocatori[0].num_carte - 1; k++) p->giocatori[0].mano[k] = p->giocatori[0].mano[k+1];
                p->giocatori[0].num_carte--; // Decrementa il numero di carte reali dell'utente

                p->animando = true;
                p->timer_anim = 0;
                p->durata_animazione = 0.3f; // Durata dello spostamento impostata a 0.3 secondi
                p->da_giocatore = 0;
                p->inizio_animazione = (Vector2){(float)startX + i*spacing, (float)SCHERMO_ALTEZZA - 100};
                p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};

                // Resetta i vincoli di pescata del turno essendosi conclusa la giocata con successo
                p->ha_pescato = false;
                p->puo_passare = false;
            }
            break; // Interrompe il ciclo per evitare che il click si propaghi alle carte sottostanti
        }
    }
}

/**
 * STRATEGIA IA AVANZATA: Analizza la mano del Bot restituendo la carta migliore da giocare per preservare i Jolly
 */
static int SceltaStrategica(Partita *p, int id_giocatore) {
    Giocatore *bot = &p->giocatori[id_giocatore];
    // PRIORITÀ 1: Cerca e gioca prima le carte numeriche ordinarie per conservare i poteri speciali per le fasi calde
    for (int i = 0; i < bot->num_carte; i++) {
        if (PuoGiocare(bot->mano[i], p) && bot->mano[i].tipo == NUMERO) return i;
    }
    // PRIORITÀ 2: Se non ci sono numeri compatibili, sacrifica e gioca la prima carta ad effetto utile in mano
    for (int i = 0; i < bot->num_carte; i++) {
        if (PuoGiocare(bot->mano[i], p)) return i;
    }
    return -1; // Nessuna carta compatibile trovata in mano
}

/**
 * Aggiorna lo stato logico della partita, muove i timer di gioco ed esegue i ragionamenti dei bot artificiali
 */
void AggiornaPartita(Partita *p) {
    if (p->partita_finita) return; // Arresta i calcoli logici se il match è terminato

    // GESTIONE ANIMAZIONE DELL'EASTER EGG DEL DELTAPLANO ("UNO" ALERT)
    if (p->anima_deltaplano) {
        p->pos_x_deltaplano -= 650.0f * GetFrameTime(); // Muove il deltaplano verso sinistra in modo lineare uniforme
        if (p->pos_x_deltaplano < -400.0f) {
            p->anima_deltaplano = false; // Disattiva l'animazione quando esce completamente dai confini visivi
            p->pos_x_deltaplano = (float)SCHERMO_LARGHEZZA; // Riposiziona l'origine del volo sul margine destro
        }
        return; // Durante il volo del deltaplano la logica dei turni viene temporaneamente congelata
    }

    // MONITORAGGIO DEL REGOLAMENTO DI NOTIFICA DI ULTIMA CARTA ("UNO!")
    if (!p->animando) {
        for(int i=0; i<p->num_giocatori; i++) {
            // Se un giocatore scende a 1 carta e non è ancora stato segnalato, fa scattare l'evento del deltaplano
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

    // Decrementa il timer di persistenza dei testi di notifica a schermo
    if (p->timer_messaggio > 0) {
        p->timer_messaggio -= GetFrameTime();
        return;
    }

    // CALCOLO DELLA TRAIETTORIA DELLA CARTA IN ANIMAZIONE (INTERPOLAZIONE PARAMETRICA)
    if (p->animando) {
        p->timer_anim += GetFrameTime();
        float t = p->timer_anim / p->durata_animazione; // Calcola il coefficiente di avanzamento percentuale (0.0 -> 1.0)
        if (t >= 1.0f) {
            p->animando = false;
            ProcessaCartaGiocata(p); // L'animazione si è conclusa, applica gli effetti della carta sul tavolo
        } else {
            // Formula matematica del LERP per determinare le coordinate correnti (X,Y) della carta lungo il vettore di lancio
            p->posizione_animazione.x = p->inizio_animazione.x + (p->fine_animazione.x - p->inizio_animazione.x) * t;
            p->posizione_animazione.y = p->inizio_animazione.y + (p->fine_animazione.y - p->inizio_animazione.y) * t;
        }
        return;
    }

    // GESTIONE DEI LOGIC TREE E DEL CALCOLO TEMPORIZZATO DEI BOT (ID != 0)
    static float accumulatore_tempo_bot = 0.0f;
    if (p->turno != 0 && !p->scegli_colore) {
        accumulatore_tempo_bot += GetFrameTime(); // Incrementa il tempo di "riflessione" artificiale del bot

        if (accumulatore_tempo_bot >= 0.8f) { // Il bot compie la propria mossa dopo 0.8 secondi di attesa simulata
            accumulatore_tempo_bot = 0.0f;
            int b_idx = p->turno;
            int idx = -1;

            // RAMIFICAZIONE COMPORTAMENTALE IN BASE ALLA DIFFICOLTÀ ASSEGNATA
            if (p->giocatori[b_idx].difficolta_ia == 1) {
                idx = SceltaStrategica(p, b_idx); // Intelligenza Difficile (Usa le regole di priorità)
            } else {
                // Intelligenza Facile: Scansiona la mano e gioca la primissima carta valida che trova senza alcuna strategia
                for (int i = 0; i < p->giocatori[b_idx].num_carte; i++) {
                    if (PuoGiocare(p->giocatori[b_idx].mano[i], p)) { idx = i; break; }
                }
            }

            if (idx != -1) {
                // Il bot possiede una mossa utile: configura l'innesco dell'animazione di lancio verso il centro
                p->carta_animata = p->giocatori[b_idx].mano[idx];
                for (int k = idx; k < p->giocatori[b_idx].num_carte - 1; k++) p->giocatori[b_idx].mano[k] = p->giocatori[b_idx].mano[k+1];
                p->giocatori[b_idx].num_carte--;

                p->animando = true;
                p->timer_anim = 0;
                p->durata_animazione = 0.4f;
                p->da_giocatore = b_idx;
                p->inizio_animazione = OttieniPosizioneMazzoGiocatore(b_idx, p->num_giocatori); // Origine corretta dal mazzo del bot
                p->fine_animazione = (Vector2){SCHERMO_LARGHEZZA/2.0f + 110.0f, SCHERMO_ALTEZZA/2.0f};
            } else if (p->giocatori[b_idx].num_carte < MANO_MAX) {
                // Il bot non ha carte utili: pesca obbligatoriamente un elemento dal mazzo e passa il turno al successivo
                p->giocatori[b_idx].mano[p->giocatori[b_idx].num_carte++] = Pesca(p);
                p->turno = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            } else {
                // Raggiunto il limite massimo di carte in mano (MANO_MAX): passa il turno d'ufficio senza pescare
                p->turno = (p->turno + p->direzione + p->num_giocatori) % p->num_giocatori;
            }
        }
    } else {
        accumulatore_tempo_bot = 0.0f; // Resetta l'accumulatore quando il turno ritorna al giocatore umano
    }

    // CONTROLLO DELLE CONDIZIONI DI VITTORIA O SCONFITTA ASSOLUTA (MANO SVUOTATA)
    for(int i=0; i<p->num_giocatori; i++) {
        if (p->giocatori[i].num_carte == 0) {
            p->partita_finita = true;
            p->vincitore = i; // Registra l'indice del trionfatore del match

            if (!p->suono_fine_giocato) {
                StopBackgroundMusic(); // Interrompe il loop audio di sottofondo della partita
                if (i == 0) {
                    // Vittoria dell'utente umano: avvia l'audio dedicato e aggiorna i record binari con esito POSITIVO (true)
                    PlayWinSound();
                    SalvaRisultatoPartita(p->utente_corrente, true);
                } else {
                    // Vittoria di un bot (Sconfitta dell'umano): traccia l'esito NEGATIVO (false) sul file delle statistiche
                    PlayDefeatSound();
                    SalvaRisultatoPartita(p->utente_corrente, false);
                }
                p->suono_fine_giocato = true; // Impedisce la riproduzione continua dell'audio nei fotogrammi successivi
            }
            break;
        }
    }
}