cmake_minimum_required(VERSION 3.20)
project(UNO C)

set(CMAKE_C_STANDARD 99)

# CREA IL PROGRAMMA (DEVE ESSERE PRIMA)
add_executable(UNO
        main.c
        game.h
        game.c
        audio.c
        graphics.h
        graphics.c
        audio.h
)

# LINK RAYLIB (DOPO add_executable)
find_package(raylib REQUIRED)
target_link_libraries(UNO raylib)

# COPIA ASSET
file(COPY carte DESTINATION ${CMAKE_BINARY_DIR})
file(COPY assets_musica.mp3 DESTINATION ${CMAKE_BINARY_DIR})