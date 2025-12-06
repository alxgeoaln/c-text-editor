#pragma once

#include <stdbool.h>

typedef enum Source { ADD, ORIGINAL } source_t;

typedef struct {
  int row;
  int col;
} Position;

typedef struct {
    size_t *start_indices; 
    int count;            
} line_cache_t;

typedef struct Piece {
  source_t source;
  size_t length;
  size_t offset;
  struct Piece *next;
} piece_t;

typedef struct {
  char *data;
  size_t length;
  size_t capacity;
} text_buffer_t;

typedef struct PieceTable {
  piece_t *piece;
  const text_buffer_t *original_buffer;
  text_buffer_t *add_buffer;
  size_t length;
} piece_table_t;

piece_table_t *create_piece_table();
piece_t *create_piece(source_t source, size_t length, size_t offset);
void create_original_buffer(piece_table_t *piece_table, char *value);
void insert_to_add_buffer(piece_table_t *piece_table, char *value,
                          size_t offset);
char* get_text_from_piece_table(piece_table_t *piece_table);
void delete(piece_table_t *pice_table, size_t start_index, size_t length);
char get_char_at(piece_table_t *piece_table, size_t index);
char* get_text_range(piece_table_t *piece_table, size_t start_index,size_t end_index);
void destroy_piece_table(piece_table_t *piece_table);
Position index_to_row_col(piece_table_t *piece_table, size_t index);
size_t row_col_to_index(piece_table_t *piece_table, int row, int col);

line_cache_t create_line_cache();
bool update_line_cache(line_cache_t *cache, int count, size_t index);
void destroy_line_cache(line_cache_t *cache);

int get_line_count(piece_table_t *piece_table, line_cache_t *line_cache);
int get_line_length(piece_table_t *piece_table, int target_row);

void print_piece(piece_t *piece);
void print_all_pieces(piece_table_t *piece_table);