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