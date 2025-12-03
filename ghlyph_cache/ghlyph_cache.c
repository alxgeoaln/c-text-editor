#include "ghlyph_cache.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

bool glyph_cache_init(glyph_cache_t *cache, TTF_Font *font,
                      SDL_Renderer *renderer, SDL_Color color) {
  if (!cache || !font || !renderer) {
    return false;
  }

  glyph_cache_destroy(cache);

  // Initialize all textures to NULL
  memset(cache->textures, 0, sizeof(cache->textures));
  cache->initialized = false;

  // Get character dimensions from the font
  cache->char_height = TTF_FontHeight(font);

  // Get width from a sample character (monospace fonts have uniform width)
  int min_x, max_x, min_y, max_y, advance;
  if (TTF_GlyphMetrics(font, 'M', &min_x, &max_x, &min_y, &max_y, &advance) ==
      0) {
    cache->char_width = advance;
  } else {
    fprintf(stderr, "Failed to get glyph metrics: %s\n", TTF_GetError());
    return false;
  }

  // Pre-render all printable ASCII characters (32-126)
  for (int i = 32; i < 127; i++) {
    char str[2] = {(char)i, '\0'};

    SDL_Surface *surface = TTF_RenderText_Blended(font, str, color);
    if (!surface) {
      fprintf(stderr, "Failed to render glyph '%c': %s\n", (char)i,
              TTF_GetError());
      glyph_cache_destroy(cache);
      return false;
    }

    cache->textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!cache->textures[i]) {
      fprintf(stderr, "Failed to create texture for glyph '%c': %s\n", (char)i,
              SDL_GetError());
      glyph_cache_destroy(cache);
      return false;
    }
  }

  cache->initialized = true;
  // printf("Glyph cache initialized: %dx%d characters\n", cache->char_width,
        //  cache->char_height);
  return true;
}

void glyph_cache_destroy(glyph_cache_t *cache) {
  if (!cache) {
    return;
  }

  for (int i = 0; i < GLYPH_CACHE_SIZE; i++) {
    if (cache->textures[i]) {
      SDL_DestroyTexture(cache->textures[i]);
      cache->textures[i] = NULL;
    }
  }

  cache->initialized = false;
}

void glyph_cache_render_char(glyph_cache_t *cache, SDL_Renderer *renderer,
                             char c, int col, int row) {
  if (!cache || !cache->initialized || !renderer) {
    return;
  }

  // Skip non-printable characters
  unsigned char uc = (unsigned char)c;
  if (uc >= GLYPH_CACHE_SIZE || !cache->textures[uc]) {
    return;
  }

  SDL_Texture *texture = cache->textures[c];
  if (!texture)
    return;

  int tex_w, tex_h;
  SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);

  // Calculate pixel position from grid position
  int pixel_x = col * cache->char_width;
  int pixel_y = row * cache->char_height;

  SDL_Rect dest = {pixel_x, pixel_y, tex_w, tex_h};

  SDL_RenderCopy(renderer, cache->textures[uc], NULL, &dest);
}

void glyph_cache_render_string(glyph_cache_t *cache, SDL_Renderer *renderer,
                               const char *str, int col, int row) {
  if (!cache || !cache->initialized || !renderer || !str) {
    return;
  }

  int current_col = col;
  int current_row = row;

  for (size_t i = 0; str[i] != '\0'; i++) {
    if (str[i] == '\n') {
      current_row++;
      current_col = col;
    } else {
      glyph_cache_render_char(cache, renderer, str[i], current_col,
                              current_row);
      current_col++;
    }
  }
}
