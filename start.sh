gcc main.c piece_table/piece_table.c window/window.c ghlyph_cache/ghlyph_cache.c command/command.c \
    `sdl2-config --cflags --libs` \
    -lSDL2_ttf \
    -o main
./main