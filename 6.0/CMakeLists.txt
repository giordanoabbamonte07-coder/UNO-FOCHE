cmake_minimum_required(VERSION 3.25)
project(UNO_FOCHE C)

# Imposta lo standard del C
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Elenco di tutti i moduli .c del gioco
set(SOURCES
        main.c
        game.c
        audio.c
        graphics.c
        main.c
        "../UNO 1vs1 5.0/main.c"
)

# Definizione dell'eseguibile (chiamato uno_game come quello in cartella)
add_executable(uno_game ${SOURCES})

# =====================================================================
# NOTA SULLE LIBRERIE ESTERNE
# Visto che hai moduli per audio e grafica, probabilmente usi una libreria
# come Raylib o SDL2. Scommenta le righe relative alla libreria che usi:
# =====================================================================

# --- Se usi RAYLIB, scommenta queste due righe: ---
 find_package(raylib REQUIRED)
 target_link_libraries(uno_game PRIVATE raylib)

# --- Se usi SDL2, scommenta queste due righe (adatta se usi anche SDL2_image/mixer): ---
# find_package(SDL2 REQUIRED)
# target_link_libraries(uno_game PRIVATE SDL2::SDL2)