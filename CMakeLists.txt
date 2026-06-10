cmake_minimum_required(VERSION 3.20)
project(UNO C)

set(CMAKE_C_STANDARD 99)

# --- RICERCA AUTOMATICA E UNIVERSALE DI RAYLIB ---
# Cerca la libreria nel sistema in modo dinamico (funziona su Mac M1/Intel, Linux e Windows)
find_package(raylib REQUIRED)

# Elenco dei file sorgente
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

# --- LINK DELLE LIBRERIE ---
# Collega Raylib usando il target generato automaticamente da CMake
target_link_libraries(UNO raylib)

# Se il sistema operativo è macOS, aggiunge i framework nativi di Apple necessari per la grafica
if(APPLE)
    target_link_libraries(UNO "-framework IOKit")
    target_link_libraries(UNO "-framework Cocoa")
    target_link_libraries(UNO "-framework OpenGL")
endif()

# --- PARTE FONDAMENTALE PER GLI ASSET ---
# Copia le cartelle multimediali nella cartella di build per non far crashare il gioco
file(COPY
    "${CMAKE_CURRENT_SOURCE_DIR}/Musiche"
    "${CMAKE_CURRENT_SOURCE_DIR}/Sfondo"
    "${CMAKE_CURRENT_SOURCE_DIR}/Animazioni"
    "${CMAKE_CURRENT_SOURCE_DIR}/Pulsanti"
    "${CMAKE_CURRENT_SOURCE_DIR}/Carte"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)
