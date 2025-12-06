#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece_table.h"

piece_table_t *create_piece_table() {

  piece_table_t *piece_table = malloc(sizeof(piece_table_t));
  if (piece_table == NULL) {
    return NULL;
  }

  text_buffer_t *add_buffer = malloc(sizeof(text_buffer_t));
  if (add_buffer == NULL) {
    free(piece_table);
    return NULL;
  }

  add_buffer->capacity = 1;
  add_buffer->length = 0;
  add_buffer->data = strdup("");

  if (add_buffer->data == NULL) {
    free(add_buffer);
    free(piece_table);
    return NULL;
  }

  piece_table->piece = NULL;
  piece_table->original_buffer = NULL;
  piece_table->add_buffer = add_buffer;
  piece_table->length = 0;

  return piece_table;
}

piece_t *create_piece(source_t source, size_t length, size_t offset) {
  piece_t *piece = malloc(sizeof(piece_t));

  if (piece == NULL) {
    return NULL;
  }

  piece->source = source;
  piece->offset = offset;
  piece->length = length;
  piece->next = NULL;

  return piece;
}

void create_original_buffer(piece_table_t *piece_table, char *value) {
  size_t length = strlen(value);
  size_t capacity = length + 1;

  piece_table->length += length;

  text_buffer_t *original_buffer = malloc(sizeof(text_buffer_t));
  if (original_buffer == NULL) {
    return;
  }
  original_buffer->length = length;
  original_buffer->capacity = capacity;
  original_buffer->data = strdup(value);

  if (original_buffer->data == NULL) {
    free(original_buffer);
    return;
  }

  piece_t *piece = create_piece(ORIGINAL, length, 0);

  if (piece == NULL) {
    return;
  }

  piece_table->original_buffer = original_buffer;
  piece_table->piece = piece;
}

void delete(piece_table_t *piece_table, size_t start_index,
            size_t delete_length) {
  piece_t *curr = piece_table->piece;
  piece_t *prev = NULL;
  size_t pos = 0;

  piece_table->length -= delete_length;

  while (curr && delete_length > 0) {
    size_t piece_end = pos + curr->length;
    size_t delete_end = start_index + delete_length;

    if (piece_end > start_index && pos < delete_end) {

      // Delete entire piece
      if (start_index <= pos && delete_end >= piece_end) {
        if (prev == NULL) {
          piece_table->piece = curr->next;
        } else {
          prev->next = curr->next;
        }

        delete_length -= (piece_end - pos);

        piece_t *temp = curr;
        curr = curr->next;
        free(temp);
        continue;

        // Delete within/partial piece
      } else {
        size_t local_delete_start = (start_index > pos) ? start_index - pos : 0;
        size_t local_delete_end_in_piece = local_delete_start + delete_length;

        if (local_delete_end_in_piece > curr->length) {
          local_delete_end_in_piece = curr->length;
        }

        size_t actually_deleted =
            local_delete_end_in_piece - local_delete_start;
        delete_length -= actually_deleted;

        size_t before_length = local_delete_start;
        size_t before_offset = curr->offset;

        size_t after_length = curr->length - local_delete_end_in_piece;
        size_t after_offset = curr->offset + local_delete_end_in_piece;

        piece_t *before = NULL;
        piece_t *after = NULL;

        if (before_length > 0) {
          before = create_piece(curr->source, before_length, before_offset);
        }

        if (after_length > 0) {
          after = create_piece(curr->source, after_length, after_offset);
        }

        piece_t *last_new_piece = NULL;

        if (before && after) {
          after->next = curr->next;
          before->next = after;

          if (prev) {
            prev->next = before;
          } else {
            piece_table->piece = before;
          }
          last_new_piece = after;
        } else if (before && !after) {
          before->next = curr->next;

          if (prev) {
            prev->next = before;
          } else {
            piece_table->piece = before;
          }
          last_new_piece = before;
        } else if (!before && after) {
          after->next = curr->next;
          if (prev) {
            prev->next = after;
          } else {
            piece_table->piece = after;
          }
          last_new_piece = after;
        } else {
          // Both NULL - entire piece deleted
          if (prev) {
            prev->next = curr->next;
          } else {
            piece_table->piece = curr->next;
          }
          last_new_piece = prev;
        }

        piece_t *temp = curr;
        piece_t *next = curr->next;
        free(temp);

        // Update for next iteration
        prev = last_new_piece;
        curr = next;

        // Recalculate pos by traversing from start
        pos = 0;
        piece_t *p = piece_table->piece;
        while (p && p != curr) {
          pos += p->length;
          p = p->next;
        }

        continue;
      }
    }

    prev = curr;
    pos += curr->length;
    curr = curr->next;
  }
}

void insert_to_add_buffer(piece_table_t *piece_table, char *value,
                          size_t index) {
  size_t length = strlen(value);
  text_buffer_t *add_buffer = piece_table->add_buffer;

  piece_table->length += length;

  // if the capacity is to low allocate new memory, add more capacity
  if (add_buffer->length + length + 1 > add_buffer->capacity) {
    size_t new_capacity = (add_buffer->length + length + 1) * 2;
    char *new_data = realloc(add_buffer->data, new_capacity);
    if (!new_data)
      return;
    add_buffer->data = new_data;
    add_buffer->capacity = new_capacity;
  }

  // add the new value to ->data buffer
  memcpy(add_buffer->data + add_buffer->length, value, length);
  add_buffer->length += length;
  add_buffer->data[add_buffer->length] = '\0';

  // create a new ADD piece
  piece_t *add_piece = create_piece(ADD, length, add_buffer->length - length);

  piece_t *curr = piece_table->piece;
  piece_t *prev = NULL;
  size_t pos = 0;

  while (curr && pos + curr->length <= index) {
    pos += curr->length;
    prev = curr;
    curr = curr->next;
  }

  if (curr && pos < index) {
    // in case the new char is in middle of piece
    size_t split_offset = index - pos;
    size_t first_len = split_offset;
    size_t second_len = curr->length - split_offset;

    piece_t *first_piece = create_piece(curr->source, first_len, curr->offset);
    piece_t *second_piece =
        create_piece(curr->source, second_len, curr->offset + first_len);

    if (!first_piece || !second_piece) {
      free(first_piece);
      free(second_piece);
      free(add_piece);
      return;
    }

    first_piece->next = add_piece;
    add_piece->next = second_piece;
    second_piece->next = curr->next;

    if (prev) {
      prev->next = first_piece;
    } else {
      piece_table->piece = first_piece;
    }

    free(curr);
  } else {
    add_piece->next = curr;
    if (prev) {
      prev->next = add_piece;
    } else {
      piece_table->piece = add_piece;
    }
  }
}

char *get_text_from_piece_table(piece_table_t *piece_table) {
  size_t new_text_length = 0;

  piece_t *current = piece_table->piece;

  while (current) {
    new_text_length += current->length;
    current = current->next;
  }

  text_buffer_t *text = malloc(sizeof(text_buffer_t));
  if (text == NULL) {
    exit(1);
  }

  char *data = malloc(sizeof(char) * new_text_length + 1);
  if (data == NULL) {
    exit(1);
  }

  free(current);

  piece_t *curr = piece_table->piece;

  size_t acc = 0;
  while (curr) {
    size_t start = curr->offset;
    size_t end = curr->offset + curr->length;
    size_t length = end - start;

    if (curr->source == ORIGINAL) {
      memcpy(data + acc, piece_table->original_buffer->data + start, length);
    } else {
      memcpy(data + acc, piece_table->add_buffer->data + start, length);
    }

    acc += length;
    curr = curr->next;
  }

  free(curr);

  return data;
}


char get_char_at(piece_table_t *piece_table, size_t index) {
  piece_t *curr = piece_table->piece;
  size_t pos = 0;

  while (curr && pos + curr->length <= index) {
    pos += curr->length;
    curr = curr->next;
  }

  if (curr == NULL) {
    printf("Index not in range\n");
    exit(1);
  }

  if (curr->source == ADD) {
    return piece_table->add_buffer->data[index - pos + curr->offset];
  } else {
    return piece_table->original_buffer->data[index - pos + curr->offset];
  }
}

char* get_text_range(piece_table_t *piece_table, size_t start_index,
                    size_t end_index) {

  if (start_index > end_index) {
    return "";
  }

  piece_t *curr = piece_table->piece;
  size_t pos = 0;

  size_t length = end_index - start_index;

  text_buffer_t *text = malloc(sizeof(text_buffer_t));
  if (text == NULL) {
    return "";
  }

  char *data = malloc(sizeof(char) * length + 1);
  if (data == NULL) {
    return "";
  }

  size_t multi_piece_offset = 0;
  size_t multi_piece_len = 0;
  int is_first_piece = 1;
  while (curr) {
    size_t start = curr->offset;
    if (pos + curr->length > start_index && pos < end_index) {
      // print_piece(curr);

      // within a piece
      if (length <= curr->length) {
        size_t offset = 0;
        if (pos < start_index) {
          offset = start_index - pos;
        }

        if (curr->source == ORIGINAL) {
          memcpy(data, piece_table->original_buffer->data + start + offset,
                 length);
        } else {
          memcpy(data, piece_table->add_buffer->data + start + offset,
                 length + 1);
        }
      } else {
        // multiple piece

        // first piece -> shift offset if needed
        if (is_first_piece) {
          if (pos < start_index) {
            multi_piece_offset = start_index - pos;
          }
          if (curr->source == ORIGINAL) {
            memcpy(data,
                   piece_table->original_buffer->data + start +
                       multi_piece_offset,
                   curr->length - multi_piece_offset);
          } else {
            memcpy(data,
                   piece_table->add_buffer->data + start + multi_piece_offset,
                   curr->length - multi_piece_offset);
          }

          multi_piece_len = curr->length - multi_piece_offset;
          is_first_piece = 0;
          // in between piece -> copy the whole piece
        } else if (pos + curr->length <= end_index) {
          if (curr->source == ORIGINAL) {
            memcpy(data + multi_piece_len,
                   piece_table->original_buffer->data + start, curr->length);
          } else {
            memcpy(data + multi_piece_len,
                   piece_table->add_buffer->data + start, curr->length);
          }
          multi_piece_len += curr->length;
        } else {
          if (curr->source == ORIGINAL) {
            memcpy(data + multi_piece_len,
                   piece_table->original_buffer->data + start,
                   end_index - pos + 1);
          } else {
            memcpy(data + multi_piece_len,
                   piece_table->add_buffer->data + start, end_index - pos + 1);
          }
        }
      }
    }

    pos += curr->length;
    curr = curr->next;
  }

  printf("Data from i to i: '%s'\n", data);
  return data;
}

Position index_to_row_col(piece_table_t *piece_table, size_t index) {
  size_t row = 0;
  size_t col = 0;
  size_t last_newline_pos = -1;
  size_t pos = 0;

  piece_t *curr = piece_table->piece;

  while (curr) {
    for (size_t i = curr->offset; i < curr->length + curr->offset; i++) {
      char letter;
      if (curr->source == ORIGINAL) {
        letter = piece_table->original_buffer->data[i];
      } else {
        letter = piece_table->add_buffer->data[i];
      }

      if (letter == '\n') {
        if (pos == index) {
          col = pos - last_newline_pos - 1;
          Position p;
          p.row = row;
          p.col = col;

          return p;
        }
        row++;
        last_newline_pos = pos;
      }

      if (pos == index) {
        col = pos - last_newline_pos - 1;
        Position p;
        p.row = row;
        p.col = col;

        return p;
      }

      pos++;
    }

    curr = curr->next;
  }

  // Fallback if index is at/beyond end
  col = pos - last_newline_pos - 1;
  Position p = {row, col};
  return p;
}

size_t row_col_to_index(piece_table_t *piece_table, int row, int col) {

  if (row == 0 && col == 0) {
    return 0;
  }

  int curr_row = 0;
  int curr_col = 0;

  size_t pos = 0;

  piece_t *curr = piece_table->piece;

  while (curr) {
    for (size_t i = curr->offset; i < curr->length + curr->offset; i++) {
      char letter;
      if (curr->source == ORIGINAL) {
        letter = piece_table->original_buffer->data[i];
      } else {
        letter = piece_table->add_buffer->data[i];
      }

      if (letter == '\n') {
        if (curr_row != row) {
          curr_row += 1;
          curr_col = 0;
        } else {
          return pos;
        }
      } else {
        curr_col = curr_col + 1;
      }

      if (curr_row == row && curr_col == col) {
        return pos + 1;
      }

      pos++;
    }

    curr = curr->next;
  }

  return pos;
}

line_cache_t create_line_cache() {
  size_t *start_indices = malloc(sizeof(size_t) * 0);
  if(start_indices == NULL) {
    printf("start_indices NULL\n");
    exit(1);
  }

  line_cache_t line_cache = {
    .count = 0,
    .start_indices = start_indices
  };

  return line_cache;
}

bool update_line_cache(line_cache_t *cache, int count, size_t index) {
    if (!cache) {
        return false;
    }
    
    int old_count = cache->count;
    int new_total_count = cache->count + count;

    size_t *temp = realloc(cache->start_indices, sizeof(size_t) * new_total_count);

    if (temp == NULL) {
        return false;
    }

    temp[old_count] = index;

    cache->start_indices = temp;
    cache->count = new_total_count;

    return true;
}

void destroy_line_cache(line_cache_t *cache) {
  if (!cache) {
    return;
  }

  free(cache->start_indices);
  cache->start_indices = NULL;
  // free(cache); // Do not free the struct itself if it was allocated on the stack
}

int get_line_count(piece_table_t *piece_table, line_cache_t *line_cache) {
  piece_t *curr = piece_table->piece;

  update_line_cache(line_cache, 1, 0); // line 0 -> index 0

  int line = 0;
  size_t pos = 0;

  while (curr) {
    for (size_t i = curr->offset; i < curr->length + curr->offset; i++) {
      char letter;
      if (curr->source == ORIGINAL) {
        letter = piece_table->original_buffer->data[i];
      } else {
        letter = piece_table->add_buffer->data[i];
      }

      if (letter == '\n') {
        update_line_cache(line_cache, 1, pos + 1);
        line += 1;
      }

      pos++;
    }

    curr = curr->next;
  }

  return line;
}

int get_line_length(piece_table_t *piece_table, int target_row) {
  piece_t *curr = piece_table->piece;
  int length = 0;
  int row = 0;

  while (curr) {
    for (size_t i = curr->offset; i < curr->length + curr->offset; i++) {
      char letter;
      if (curr->source == ORIGINAL) {
        letter = piece_table->original_buffer->data[i];
      } else {
        letter = piece_table->add_buffer->data[i];
      }

      if (row == target_row) {
        // printf("letter %c\n", letter);
        length += 1;
      } else if (letter == '\n') {
        if (row == target_row) {
          return length;
        } else {
          row += 1;
        }
      }
    }

    curr = curr->next;
  }

  return length;
}

void destroy_piece_table(piece_table_t *piece_table) {
  if (piece_table == NULL) {
    return;
  }

  // free pieces
  piece_t *curr = piece_table->piece;

  while (curr) {
    piece_t *temp = curr->next;
    free(curr);
    curr = temp;
  }

  // free add buffer
  if (piece_table->add_buffer) {
    free(piece_table->add_buffer->data);
    free(piece_table->add_buffer);
  }

  // free original buffer
  if (piece_table->original_buffer) {
    free(piece_table->original_buffer->data);
  }

  free(piece_table);
}

void print_piece(piece_t *piece) {
  if (piece == NULL) {
    printf("Piece: NULL\n");
    return;
  }

  printf("Piece { source: %s, offset: %zu, length: %zu, next: %p }\n",
         piece->source == ORIGINAL ? "ORIGINAL" : "ADD", piece->offset,
         piece->length, (void *)piece->next);
}

void print_all_pieces(piece_table_t *piece_table) {
  if (piece_table == NULL) {
    printf("Piece table: NULL\n");
    return;
  }

  piece_t *curr = piece_table->piece;

  printf("--------------- \n");
  while (curr) {
    print_piece(curr);
    curr = curr->next;
  }
  printf("--------------- \n");
}