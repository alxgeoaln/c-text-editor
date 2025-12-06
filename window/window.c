#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

#include "../command/command.h"
#include "../ghlyph_cache/ghlyph_cache.h"
#include "../piece_table/piece_table.h"
#include "window.h"

#define WINDOW_TITLE "Text editor"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int font_px = 20; // Logical pixel height (what you see on screen)

int window_init(piece_table_t *piece_table, command_t *command_obj) {
  struct Window window = {.window = NULL, .renderer = NULL};

  line_cache_t line_cache = create_line_cache();

  if (sdl_initialize(&window)) {
    window_cleanup(&window, piece_table, &line_cache, EXIT_SUCCESS);
  }

  SDL_Color text_color = {0, 0, 0, 255};

  // Initialize TTF
  if (TTF_Init() == -1) {
    printf("TTF_Init Error: %s\n", TTF_GetError());
    window_cleanup(&window, piece_table, &line_cache, EXIT_SUCCESS);
  }

  // Handle font
  TTF_Font *font = TTF_OpenFont("assets/RobotoMono.ttf",
                                calculate_point_size(font_px, window.window));
  if (!font) {
    printf("TTF_OpenFont Error: %s\n", TTF_GetError());
    window_cleanup(&window, piece_table, &line_cache, EXIT_SUCCESS);
  }

  int char_height = TTF_FontHeight(font);

  // Initialize glyph cache
  glyph_cache_t glyph_cache = {0};
  if (!glyph_cache_init(&glyph_cache, font, window.renderer, text_color)) {
    printf("Failed to initialize glyph cache\n");
    TTF_CloseFont(font);
    window_cleanup(&window, piece_table, &line_cache, EXIT_SUCCESS);
  }

  printf("char_width: %d\n", glyph_cache.char_width);
  printf("char_height: %d\n", char_height);

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

  int lines_count = get_line_count(piece_table, &line_cache);

  int target_col = -1;

  int scroll_offset_row = 0;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED ||
            event.window.event == SDL_WINDOWEVENT_MOVED ||
            event.window.event == SDL_WINDOWEVENT_RESIZED) {

          TTF_CloseFont(font);

          font = TTF_OpenFont("assets/RobotoMono.ttf",
                              calculate_point_size(font_px, window.window));
          if (!font) {
          }

          char_height = TTF_FontHeight(font);

          if (!glyph_cache_init(&glyph_cache, font, window.renderer,
                                text_color)) {
            printf("Failed to initialize glyph cache\n");
            window_cleanup(&window, piece_table, &line_cache, EXIT_SUCCESS);
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
        case SDLK_z:
           if (event.key.keysym.mod & KMOD_GUI ||
                     event.key.keysym.mod & KMOD_CTRL) {
            undo(command_obj, piece_table, &index);

            position = index_to_row_col(piece_table, index);
            lines_count = get_line_count(piece_table, &line_cache);
          }
          break;

        case SDLK_ESCAPE:
          running = false;
          break;

        case SDLK_BACKSPACE:
          if (index > 0) {
            char *deleted_text = get_text_range(piece_table, index - 1, index);
            create_delete_command(command_obj, DELETE, index - 1, 1,
                                  deleted_text);
            delete(piece_table, index - 1, 1);
            index -= 1;
            position = index_to_row_col(piece_table, index);

            lines_count = get_line_count(piece_table, &line_cache);
          }
          break;

        case SDLK_RETURN:
          insert_to_add_buffer(piece_table, "\n", index);
          create_insert_command(command_obj, INSERT, "\n", index);
          index += 1;
          lines_count += 1;
          if (!update_line_cache(&line_cache, 1, index)) {
            printf("Line cache doesn't update\n");
          }

          position = index_to_row_col(piece_table, index);
          break;

        case SDLK_LEFT:
          if (index > 0) {
            if (target_col > -1) {
              target_col = -1;
            }

            index -= 1;
            position = index_to_row_col(piece_table, index);
          }
          break;

        case SDLK_RIGHT:
          if (index < piece_table->length) {
            if (target_col > -1) {
              target_col = -1;
            }

            index += 1;
            position = index_to_row_col(piece_table, index);
          }
          break;

        case SDLK_UP:
          if (position.row > 0) {
            if (target_col == -1) {
              target_col = position.col;
            }

            size_t current_line = line_cache.start_indices[position.row];
            size_t prev_line = line_cache.start_indices[position.row - 1];

            int desired_col = MIN(target_col, current_line - prev_line - 1);

            index =
                row_col_to_index(piece_table, position.row - 1, desired_col);

            position = index_to_row_col(piece_table, index);
          }
          break;

        case SDLK_DOWN:
          if (position.row < lines_count) {
            if (target_col == -1) {
              target_col = position.col;
            }

            int target_row = position.row + 1;

            size_t index_target_line = line_cache.start_indices[target_row];
            size_t target_len = 0;

            if (target_row < lines_count) {
              size_t next_index_target =
                  line_cache.start_indices[target_row + 1];
              target_len = next_index_target - index_target_line - 1;
            } else {
              target_len = piece_table->length - index_target_line;
            }

            int desired_col = MIN(target_col, target_len);

            index = row_col_to_index(piece_table, target_row, desired_col);

            position = index_to_row_col(piece_table, index);
          }
          break;

        case SDLK_TAB: {
          char *tab_string = "    ";
          size_t tab_length = strlen(tab_string);
          insert_to_add_buffer(piece_table, tab_string, index);
          create_insert_command(command_obj, INSERT, tab_string, index);
          index += tab_length;
          position = index_to_row_col(piece_table, index);
          break;
        }
        default:
          break;
        }
      }

      if (event.type == SDL_TEXTINPUT) {
        char *input_text = event.text.text;
        insert_to_add_buffer(piece_table, input_text, index);
        create_insert_command(command_obj, INSERT, input_text, index);
        index += strlen(input_text);
        position = index_to_row_col(piece_table, index);

        if (position.col >= cols) {
          insert_to_add_buffer(piece_table, "\n", index);
          create_insert_command(command_obj, INSERT, "\n", index);
          index += 1;
          lines_count += 1;
          if (!update_line_cache(&line_cache, 1, index)) {
            printf("Line cache doesn't update\n");
          }
          position = index_to_row_col(piece_table, index);
        }
      }
    }

    // Adjust scroll view if cursor moves out of visible bounds
    if (position.row < scroll_offset_row) {
      scroll_offset_row = position.row;
    } else if (position.row >= scroll_offset_row + rows) {
      scroll_offset_row = position.row - rows + 1;
    }

    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
    SDL_RenderClear(window.renderer);

    glyph_cache_render_string(&glyph_cache, window.renderer,
                              get_text_from_piece_table(piece_table), 0,
                              -scroll_offset_row);

    get_cursor(window.renderer, char_height, position.row - scroll_offset_row,
               position.col, glyph_cache.char_width);

    SDL_RenderPresent(window.renderer);
  }

  window_cleanup(&window, piece_table, &line_cache, EXIT_SUCCESS);

  return 0;
}

void window_cleanup(struct Window *window, piece_table_t *piece_table,
                    line_cache_t *line_cache, int exit_status) {
  SDL_DestroyRenderer(window->renderer);
  SDL_DestroyWindow(window->window);
  SDL_Quit();
  TTF_Quit();
  destroy_piece_table(piece_table);
  if (line_cache->start_indices != NULL) {
    destroy_line_cache(line_cache);
  }
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

// Function to calculate the required point size for consistent rendering
// target_pixel_height: desired height in LOGICAL pixels (what you see on
// screen)
int calculate_point_size(int target_pixel_height, SDL_Window *window) {
  // Get the window size (logical pixels)
  int window_w, window_h;
  SDL_GetWindowSize(window, &window_w, &window_h);

  // Get the renderer output size (physical pixels)
  SDL_Renderer *renderer = SDL_GetRenderer(window);
  int render_w, render_h;
  if (renderer &&
      SDL_GetRendererOutputSize(renderer, &render_w, &render_h) == 0) {
    // Calculate the scale factor (e.g., 2.0 on Retina)
    float scale = (float)render_h / (float)window_h;

    // Scale the target pixel height to physical pixels
    int physical_pixels = (int)(target_pixel_height * scale + 0.5f);

    // TTF_OpenFont expects points. At 72 DPI, 1pt = 1px.
    // SDL_ttf on macOS with high-DPI renders at the physical resolution,
    // so we pass the physical pixel size directly as the point size.
    return (physical_pixels > 8) ? physical_pixels : 8;
  }

  // Fallback: just use the target as-is
  printf("Fallback: using target_pixel_height directly: %d\n",
         target_pixel_height);
  return (target_pixel_height > 8) ? target_pixel_height : 8;
}
