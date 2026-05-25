cmake_minimum_required(VERSION 3.20)
project(UNO C)

set(CMAKE_C_STANDARD 99)

# Configurazione Raylib per Fedora
# Assicurati di aver installato raylib sul tuo sistema tramite:
# sudo dnf install raylib-devel
# I path di sistema (come /usr/include e /usr/lib64) vengono trovati automaticamente.

# Elenco dei file sorgente (tutti i file .c e .h del progetto)
set(SOURCES
        main.c
        game.c
        graphics.c
        audio.c
        menu.c
        menu_scelta.h
        menu_scelta.c
        archivio.h
        archivio.c
)

add_executable(UNO ${SOURCES})

# Link delle librerie necessarie su Linux (Fedora)
# Su Linux, Raylib richiede spesso di essere linkata esplicitamente a queste librerie di sistema
target_link_libraries(UNO
        raylib
        GL       # OpenGL
        m        # Libreria matematica
        pthread  # Threading
        dl       # Dynamic linking
        rt       # Real-time
        X11      # Server grafico X11
)

# --- PARTE FONDAMENTALE PER GLI ASSET ---
# Questa sezione copia le cartelle dei contenuti nella cartella dove viene creato l'eseguibile.
# Senza questo, il gioco non troverebbe le immagini e le musiche.

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/Musiche"
        "${CMAKE_CURRENT_SOURCE_DIR}/Sfondo"
        "${CMAKE_CURRENT_SOURCE_DIR}/Animazioni"
        "${CMAKE_CURRENT_SOURCE_DIR}/Pulsanti"
        "${CMAKE_CURRENT_SOURCE_DIR}/Carte"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)