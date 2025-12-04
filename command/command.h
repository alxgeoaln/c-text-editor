typedef enum { INSERT, DELETE } command_action_type_t;

typedef struct {
  command_action_type_t type;
  char *value;
  size_t index;
} command_insert_action_t;

typedef struct {
  command_action_type_t type;
  size_t start_index;
  size_t delete_length;
} command_delete_action_t;

typedef union {
  command_insert_action_t insert_cmd;
  command_delete_action_t delete_cmd;
} command_action_t;

typedef struct {
  int count;
  int index;
  command_action_t *history;
} command_t;

command_t create_command_object();
void create_insert_command(command_t *command_obj, command_action_type_t action,
                           char *value, size_t index);
void create_delete_command(command_t *command_obj, command_action_type_t action,
                           size_t start_index, size_t delete_length);

void destroy_command(command_t *command_obj);
