#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <gtk/gtk.h>

#define UR_MAX_WORD_LEN 256


typedef enum {
    ACTION_INSERT,
    ACTION_DELETE
} ActionType;

typedef struct Command {
    ActionType type;
    char* word;
    int offset;              
    struct Command* next;    
} Command;

typedef struct Stack {
    Command* top;
    int size;
} Stack;

Stack*   stack_create(void);
void     stack_push(Stack* s, Command* cmd);
Command* stack_pop(Stack* s);
int      stack_is_empty(const Stack* s);
void     stack_clear(Stack* s);
void     stack_destroy(Stack* s);

typedef struct UndoRedoManager {
    GtkTextBuffer* buffer;
    Stack* undo_stack;
    Stack* redo_stack;

    char pending_word[UR_MAX_WORD_LEN];
    int  pending_len;
    int  pending_offset;
    ActionType pending_type;
    int  has_pending;
} UndoRedoManager;

UndoRedoManager* ur_create(GtkTextBuffer* buffer);
void ur_destroy(UndoRedoManager* mgr);

void ur_notify_insert(UndoRedoManager* mgr, int offset, char c);

void ur_notify_delete(UndoRedoManager* mgr, int offset, char c);

void ur_flush(UndoRedoManager* mgr);

int ur_undo(UndoRedoManager* mgr);

int ur_redo(UndoRedoManager* mgr);

#endif 
