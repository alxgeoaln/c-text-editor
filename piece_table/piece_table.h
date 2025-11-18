typedef enum Source { ADD, ORIGINAL } source_t;

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
} piece_table_t;

piece_table_t *create_piece_table();
piece_t *create_piece(source_t source, size_t length, size_t offset);
void create_original_buffer(piece_table_t *piece_table, char *value);
void insert_to_add_buffer(piece_table_t *piece_table, char *value,
                          size_t offset);
void get_text_from_piece_table(piece_table_t *piece_table);
