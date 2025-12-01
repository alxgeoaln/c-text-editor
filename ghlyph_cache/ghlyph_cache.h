#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define GLYPH_CACHE_SIZE 128 

typedef struct {
    SDL_Texture *textures[GLYPH_CACHE_SIZE];
    int char_width;
    int char_height;
    bool initialized;
} glyph_cache_t;

// Initialize the glyph cache with a font
bool glyph_cache_init(glyph_cache_t *cache, TTF_Font *font, SDL_Renderer *renderer, SDL_Color color);

// Clean up the glyph cache
void glyph_cache_destroy(glyph_cache_t *cache);

// Render a single character at grid position (col, row)
void glyph_cache_render_char(glyph_cache_t *cache, SDL_Renderer *renderer, char c, int col, int row);

// Render a string starting at grid position (col, row)
void glyph_cache_render_string(glyph_cache_t *cache, SDL_Renderer *renderer, const char *str, int col, int row);