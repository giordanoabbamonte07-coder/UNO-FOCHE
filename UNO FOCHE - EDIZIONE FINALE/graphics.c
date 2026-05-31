#include "graphics.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Array globale di stringhe per mappare l'enumerazione 'Colore' nei nomi delle cartelle/file degli asset
const char* nomi_colore_gfx[] = {"rosso", "giallo", "verde", "blu", "nero"};

// Texture di sicurezza generata a runtime in caso di fallimento del caricamento di una carta reale
static Texture2D fallback;

/**
 * Genera via software una texture grigia temporanea da usare se un file PNG sul disco è corrotto o mancante
 */
Texture2D CreaFallback(void) {
    // Genera un'immagine in RAM di 60x90 pixel colorata di grigio scuro
    Image img = GenImageColor(60, 90, DARKGRAY);
    // Trasferisce i pixel dall'immagine in RAM alla memoria video (VRAM) sotto forma di Texture
    Texture2D t = LoadTextureFromImage(img);
    // Libera immediatamente la memoria di sistema (RAM) occupata dall'immagine temporanea
    UnloadImage(img);
    return t;
}

/**
 * Carica una texture in sicurezza verificando preventivamente la reale esistenza del file sul disco
 */
Texture2D CaricaSicuro(const char *path) {
    // Se il file esiste sul percorso indicato, lo carica normalmente in VRAM
    if (FileExists(path)) return LoadTexture(path);
    // Segnala l'errore sulla console di debug se l'asset è mancante
    printf("ATTENZIONE: File mancante -> %s\n", path);
    // Restituisce la texture grigia di fallback per evitare che il gioco vada in crash per puntatore nullo
    return fallback;
}

/**
 * Funzione di disegno standard per renderizzare una singola carta a schermo applicando rotazioni e centrature
 */
void DisegnaCarta(Texture2D t, float cx, float cy, float rotazione) {
    // Rettangolo sorgente: campiona l'intera dimensione della texture originale
    Rectangle source = { 0, 0, (float)t.width, (float)t.height };
    // Rettangolo di destinazione: definisce la posizione X,Y e le dimensioni di visualizzazione standardizzate
    Rectangle dest = { cx, cy, (float)CARTA_LARGHEZZA, (float)CARTA_ALTEZZA };
    // Imposta il perno di rotazione e traslazione esattamente al centro geometrico della carta
    Vector2 origin = { CARTA_LARGHEZZA / 2.0f, CARTA_ALTEZZA / 2.0f };
    // Renderizza la carta applicando i parametri geometrici e mantenendo la tinta bianca originale
    DrawTexturePro(t, source, dest, origin, rotazione, WHITE);
}

/**
 * Disegna un rettangolo colorato leggermente più grande dietro una carta per evidenziarla (es: per mosse valide)
 */
void EvidenziaCarta(float cx, float cy, float rotazione, Color colore) {
    // Crea un rettangolo maggiorato di 12 pixel rispetto alla dimensione standard della carta
    Rectangle rec = { cx, cy, CARTA_LARGHEZZA + 12, CARTA_ALTEZZA + 12 };
    // Centra il perno del rettangolo di selezione sul centro geometrico della carta sottostante
    Vector2 origin = { (CARTA_LARGHEZZA + 12) / 2.0f, (CARTA_ALTEZZA + 12) / 2.0f };
    // Renderizza il rettangolo pieno ruotato che fungerà da alone o contorno luminoso
    DrawRectanglePro(rec, origin, rotazione, colore);
}

/**
 * Disegna un testo con un effetto finto-grassetto / bordo nero simulando uno scostamento di pixel sui quattro lati
 */
void DisegnaTestoPixelGrassetto(const char* testo, int x, int y, int fontSize, Color baseColor) {
    // Determina lo spessore dell'ombra/bordo in base alla dimensione del carattere scelto
    int thick = (fontSize > 40) ? 3 : 2;
    // Ciclo annidato per disegnare il testo in nero tutto intorno alla coordinata centrale (effetto contorno)
    for (int i = -thick; i <= thick; i++) {
        for (int j = -thick; j <= thick; j++) {
            // Salta il centro perfetto per non sovrascrivere preventivamente il testo colorato
            if (i != 0 || j != 0) DrawText(testo, x + i, y + j, fontSize, BLACK);
        }
    }
    // Disegna infine il testo principale in primo piano sopra lo strato d'ombra nero
    DrawText(testo, x, y, fontSize, baseColor);
}

/**
 * Funzione di lookup che estrae la texture corretta dalla matrice tridimensionale della struttura grafica
 */
Texture2D OttieniCarta(Grafica *gfx, Carta c) {
    // Se la carta è numerica, accede usando il colore, il tipo NUMERO e lo slot del valore (0-9)
    if (c.tipo == NUMERO) return gfx->carte[c.colore][NUMERO][c.valore];
    // Se è una carta ad effetto (Salta, Inverti, +2, Jolly), legge la texture stoccata all'indice zero
    return gfx->carte[c.colore][c.tipo][0];
}

/**
 * Alloca la texture di fallback e carica l'intero set di immagini di gioco, sfondi, pulsanti e animazioni
 */
void InizializzaGrafica(Grafica *gfx) {
    fallback = CreaFallback(); // Alloca la texture di emergenza
    // Caricamento controllato degli asset d'interfaccia e multimediali
    gfx->sfondo = CaricaSicuro("Sfondo/sfondo_partita.png");
    gfx->foca = CaricaSicuro("Animazioni/foca_popup.png");
    gfx->deltaplano = CaricaSicuro("Animazioni/popup_deltaplano.png");
    gfx->win_img = CaricaSicuro("Animazioni/win.png");
    gfx->defeat_img = CaricaSicuro("Animazioni/defeat.png");
    gfx->retro = CaricaSicuro("Carte/back_card.png");
    gfx->btn_sospendi = CaricaSicuro("Pulsanti/sospendi.png");

    // Ciclo per caricare le carte dei 4 colori standard (Rosso, Giallo, Verde, Blu)
    for (int c = 0; c < 4; c++) {
        // Carica le carte numeriche da 0 a 9 per il colore corrente
        for (int v = 0; v <= 9; v++) {
            char p[128];
            sprintf(p, "Carte/%s_%d.png", nomi_colore_gfx[c], v);
            gfx->carte[c][NUMERO][v] = CaricaSicuro(p);
        }
        // Stringhe di buffer per comporre i percorsi dei file delle carte speciali colorate
        char p1[128], p2[128], p3[128];
        sprintf(p1, "Carte/%s_stop.png", nomi_colore_gfx[c]);
        gfx->carte[c][SALTA][0] = CaricaSicuro(p1);
        sprintf(p2, "Carte/%s_rev.png", nomi_colore_gfx[c]);
        gfx->carte[c][INVERTI][0] = CaricaSicuro(p2);
        sprintf(p3, "Carte/%s_p2.png", nomi_colore_gfx[c]);
        gfx->carte[c][PIU_DUE][0] = CaricaSicuro(p3);
    }
    // Caricamento dei due tipi di carte Jolly nere (Cambia Colore e Più Quattro)
    gfx->carte[NERO][CAMBIA_COLORE][0] = CaricaSicuro("Carte/wild.png");
    gfx->carte[NERO][PIU_QUATTRO][0] = CaricaSicuro("Carte/wild_4.png");
}

/**
 * Funzione centrale del modulo: renderizza l'intero tavolo da gioco, le mani e l'interfaccia utente ad ogni frame
 */
void DisegnaPartita(Grafica *gfx, Partita *p) {
    // Renderizza lo sfondo di gioco adattandolo forzatamente alla risoluzione della finestra 1200x800
    DrawTexturePro(gfx->sfondo, (Rectangle){0, 0, (float)gfx->sfondo.width, (float)gfx->sfondo.height},
                   (Rectangle){0, 0, 1200, 800}, (Vector2){0,0}, 0, WHITE);

    // Calcolo del posizionamento geometrico del mazzo coperto al centro dello schermo
    float mazzoX = 1200/2.0f - 110.0f;
    float mazzoY = 800/2.0f;

    // Se è il turno del giocatore umano, non ha mosse utili e non ci sono blocchi, evidenzia il mazzo di pesca in oro
    if (p->turno == 0 && !GiocatoreHaMosse(p, 0) && !p->animando && !p->scegli_colore) {
        EvidenziaCarta(mazzoX, mazzoY, 0, GOLD);
    }

    // Disegna la carta coperta del mazzo di pesca e la carta scoperta in cima alla pila degli scarti
    DisegnaCarta(gfx->retro, mazzoX, mazzoY, 0);
    DisegnaCarta(OttieniCarta(gfx, p->cima), 1200/2.0f + 110.0f, 800 / 2.0f, 0);

    // ===================================================================================
    // RENDERMANO GIOCATORE UMANO (Posizionato in basso a ventaglio)
    // ===================================================================================
    int spacing = 65;       // Pixel di distanza standard tra una carta e l'altra
    int max_w = 1200 - 350; // Larghezza massima totale consentita per il blocco delle carte in mano
    // Se la mano è troppo piena, riduce dinamicamente lo spacing per non far uscire le carte dai bordi dello schermo
    if (p->giocatori[0].num_carte * spacing > max_w) spacing = max_w / p->giocatori[0].num_carte;
    // Calcola il punto X di partenza per assicurare che il blocco di carte risulti perfettamente centrato orizzontalmente
    int startX = 600 - ((p->giocatori[0].num_carte - 1) * spacing) / 2;

    for (int i = 0; i < p->giocatori[0].num_carte; i++) {
        float dist = i - (p->giocatori[0].num_carte - 1)/2.0f; // Distanza normalizzata dal centro della mano
        float rot = dist * 5.0f;                               // Calcola un'inclinazione angolare progressiva a ventaglio
        float py = 800 - 100 + abs((int)dist) * 3;             // Effetto curvatura verticale parabolica verso il basso

        // Se è il turno dell'utente e la carta è giocabile, la solleva di 25px e le disegna un contorno dorato
        if (p->turno == 0 && !p->scegli_colore && PuoGiocare(p->giocatori[0].mano[i], p)) {
            py -= 25;
            EvidenziaCarta((float)startX + i*spacing, py, rot, GOLD);
        }
        // Rendering finale della singola carta dell'utente umano
        DisegnaCarta(OttieniCarta(gfx, p->giocatori[0].mano[i]), (float)startX + i*spacing, py, rot);
    }

    // ===================================================================================
    // GESTIONE DEI BOT DINAMICA (Rendering adattivo per partite a 2 o a 4 giocatori)
    // ===================================================================================
    int sp = 35; // Spaziatura ridotta per le carte coperte possedute dai bot
    for (int b = 1; b < p->num_giocatori; b++) {
        Vector2 pos = OttieniPosizioneMazzoGiocatore(b, p->num_giocatori); // Ottiene le coordinate del mazzo del bot b
        int num_carte_bot = p->giocatori[b].num_carte;

        // SE LA PARTITA È 1 VS 1: Il Bot unico si posiziona in alto, speculare all'utente umano
        if (p->num_giocatori == 2) {
            int botX = (int)pos.x - ((num_carte_bot - 1) * sp) / 2;
            for (int i = 0; i < num_carte_bot; i++) {
                float dist = i - (num_carte_bot - 1)/2.0f;
                float py = pos.y - abs((int)dist) * 3; // Curvatura invertita verso l'alto
                DisegnaCarta(gfx->retro, (float)botX + i*sp, py, -dist * 5.0f);
            }
        }
        // SE LA PARTITA È MULTIPLAYER A 4 GIOCATORI: Distribuisce i 3 Bot sui tre lati dello schermo
        else {
            // BOT 1: Posizionato sul lato sinistro della finestra (sviluppo verticale, ruotato di 90 gradi)
            if (b == 1) {
                int bot1Y = (int)pos.y - ((num_carte_bot - 1) * sp) / 2;
                for (int i = 0; i < num_carte_bot; i++) {
                    float dist = i - (num_carte_bot - 1)/2.0f;
                    DisegnaCarta(gfx->retro, pos.x - abs((int)dist)*3, (float)bot1Y + i*sp, 90.0f + dist * 5.0f);
                }
            }
            // BOT 2: Posizionato sul lato alto dello schermo (sviluppo orizzontale rovesciato)
            else if (b == 2) {
                int bot2X = (int)pos.x - ((num_carte_bot - 1) * sp) / 2;
                for (int i = 0; i < num_carte_bot; i++) {
                    float dist = i - (num_carte_bot - 1)/2.0f;
                    float py = pos.y - abs((int)dist) * 3;
                    DisegnaCarta(gfx->retro, (float)bot2X + i*sp, py, -dist * 5.0f);
                }
            }
            // BOT 3: Posizionato sul lato destro della finestra (sviluppo verticale, ruotato di -90 gradi)
            else if (b == 3) {
                int bot3Y = (int)pos.y - ((num_carte_bot - 1) * sp) / 2;
                for (int i = 0; i < num_carte_bot; i++) {
                    float dist = i - (num_carte_bot - 1)/2.0f;
                    DisegnaCarta(gfx->retro, pos.x + abs((int)dist)*3, (float)bot3Y + i*sp, -90.0f - dist * 5.0f);
                }
            }
        }
    }

    // ===================================================================================
    // LOGICA ANIMAZIONI E TESTI DI INTERFACCIA
    // ===================================================================================

    // Se c'è un'animazione di spostamento in corso, disegna la carta in volo facendola ruotare su se stessa su X,Y calcolate
    if (p->animando) {
        DisegnaCarta(OttieniCarta(gfx, p->carta_animata), p->posizione_animazione.x, p->posizione_animazione.y, p->timer_anim * 720.0f);
    }

    // Se l'easter egg del deltaplano è attivo, lo renderizza applicando una scala di riduzione dello 0.6f
    if (p->anima_deltaplano) {
        DrawTextureEx(gfx->deltaplano, (Vector2){p->pos_x_deltaplano, 150}, 0, 0.6f, WHITE);
    }

    // Banner di testo informativo centrale: viene renderizzato solo se il rispettivo timer di persistenza è maggiore di zero
    if (p->timer_messaggio > 0) {
        DisegnaTestoPixelGrassetto(p->messaggio, 600 - MeasureText(p->messaggio, 50)/2, 220, 50, YELLOW);
    }

    // RENDERING MODALE SELEZIONE COLORE (Invocato dopo il lancio di un Jolly nero)
    if (p->scegli_colore) {
        // Applica un velo nero in semitrasparenza (alfa 50%) su tutto lo schermo per oscurare il background
        DrawRectangle(0, 0, 1200, 800, Fade(BLACK, 0.5f));
        Color cols[] = {RED, YELLOW, GREEN, BLUE};
        // Disegna i 4 quadrati cliccabili rappresentanti i colori selezionabili
        for (int i=0; i<4; i++) {
            DrawRectangle(600-160 + i*85, 400-40, 75, 75, cols[i]);
        }

        // Disegna il testo guida posizionato esattamente sopra la palette dei quattro quadrati
        DisegnaTestoPixelGrassetto("SCEGLI COLORE!", 600 - MeasureText("SCEGLI COLORE!", 40)/2, 310, 40, GOLD);

        // Se l'animazione d'interfaccia della foca è attiva, la renderizza centrandola geometricamente sul pannello
        if (p->anima_foca) {
            DrawTextureEx(gfx->foca, (Vector2){1200.0f/2 - (gfx->foca.width*0.45f)/2, 800.0f/2 - (gfx->foca.height*0.45f) - 120}, 0, 0.45f, WHITE);
        }
    }

    // RENDERING GESTIONE TASTO "SOSPENDI PARTITA"
    if (!p->is_guest && gfx->btn_sospendi.id > 0 && !p->partita_finita) {
        Vector2 mousePos = GetMousePosition();
        Rectangle rectSospendi = { 990, 700, 180, 70 }; // Coordinate ed estensione del pulsante di salvataggio
        Color coloreTasto = WHITE;

        // Gestione degli stati visivi del pulsante tramite intercettazione delle collisioni del cursore
        if (CheckCollisionPointRec(mousePos, rectSospendi)) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                coloreTasto = GRAY;              // Stato Clicked: Scurisce il pulsante per simulare la pressione fisica
            } else {
                coloreTasto = Fade(WHITE, 0.6f); // Stato Hover: Scolorisce parzialmente la texture al passaggio del mouse
            }
        }

        // Disegna fisicamente il pulsante apponendo la variazione cromatica calcolata sopra
        DrawTexturePro(gfx->btn_sospendi, (Rectangle){ 0, 0, (float)gfx->btn_sospendi.width, (float)gfx->btn_sospendi.height },
                       rectSospendi, (Vector2){ 0, 0 }, 0, coloreTasto);
    }

    // INTERFACCIA DI FINE PARTITA (SCHERMATA DI GAME OVER)
    if (p->partita_finita) {
        // Oscura pesantemente il tavolo di gioco con una maschera nera all'85% di opacità
        DrawRectangle(0, 0, 1200, 800, Fade(BLACK, 0.85f));
        // Sceglie la texture finale da mostrare a schermo basandosi sull'indice del vincitore (0 è l'umano)
        Texture2D finale = (p->vincitore == 0) ? gfx->win_img : gfx->defeat_img;

        // Logica di scorrimento verticale: fa salire l'immagine dal fondo fino alla quota fissa Y = 200
        static float animY = 800.0f;
        if (animY > 200.0f) {
            animY -= 5.0f;
        }
        // Disegna l'immagine di vittoria o sconfitta centrata orizzontalmente con riscalatura allo 0.8f
        DrawTextureEx(finale, (Vector2){600.0f - (finale.width*0.8f)/2, animY}, 0, 0.8f, WHITE);

        // Seleziona e scrive il banner testuale definitivo applicando il font pixel ad alto spessore
        const char* t = (p->vincitore == 0) ? "VITTORIA FOCALIZZATA!" : "HAI PERSO!";
        DisegnaTestoPixelGrassetto(t, 600 - MeasureText(t, 60)/2, 100, 60, (p->vincitore == 0) ? GOLD : RED);
    }
}

/**
 * Libera sistematicamente dalla memoria video (VRAM) tutte le texture caricate per prevenire memory leak grafici
 */
void ScaricaGrafica(Grafica *gfx) {
    UnloadTexture(gfx->sfondo);
    UnloadTexture(gfx->foca);
    UnloadTexture(gfx->deltaplano);
    UnloadTexture(gfx->win_img);
    UnloadTexture(gfx->defeat_img);
    UnloadTexture(gfx->retro);
    UnloadTexture(gfx->btn_sospendi);
    UnloadTexture(fallback); // Dealloca la texture di fallback

    // Scansione a tappeto della matrice tridimensionale per ripulire le singole texture registrate di ogni carta
    for (int c = 0; c < 5; c++)
        for (int t = 0; t < 6; t++)
            for (int v = 0; v < 13; v++)
                // Controlla se l'ID di Raylib è valido prima di tentare lo scaricamento della risorsa
                if (gfx->carte[c][t][v].id > 0) UnloadTexture(gfx->carte[c][t][v]);
}