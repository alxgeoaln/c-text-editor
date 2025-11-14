#include <stdlib.h>;
#include <string.h>;

#include "piece_table.h";

piece_table_t *create_piece_table()
{

    piece_table_t *piece_table = malloc(sizeof(piece_table_t));
    if (piece_table == NULL)
    {
        return NULL;
    }

    text_buffer_t *add_buffer = malloc(sizeof(text_buffer_t));
    if (add_buffer == NULL)
    {
        free(piece_table);
        return NULL;
    }

    add_buffer->capacity = 0;
    add_buffer->length = 0;
    add_buffer->data = strdup("");

    if (add_buffer->data == NULL)
    {
        free(add_buffer);
        free(piece_table);
    }

    piece_table->piece = NULL;
    piece_table->original_buffer = NULL;
    piece_table->add_buffer = add_buffer;

    return piece_table;
}

piece_t *create_piece(source_t source, size_t length, size_t offset)
{
    piece_t *piece = malloc(sizeof(piece_t));

    if (piece == NULL)
    {
        return NULL;
    }

    piece->source = source;
    piece->offset = offset;
    piece->length = length;
    piece->next = NULL;

    return piece;
}

void create_original_buffer(piece_table_t *piece_table, char *value)
{
    size_t length = strlen(value);
    size_t capacity = length + 1;

    text_buffer_t *original_buffer = malloc(sizeof(text_buffer_t));
    if (original_buffer == NULL)
    {
        return;
    }
    original_buffer->length = length;
    original_buffer->capacity = capacity;
    original_buffer->data = strdup(value);

    if (original_buffer->data == NULL)
    {
        free(original_buffer->data);
        free(original_buffer);
        return;
    }

    piece_t *piece = create_piece(ORIGINAL, length, 0);

    if (piece == NULL)
    {
        return;
    }

    piece_table->original_buffer = original_buffer;
    piece_table->piece = piece;
}

void insert_to_add_buffer(piece_table_t *piece_table, char *value, size_t index)
{
    size_t length = strlen(value);
    text_buffer_t *add_buffer = piece_table->add_buffer;

    // if the capacity is to low allocate new memory, add more capacity
    if (add_buffer->length + length + 1 > add_buffer->capacity)
    {
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

    while (curr && pos + curr->length <= index)
    {
        pos = curr->length;
        prev = curr;
        curr = curr->next;
    }

    if (curr && pos < index)
    {
        // in case the new char is in middle of piece
        size_t curr_length = curr->length;
        size_t second_piece_length = curr_length - index;
        size_t first_piece_length = curr_length - second_piece_length;

        piece_t *first_piece = create_piece(curr->source, first_piece, curr->offset);
        piece_t *second_piece = create_piece(curr->source, second_piece, first_piece_length + length);

        second_piece = curr->next;
        add_piece->next = second_piece;
        first_piece->next = add_piece;
        if (prev)
        {
            prev->next = first_piece;
        }
    }
    else
    {
        add_piece->next = curr;
        // in case if its in between pieces
        if(prev) {
            prev->next = add_piece;
        }
    }
}
