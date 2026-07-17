#include "undo_redo.h"
#include <stdlib.h>
#include <string.h>

void initStack(CommandStack *stack) {
  stack->top = NULL;
  stack->size = 0;
}

void push(CommandStack *stack, Command *cmd) {
  cmd->next = stack->top;
  stack->top = cmd;
  stack->size++;
}

Command *pop(CommandStack *stack) {
  if (stack->top == NULL)
    return NULL;
  Command *cmd = stack->top;
  stack->top = cmd->next;
  cmd->next = NULL;
  stack->size--;
  return cmd;
}

int isEmptyStack(CommandStack *stack) { return stack->size == 0; }

void clearStack(CommandStack *stack) {
  Command *cur = stack->top;
  while (cur != NULL) {
    Command *tmp = cur;
    cur = cur->next;
    if (tmp->data != NULL) {
      free(tmp->data);
    }
    free(tmp);
  }
  stack->top = NULL;
  stack->size = 0;
}

void initManager(UndoRedoManager *mgr) {
  initStack(&mgr->undoStack);
  initStack(&mgr->redoStack);
}

void destroyManager(UndoRedoManager *mgr) {
  clearStack(&mgr->undoStack);
  clearStack(&mgr->redoStack);
}

int canUndo(UndoRedoManager *mgr) { return !isEmptyStack(&mgr->undoStack); }
int canRedo(UndoRedoManager *mgr) { return !isEmptyStack(&mgr->redoStack); }

void recordInsert(UndoRedoManager *mgr, EditorList *list, const char *text,
                  int len) {
  int position = list->index_cursor;
  for (int i = 0; i < len; i++) {
    insertChar(list, text[i]);
  }
  Command *cmd = (Command *)malloc(sizeof(Command));
  cmd->data = (char *)malloc(len + 1);
  strncpy(cmd->data, text, len);
  cmd->data[len] = '\0';
  cmd->position = position;
  cmd->type = ACTION_INSERT;
  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack); /* gõ mới -> xoá sạch Redo */
}

void recordDeleteChar(UndoRedoManager *mgr, EditorList *list) {
  if (list->cursor == NULL)
    return; /* không có gì để backspace */

  int position = list->index_cursor;
  int count = 0;
  Node *temp = list->cursor;

  do {
    int is_continuation = ((temp->data & 0xC0) == 0x80);
    count++;
    temp = temp->prev;
    if (!is_continuation)
      break;
  } while (temp != NULL);
  temp = list->cursor;

  char *deleted_str = (char *)malloc(count + 1);
  for (int i = count - 1; i >= 0; i--) {
    deleted_str[i] = temp->data;
    temp = temp->prev;
  }
  deleted_str[count] = '\0';
  deleteChar(list);

  Command *cmd = (Command *)malloc(sizeof(Command));
  if (cmd == NULL)
    return;
  cmd->type = ACTION_DELETE_LEFT;
  cmd->data = deleted_str;
  cmd->position = position;

  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack);
}

void recordDeleteWord(UndoRedoManager *mgr, EditorList *list) {
  if (list->cursor == NULL)
    return;
  int position = list->index_cursor;
  int count = 0;
  Node *temp = list->cursor;

  while (temp != NULL && temp->data == ' ') {
    count++;
    temp = temp->prev;
  }
  while (temp != NULL && temp->data != ' ') {
    count++;
    temp = temp->prev;
  }
  char *deleted_str = (char *)malloc(count + 1);
  deleted_str[count] = '\0';

  temp = list->cursor;
  for (int i = count - 1; i >= 0; i--) {
    deleted_str[i] = temp->data;
    temp = temp->prev;
  }
  deleteWord(list);
  Command *cmd = (Command *)malloc(sizeof(Command));
  cmd->data = deleted_str;
  cmd->type = ACTION_DELETE_LEFT;
  cmd->position = position;
  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack);
}

void recordDeleteRight(UndoRedoManager *mgr, EditorList *list) {
  Node *target = (list->cursor == NULL) ? list->head : list->cursor->next;
  if (target == NULL)
    return; /* không có ký tự bên phải để xoá */

  int position = list->index_cursor;
  int count = 0;
  Node *temp = target;
  do {
    count++;
    temp = temp->next;
    if (temp == NULL || (temp->data & 0xC0) != 0x80)
      break;
  } while (1);

  char *deleted_str = (char *)malloc(count + 1);
  deleted_str[count] = '\0';
  temp = target;
  for (int i = 0; i < count; i++) {
    deleted_str[i] = temp->data;
    temp = temp->next;
  }
  deleteRight(list);

  Command *cmd = (Command *)malloc(sizeof(Command));
  if (cmd == NULL)
    return;
  cmd->data = deleted_str;
  cmd->type = ACTION_DELETE_RIGHT;
  cmd->position = position;
  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack);
}

void undo(UndoRedoManager *mgr, EditorList *list) {
  if (isEmptyStack(&mgr->undoStack))
    return;
  Command *cmd = pop(&mgr->undoStack);

  switch (cmd->type) {
  case ACTION_INSERT: {
    /* đảo ngược: xoá ký tự vừa chèn */
    int len = strlen(cmd->data);
    moveCursorToIndex(list, cmd->position + len);
    for (int i = 0; i < len; i++) {
      if ((cmd->data[i] & 0xC0) != 0x80) {
        deleteChar(list);
      }
    }
    break;
  }
  case ACTION_DELETE_LEFT:
    /* đảo ngược: chèn lại ký tự đã backspace */
    moveCursorToIndex(list, cmd->position - strlen(cmd->data));
    for (int i = 0; cmd->data[i] != '\0'; i++) {
      insertChar(list, cmd->data[i]);
    }

    break;

  case ACTION_DELETE_RIGHT:
    /* đảo ngược: chèn lại ký tự đã delete, không di chuyển cursor */
    moveCursorToIndex(list, cmd->position);
    for (int i = 0; cmd->data[i] != '\0'; i++) {
      insertChar(list, cmd->data[i]);
    }
    moveCursorLeft(list);
    break;
  }

  push(&mgr->redoStack, cmd); /* tái sử dụng node cho Redo */
}

void redo(UndoRedoManager *mgr, EditorList *list) {
  if (isEmptyStack(&mgr->redoStack))
    return;
  Command *cmd = pop(&mgr->redoStack);

  switch (cmd->type) {
  case ACTION_INSERT:
    moveCursorToIndex(list, cmd->position);
    for (int i = 0; cmd->data[i] != '\0'; i++) {
      insertChar(list, cmd->data[i]);
    }
    break;

  case ACTION_DELETE_LEFT:
    moveCursorToIndex(list, cmd->position);
    for (int i = 0; i < strlen(cmd->data); i++) {
      if ((cmd->data[i] & 0xC0) != 0x80) {
        deleteChar(list);
      }
    }
    break;

  case ACTION_DELETE_RIGHT:
    moveCursorToIndex(list, cmd->position);
    for (int i = 0; i < strlen(cmd->data); i++) {
      if ((cmd->data[i] & 0xC0) != 0x80) {
        deleteRight(list);
      }
    }
    break;
  }
  push(&mgr->undoStack, cmd);
}

void recordDeleteRange(UndoRedoManager *mgr, EditorList *list,
                       const char *deleted_text, int len) {
  Command *cmd = (Command *)malloc(sizeof(Command));
  cmd->data = (char *)malloc(len + 1);
  strncpy(cmd->data, deleted_text, len);
  cmd->data[len] = '\0';
  cmd->position = list->index_cursor;
  cmd->type = ACTION_DELETE_LEFT;

  int target_index = list->index_cursor - len;
  while (list->index_cursor > target_index) {
    deleteChar(list);
  }
  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack);
}
