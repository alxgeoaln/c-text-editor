#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "../piece_table/piece_table.h"

command_t create_command_object() {

  command_t command = {.count = 0, .index = -1, .history = NULL};

  return command;
}

void create_insert_command(command_t *command_obj, command_action_type_t action,
                           char *value, size_t index) {

  if (command_obj->index < command_obj->count - 1) {
    command_action_t *history = command_obj->history;

    for (int i = command_obj->index + 1; i < command_obj->count; i++) {
      if (history[i].delete_cmd.type == DELETE && history[i].delete_cmd.deleted_text != NULL) {
        free(history[i].delete_cmd.deleted_text);
      } else if (history[i].insert_cmd.type == INSERT && history[i].insert_cmd.value != NULL) {
        free(history[i].insert_cmd.value);
      }
    }

    command_obj->count = command_obj->index + 1;
  }

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
                           size_t start_index, size_t delete_length,
                           char *deleted_text) {

  if (command_obj->index < command_obj->count - 1) {
    command_action_t *history = command_obj->history;

    for (int i = command_obj->index + 1; i < command_obj->count; i++) {
      if (history[i].delete_cmd.type == DELETE && history[i].delete_cmd.deleted_text != NULL) {
        free(history[i].delete_cmd.deleted_text);
      } else if (history[i].insert_cmd.type == INSERT && history[i].insert_cmd.value != NULL) {
        free(history[i].insert_cmd.value);
      }
    }

    command_obj->count = command_obj->index + 1;
  }

  command_delete_action_t command_action = {.type = action,
                                            .start_index = start_index,
                                            .delete_length = delete_length,
                                            .deleted_text = deleted_text};

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

void undo(command_t *command_obj, piece_table_t *piece_table, size_t *cursor_index) {
  int index = command_obj->index;

  if (index < 0) {
    return;
  }

  command_action_t history = command_obj->history[index];

  if (history.insert_cmd.type == INSERT) {
    size_t value_len = strlen(history.insert_cmd.value);
    delete(piece_table, history.insert_cmd.index, value_len);
    *cursor_index = history.insert_cmd.index;
  } else {
    insert_to_add_buffer(piece_table, history.delete_cmd.deleted_text,
                         history.delete_cmd.start_index);
    *cursor_index = history.delete_cmd.start_index + history.delete_cmd.delete_length;
  }

  command_obj->index -= 1;
}

// TODO: to be tested
void redo(command_t *command_obj, piece_table_t *piece_table, size_t *cursor_index) {
  int index = command_obj->index;

  if (index >= command_obj->count - 1) {
    return;
  }

  command_action_t history = command_obj->history[index];

  if (history.insert_cmd.type == INSERT) {
    insert_to_add_buffer(piece_table, history.insert_cmd.value,
                         history.insert_cmd.index);
    *cursor_index = history.insert_cmd.index;
  } else {
    delete(piece_table, history.delete_cmd.start_index, history.delete_cmd.delete_length);
    *cursor_index = history.delete_cmd.start_index;
  }

  command_obj->index += 1;
}
  
void destroy_command(command_t *command_obj) {

  for (size_t i = 0; i < command_obj->count; i++) {
    command_action_t curr_item = command_obj->history[i];

    if (curr_item.insert_cmd.type == INSERT) {
      free(curr_item.insert_cmd.value);
    } else {
      free(curr_item.delete_cmd.deleted_text);
    }
  }

  free(command_obj->history);
  command_obj->history = NULL;
  command_obj->count = 0;
  command_obj->index = -1;
}