#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

#include "../ghlyph_cache/ghlyph_cache.h"
#include "../piece_table/piece_table.h"
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
  TTF_Font *font = TTF_OpenFont("assets/RobotoMono.ttf", 40);
  if (!font) {
    printf("TTF_OpenFont Error: %s\n", TTF_GetError());
    window_cleanup(&window, EXIT_FAILURE);
  }

  int char_height = TTF_FontHeight(font);

  // Initialize glyph cache
  glyph_cache_t glyph_cache = {0};
  if (!glyph_cache_init(&glyph_cache, font, window.renderer, text_color)) {
    printf("Failed to initialize glyph cache\n");
    TTF_CloseFont(font);
    window_cleanup(&window, EXIT_FAILURE);
  }

  printf("char_width: %d\n", glyph_cache.char_width);
  printf("char_height: %d\n", char_height);
  // Calculate grid dimensions
  //   int cols = SCREEN_WIDTH / glyph_cache.char_width * 2;
  //   int rows = SCREEN_HEIGHT / char_height;

  int render_output_w, render_output_h;

  if (SDL_GetRendererOutputSize(window.renderer, &render_output_w,
                                &render_output_h) != 0) {
    printf("SDL_GetRendererOutputSize Error: %s\n", SDL_GetError());
  }

  int cols = render_output_w / glyph_cache.char_width;
  int rows = render_output_h / char_height;

  // Set background color to white so you can see it
  SDL_RenderClear(window.renderer);
  SDL_RenderPresent(window.renderer);

  // Event loop - keep window open until user closes it
  bool running = true;
  SDL_Event event;
  SDL_StartTextInput();

  size_t index = 0;
  Position position = index_to_row_col(piece_table, index);

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED ||
            event.window.event == SDL_WINDOWEVENT_MOVED ||
            event.window.event == SDL_WINDOWEVENT_RESIZED) {

          TTF_CloseFont(font);

          font = TTF_OpenFont("assets/RobotoMono.ttf", 40);
          if (!font) {
          }

          char_height = TTF_FontHeight(font);

          if (!glyph_cache_init(&glyph_cache, font, window.renderer,
                                text_color)) {
            printf("Failed to initialize glyph cache\n");
            window_cleanup(&window, EXIT_FAILURE);
          }

          int render_output_w, render_output_h;
          SDL_GetRendererOutputSize(window.renderer, &render_output_w,
                                    &render_output_h);

          cols = render_output_w / glyph_cache.char_width;
          rows = render_output_h / char_height;
        }
      }

      if (event.type == SDL_QUIT) {
        running = false;
      }

      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {

        case SDLK_ESCAPE:
          running = false;
          break;

        case SDLK_BACKSPACE:
          delete(piece_table, index - 1, 1);

          if (index > 0) {
            index -= 1;
            position = index_to_row_col(piece_table, index);
          }
          break;

        case SDLK_RETURN:
          insert_to_add_buffer(piece_table, "\n", index);
          index += 1;
          position = index_to_row_col(piece_table, index);
          break;

        default:
          break;
        }
      }

      if (event.type == SDL_TEXTINPUT) {
        char *input_text = event.text.text;
        insert_to_add_buffer(piece_table, input_text, index);
        index += strlen(input_text);
        position = index_to_row_col(piece_table, index);

        if (position.col >= cols) {
          insert_to_add_buffer(piece_table, "\n", index);
          index += 1;
          position = index_to_row_col(piece_table, index);
        }
        printf("index: %zu, position: %d, %d\n", index, position.row,
               position.col),
            printf("cols: %d, rows: %d\n", cols, rows);
      }
    }

    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
    SDL_RenderClear(window.renderer);

    glyph_cache_render_string(&glyph_cache, window.renderer,
                              get_text_from_piece_table(piece_table), 0, 0);

    get_cursor(window.renderer, char_height, position.row, position.col,
               glyph_cache.char_width);

    SDL_RenderPresent(window.renderer);
  }

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

  window->window = SDL_CreateWindow(
      WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
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

void get_cursor(SDL_Renderer *renderer, int char_height, size_t row, size_t col,
                int char_width) {

  SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
  SDL_Rect cursor_rect = {col * char_width, row * char_height, 5, char_height};
  SDL_RenderFillRect(renderer, &cursor_rect);
}
