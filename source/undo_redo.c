#include "undo_redo.h"
#include <stdlib.h>

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

int isEmptyStack(CommandStack *stack) {
  return stack->size == 0;
}

void clearStack(CommandStack *stack) {
  Command *cur = stack->top;
  while (cur != NULL) {
    Command *tmp = cur;
    cur = cur->next;
    free(tmp);
  }
  stack->top = NULL;
  stack->size = 0;
}

static void moveCursorToIndex(EditorList *list, int target) {
  while (list->index_cursor < target)
    moveCursorRight(list);
  while (list->index_cursor > target)
    moveCursorLeft(list);
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

void recordInsert(UndoRedoManager *mgr, EditorList *list, char c) {
  int position = list->index_cursor;

  insertChar(list, c);

  Command *cmd = (Command *)malloc(sizeof(Command));
  if (cmd == NULL) return;
  cmd->type = ACTION_INSERT;
  cmd->data = c;
  cmd->position = position;

  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack); /* gõ mới -> xoá sạch Redo */
}

void recordDeleteChar(UndoRedoManager *mgr, EditorList *list) {
  if (list->cursor == NULL) return; /* không có gì để backspace */

  char data = list->cursor->data;
  int position = list->index_cursor;

  deleteChar(list);

  Command *cmd = (Command *)malloc(sizeof(Command));
  if (cmd == NULL) return;
  cmd->type = ACTION_DELETE_LEFT;
  cmd->data = data;
  cmd->position = position;

  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack);
}

void recordDeleteRight(UndoRedoManager *mgr, EditorList *list) {
  Node *target = (list->cursor == NULL) ? list->head : list->cursor->next;
  if (target == NULL) return; /* không có ký tự bên phải để xoá */

  char data = target->data;
  int position = list->index_cursor;

  deleteRight(list);

  Command *cmd = (Command *)malloc(sizeof(Command));
  if (cmd == NULL) return;
  cmd->type = ACTION_DELETE_RIGHT;
  cmd->data = data;
  cmd->position = position;

  push(&mgr->undoStack, cmd);
  clearStack(&mgr->redoStack);
}

void undo(UndoRedoManager *mgr, EditorList *list) {
  if (isEmptyStack(&mgr->undoStack)) return;
  Command *cmd = pop(&mgr->undoStack);

  switch (cmd->type) {
    case ACTION_INSERT:
      /* đảo ngược: xoá ký tự vừa chèn */
      moveCursorToIndex(list, cmd->position + 1);
      deleteChar(list);
      break;

    case ACTION_DELETE_LEFT:
      /* đảo ngược: chèn lại ký tự đã backspace */
      moveCursorToIndex(list, cmd->position - 1);
      insertChar(list, cmd->data);
      break;

    case ACTION_DELETE_RIGHT:
      /* đảo ngược: chèn lại ký tự đã delete, không di chuyển cursor */
      moveCursorToIndex(list, cmd->position);
      insertChar(list, cmd->data);
      moveCursorLeft(list);
      break;
  }

  push(&mgr->redoStack, cmd); /* tái sử dụng node cho Redo */
}

void redo(UndoRedoManager *mgr, EditorList *list) {
  if (isEmptyStack(&mgr->redoStack)) return;
  Command *cmd = pop(&mgr->redoStack);

  switch (cmd->type) {
    case ACTION_INSERT:
      moveCursorToIndex(list, cmd->position);
      insertChar(list, cmd->data);
      break;

    case ACTION_DELETE_LEFT:
      moveCursorToIndex(list, cmd->position);
      deleteChar(list);
      break;

    case ACTION_DELETE_RIGHT:
      moveCursorToIndex(list, cmd->position);
      deleteRight(list);
      break;
  }

  push(&mgr->undoStack, cmd);
}
