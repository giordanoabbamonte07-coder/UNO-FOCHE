cmake_minimum_required(VERSION 3.20)
project(UNO C)

set(CMAKE_C_STANDARD 99)

# Configurazione Raylib (Assicurati che raylib sia installato sul tuo Mac, es. tramite Homebrew)
# Se hai installato raylib con 'brew install raylib', solitamente i path sono questi:
set(RAYLIB_INCLUDE_DIR "/usr/local/include")
set(RAYLIB_LIB_DIR "/usr/local/lib")

include_directories(${RAYLIB_INCLUDE_DIR})
link_directories(${RAYLIB_LIB_DIR})

# Elenco dei file sorgente (tutti i file .c che hai creato)
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

# Link delle librerie necessarie su macOS
target_link_libraries(UNO raylib)
target_link_libraries(UNO "-framework IOKit")
target_link_libraries(UNO "-framework Cocoa")
target_link_libraries(UNO "-framework OpenGL")

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