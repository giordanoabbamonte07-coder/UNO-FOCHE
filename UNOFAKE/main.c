#include "raylib.h"
#include "gioco.h"
#include <stdio.h>

int main() {
    // 1. Inizializzazione Finestra e Audio
    InitWindow(1200, 800, "UNO - Interactive Edition");
    InitAudioDevice();
    SetTargetFPS(60);

    // 2. Caricamento Risorse
    // Assicurati che il file mp3 sia nella cartella principale o in cmake-build-debug
    Music music = LoadMusicStream("assets_musica.mp3");
    if (music.stream.buffer != NULL) PlayMusicStream(music);

    // Se hai un'immagine di sfondo chiamata 'tavolo.png' nella cartella 'Carte'
    // Texture2D sfondo = LoadTexture("Carte/tavolo.png");

    // 3. Setup Gioco
    Gioco gioco;
    inizializzaGioco(&gioco);

    float timerBot = 0;
    bool vittoria = false;
    int vincitore = -1;

    // 4. Ciclo di Gioco Principale
    while (!WindowShouldClose()) {
        UpdateMusicStream(music);
        Vector2 mousePos = GetMousePosition();

        if (!vittoria) {
            // --- LOGICA TURNO ---
            if (!gioco.giocatori[gioco.turno].isBot) {
                // --- GIOCATORE UMANO ---
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    // Controlliamo le carte da destra a sinistra (per cliccare quella in primo piano)
                    for (int i = gioco.giocatori[0].numCarte - 1; i >= 0; i--) {
                        Texture2D tex = gioco.giocatori[0].mano[i].texture;
                        float w = tex.width * 0.4f;
                        float h = tex.height * 0.4f;
                        Rectangle rect = { 150.0f + i * 55.0f, 580.0f, w, h };

                        if (CheckCollisionPointRec(mousePos, rect)) {
                            if (cartaValida(gioco.giocatori[0].mano[i], gioco.scarti[gioco.topScarti - 1])) {
                                gioco.scarti[gioco.topScarti++] = gioco.giocatori[0].mano[i];
                                rimuoviCarta(&gioco.giocatori[0], i);

                                if (gioco.giocatori[0].numCarte == 0) {
                                    vittoria = true;
                                    vincitore = 0;
                                } else {
                                    applicaEffetto(&gioco, gioco.scarti[gioco.topScarti-1]);
                                    prossimoTurno(&gioco);
                                }
                            }
                            break;
                        }
                    }
                }
                // Tasto P per pescare se non si hanno carte giocabili
                if (IsKeyPressed(KEY_P)) {
                    gioco.giocatori[0].mano[gioco.giocatori[0].numCarte++] = pesca(gioco.mazzo, &gioco.topMazzo);
                    prossimoTurno(&gioco);
                }
            } else {
                // --- LOGICA BOT ---
                timerBot += GetFrameTime();
                if (timerBot > 1.5f) { // Il bot "pensa" per 1.5 secondi
                    int idx = scegliCartaBot(&gioco.giocatori[gioco.turno], gioco.scarti[gioco.topScarti - 1]);
                    if (idx != -1) {
                        Carta c = gioco.giocatori[gioco.turno].mano[idx];
                        gioco.scarti[gioco.topScarti++] = c;
                        rimuoviCarta(&gioco.giocatori[gioco.turno], idx);
                        if (gioco.giocatori[gioco.turno].numCarte == 0) {
                            vittoria = true;
                            vincitore = gioco.turno;
                        } else {
                            applicaEffetto(&gioco, c);
                        }
                    } else {
                        gioco.giocatori[gioco.turno].mano[gioco.giocatori[gioco.turno].numCarte++] = pesca(gioco.mazzo, &gioco.topMazzo);
                    }
                    if (!vittoria) prossimoTurno(&gioco);
                    timerBot = 0;
                }
            }
        }

        // --- DISEGNO ---
        BeginDrawing();
        ClearBackground(DARKGREEN);

        // Disegna lo sfondo (se presente)
        // DrawTexture(sfondo, 0, 0, WHITE);

        // 1. Carta sul tavolo (Scarti)
        if (gioco.topScarti > 0) {
            Texture2D tTex = gioco.scarti[gioco.topScarti - 1].texture;
            DrawTextureEx(tTex, (Vector2){540, 300}, 0.0f, 0.5f, WHITE);
            DrawRectangleLines(540, 300, tTex.width*0.5f, tTex.height*0.5f, RAYWHITE);
        }

        // 2. Mano del Giocatore con Effetti Visivi
        for (int i = 0; i < gioco.giocatori[0].numCarte; i++) {
            Texture2D tex = gioco.giocatori[0].mano[i].texture;
            float w = tex.width * 0.4f;
            float h = tex.height * 0.4f;
            Vector2 pos = { 150.0f + i * 55.0f, 580.0f };
            Rectangle r = { pos.x, pos.y, w, h };

            // Effetto Hover: la carta si alza se il mouse ci passa sopra
            if (!vittoria && gioco.turno == 0 && CheckCollisionPointRec(mousePos, r)) {
                DrawTextureEx(tex, (Vector2){pos.x, pos.y - 40}, 0.0f, 0.4f, WHITE);
                DrawRectangleLines(pos.x, pos.y - 40, w, h, GOLD); // Bordo dorato di selezione
            } else {
                DrawTextureEx(tex, pos, 0.0f, 0.4f, WHITE);
            }
        }

        // 3. Interfaccia Utente (UI)
        DrawText(TextFormat("TURNO: %s", (gioco.turno == 0 ? "TU" : TextFormat("BOT %d", gioco.turno))), 20, 20, 30, GOLD);

        for(int i=1; i<4; i++) {
            DrawText(TextFormat("Bot %d: %d carte", i, gioco.giocatori[i].numCarte), 980, 20 + (i*30), 20, LIGHTGRAY);
        }

        if (gioco.giocatori[gioco.turno].isBot && !vittoria) {
            DrawRectangle(450, 220, 300, 40, Fade(BLACK, 0.5f));
            DrawText("Il Bot sta decidendo...", 470, 230, 20, RAYWHITE);
        }

        // 4. Schermata Finale
        if (vittoria) {
            DrawRectangle(0, 0, 1200, 800, Fade(BLACK, 0.7f));
            const char* msg = (vincitore == 0) ? "VITTORIA! HAI VINTO!" : TextFormat("GAME OVER - VINCE BOT %d", vincitore);
            Color msgCol = (vincitore == 0) ? GREEN : RED;
            DrawText(msg, 600 - MeasureText(msg, 50)/2, 350, 50, msgCol);
            DrawText("Premi 'R' per Nuova Partita", 480, 450, 20, RAYWHITE);

            if (IsKeyPressed(KEY_R)) {
                inizializzaGioco(&gioco);
                vittoria = false;
            }
        }

        EndDrawing();
    }

    // 5. De-inizializzazione
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}