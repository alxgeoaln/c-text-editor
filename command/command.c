#include <stdlib.h>
#include <string.h>

command_t create_command_object() {

  command_t command = {.count = 0, .index = -1, .history = NULL};

  return command;
}

void create_insert_command(command_t *command_obj, command_action_type_t action,
                           char *value, size_t index) {

  command_obj->index += 1;
  command_obj->count += 1;
  command_action_t *new_history = realloc(
      command_obj->history, sizeof(command_action_t) * command_obj->count);

  if (!new_history) {
    printf("Realloc failed create_insert_command\n");
    exit(1);
  }

  command_obj->history = new_history;

  char *value_copy = malloc(strlen(value) + 1);
  strcpy(value_copy, value);

  command_insert_action_t command_action = {
      .type = action, .value = value_copy, .index = index};

  command_obj->history[command_obj->index].insert_cmd = command_action;
}

void create_delete_command(command_t *command_obj, command_action_type_t action,
                           size_t start_index, size_t delete_length) {

  command_delete_action_t command_action = {.type = action,
                                            .start_index = start_index,
                                            .delete_length = delete_length};

  command_obj->index += 1;
  command_obj->count += 1;
  command_action_t *new_history = realloc(
      command_obj->history, sizeof(command_action_t) * command_obj->count);

  if (!new_history) {
    printf("Realloc failed create_delete_command\n");
    exit(1);
  }

  command_obj->history = new_history;
  command_obj->history[command_obj->index].delete_cmd = command_action;
}

void destroy_command(command_t *command_obj) {

  for (size_t i = 0; i < command_obj->count; i++) {
    command_action_t curr_item = command_obj->history[i];

    if (curr_item.insert_cmd.type == INSERT) {
      free(curr_item.insert_cmd.value);
    }
  }

  free(command_obj->history);
  command_obj->history = NULL;
  command_obj->count = 0;
  command_obj->index = -1;
}