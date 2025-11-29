#include <stdio.h>

#include "piece_table/piece_table.h"

int main() {
  char *text = "Hello World";

  piece_table_t *piece_table = create_piece_table();
  create_original_buffer(piece_table, text);
  insert_to_add_buffer(piece_table, ",", 5);
  insert_to_add_buffer(piece_table, "!", 13);
  insert_to_add_buffer(piece_table, " ", 15);
  insert_to_add_buffer(piece_table, "test", 20);

  // delete hello
  // delete(piece_table, 0, 5);

  // delete ,
  // delete(piece_table, 5, 1);

  // delete H
  // delete(piece_table, 0, 1);

  // delete E
  // delete(piece_table, 1, 1);

  // delete ll
  // delete(piece_table, 2, 2);

  // delete lo
  // delete(piece_table, 4, 1);

  // delete W
  // delete(piece_table, 7, 1);

  // delete ld
  // delete(piece_table, 10, 2);

  // delete o,
  // delete(piece_table, 4, 2);

  // delete(piece_table, 5, 13);

  // get_char_at(piece_table, 0);
  // get_char_at(piece_table, 12);
  // get_char_at(piece_table, 17);
  // printf("char: %c \n", get_char_at(piece_table, 12));

  print_all_pieces(piece_table);
  get_text_range(piece_table, 0, 3);
  get_text_range(piece_table, 7, 9);
  get_text_range(piece_table, 14, 16);
  get_text_range(piece_table, 3, 10);
  get_text_range(piece_table, 9, 17);
  get_text_range(piece_table, 0, 17);
  // get_text_range(piece_table, 2, 10);
  get_text_from_piece_table(piece_table);

  //  printf("add_buffer: data=\"%s\", length=%lu, capacity=%lu\n",
  //            piece_table->add_buffer->data,
  //            piece_table->add_buffer->length,
  //            piece_table->add_buffer->capacity);

  //  printf("original_buffer: data=\"%s\", length=%lu, capacity=%lu\n",
  //            piece_table->original_buffer->data,
  //            piece_table->original_buffer->length,
  //            piece_table->original_buffer->capacity);

  return 0;
}