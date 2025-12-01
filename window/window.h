#include <SDL.h>
#include <stdbool.h>

typedef struct Window {
  SDL_Window *window;
  SDL_Renderer *renderer;
} window_t;

typedef struct {
  SDL_Texture *texture;
  SDL_Rect rect;
} letter_t;

int window_init(piece_table_t *piece_table);
void window_cleanup(struct Window *window, int exist_status);
bool sdl_initialize(struct Window *window);
