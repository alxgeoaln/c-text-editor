#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

#include "../piece_table/piece_table.h"
#include "../ghlyph_cache/ghlyph_cache.h"
#include "window.h"

#define WINDOW_TITLE "Text editor"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int window_init(piece_table_t *piece_table) {
  struct Window window = {.window = NULL, .renderer = NULL};

  if (sdl_initialize(&window)) {
    window_cleanup(&window, EXIT_FAILURE);
  }

  SDL_Color text_color = {0, 0, 0, 255};

  // Initialize TTF
  if (TTF_Init() == -1) {
    printf("TTF_Init Error: %s\n", TTF_GetError());
    window_cleanup(&window, EXIT_FAILURE);
  }

  // Handle font
  TTF_Font *font = TTF_OpenFont("assets/RobotoMono.ttf", 50);
  if (!font) {
    printf("TTF_OpenFont Error: %s\n", TTF_GetError());
    window_cleanup(&window, EXIT_FAILURE);
  }

  int char_height = TTF_FontHeight(font);

  int char_width;
  TTF_GlyphMetrics(font, 'M', NULL, NULL, NULL, NULL, &char_width);

  // Initialize glyph cache
  glyph_cache_t glyph_cache;
  if (!glyph_cache_init(&glyph_cache, font, window.renderer, text_color)) {
    printf("Failed to initialize glyph cache\n");
    TTF_CloseFont(font);
    window_cleanup(&window, EXIT_FAILURE);
  }

  // Calculate grid dimensions
  int cols = SCREEN_WIDTH / glyph_cache.char_width;
  int rows = SCREEN_HEIGHT / glyph_cache.char_height;
  printf("Grid: %d columns x %d rows\n", cols, rows);

  // Set background color to white so you can see it
  SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
  SDL_RenderClear(window.renderer);
  SDL_RenderPresent(window.renderer);

  // Event loop - keep window open until user closes it
  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      // ESC key to quit
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        running = false;
      }
    }

    SDL_RenderClear(window.renderer);

    glyph_cache_render_string(&glyph_cache, window.renderer, get_text_from_piece_table(piece_table), 0,
                              0);

    SDL_RenderPresent(window.renderer);
  }

  //   SDL_DestroyTexture(texture);
  TTF_CloseFont(font);
  window_cleanup(&window, EXIT_SUCCESS);

  return 0;
}

void window_cleanup(struct Window *window, int exit_status) {
  SDL_DestroyRenderer(window->renderer);
  SDL_DestroyWindow(window->window);
  SDL_Quit();
  TTF_Quit();
  exit(exit_status);
}

bool sdl_initialize(struct Window *window) {
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    return true;
  }

  window->window =
      SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!window->window) {
    fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
    return true;
  }

  window->renderer = SDL_CreateRenderer(window->window, -1, 0);
  if (!window->renderer) {
    fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
    return true;
  }

  return false;
}