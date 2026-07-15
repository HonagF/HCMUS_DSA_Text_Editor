#ifndef UNDOREDO_H
#define UNDOREDO_H

#include "list.h"

typedef enum {
  ACTION_INSERT,
  ACTION_DELETE_LEFT,  /* backspace: xoá ký tự tại cursor */
  ACTION_DELETE_RIGHT  /* delete: xoá ký tự bên phải cursor */
} ActionType;

typedef struct Command {
  ActionType type;
  char data;           /* ký tự liên quan đến hành động */
  int position;         /* index_cursor TRƯỚC khi hành động xảy ra */
  struct Command *next;
} Command;

typedef struct CommandStack {
  Command *top;
  int size;
} CommandStack;

typedef struct UndoRedoManager {
  CommandStack undoStack;
  CommandStack redoStack;
} UndoRedoManager;

/* --- Stack primitives (dùng nếu cần test riêng) --- */
void initStack(CommandStack *stack);
void push(CommandStack *stack, Command *cmd);
Command *pop(CommandStack *stack);
int isEmptyStack(CommandStack *stack);
void clearStack(CommandStack *stack);

/* --- Manager API --- */
void initManager(UndoRedoManager *mgr);
void destroyManager(UndoRedoManager *mgr);

/* Gọi các hàm này THAY CHO insertChar/deleteChar/deleteRight trực tiếp,
   vì chúng vừa thực thi hành động vừa ghi lại Command để undo/redo. */
void recordInsert(UndoRedoManager *mgr, EditorList *list, char c);
void recordDeleteChar(UndoRedoManager *mgr, EditorList *list);
void recordDeleteRight(UndoRedoManager *mgr, EditorList *list);

void undo(UndoRedoManager *mgr, EditorList *list);
void redo(UndoRedoManager *mgr, EditorList *list);

int canUndo(UndoRedoManager *mgr);
int canRedo(UndoRedoManager *mgr);

#endif
