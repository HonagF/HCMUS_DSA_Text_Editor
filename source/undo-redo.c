#include <stdlib.h>
#include <string.h>
#include "undo_redo.h"


static char* str_duplicate(const char* s) {
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy != NULL) memcpy(copy, s, len);
    return copy;
}

static Command* command_create(ActionType type, const char* word, int offset) {
    Command* cmd = (Command*)malloc(sizeof(Command));
    cmd->type = type;
    cmd->word = str_duplicate(word);
    cmd->offset = offset;
    cmd->next = NULL;
    return cmd;
}

static void command_destroy(Command* cmd) {
    free(cmd->word);
    free(cmd);
}

static void buffer_insert_at(GtkTextBuffer* buffer, int offset, const char* word) {
    GtkTextIter it;
    gtk_text_buffer_get_iter_at_offset(buffer, &it, offset);
    gtk_text_buffer_insert(buffer, &it, word, -1);
}

static void buffer_delete_range(GtkTextBuffer* buffer, int offset, int len) {
    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_offset(buffer, &start, offset);
    gtk_text_buffer_get_iter_at_offset(buffer, &end, offset + len);
    gtk_text_buffer_delete(buffer, &start, &end);
}



Stack* stack_create(void) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    s->top = NULL;
    s->size = 0;
    return s;
}

void stack_push(Stack* s, Command* cmd) {
    cmd->next = s->top;
    s->top = cmd;
    s->size++;
}

Command* stack_pop(Stack* s) {
    if (s->top == NULL) return NULL;
    Command* cmd = s->top;
    s->top = cmd->next;
    cmd->next = NULL;
    s->size--;
    return cmd;
}

int stack_is_empty(const Stack* s) {
    return s->top == NULL;
}

void stack_clear(Stack* s) {
    Command* cur = s->top;
    while (cur != NULL) {
        Command* next = cur->next;
        command_destroy(cur);
        cur = next;
    }
    s->top = NULL;
    s->size = 0;
}

void stack_destroy(Stack* s) {
    stack_clear(s);
    free(s);
}



UndoRedoManager* ur_create(GtkTextBuffer* buffer) {
    UndoRedoManager* mgr = (UndoRedoManager*)malloc(sizeof(UndoRedoManager));
    mgr->buffer = buffer;
    mgr->undo_stack = stack_create();
    mgr->redo_stack = stack_create();
    mgr->pending_len = 0;
    mgr->pending_offset = 0;
    mgr->pending_type = ACTION_INSERT;
    mgr->has_pending = 0;
    mgr->pending_word[0] = '\0';
    return mgr;
}

void ur_destroy(UndoRedoManager* mgr) {
    stack_destroy(mgr->undo_stack);
    stack_destroy(mgr->redo_stack);
    free(mgr); /* không free mgr->buffer — GTK sở hữu nó */
}

void ur_notify_insert(UndoRedoManager* mgr, int offset, char c) {
    /* ký tự này có nối liền vào pending word đang gõ không? */
    if (mgr->has_pending &&
        mgr->pending_type == ACTION_INSERT &&
        offset == mgr->pending_offset + mgr->pending_len &&
        mgr->pending_len < UR_MAX_WORD_LEN - 1) {
        mgr->pending_word[mgr->pending_len++] = c;
        mgr->pending_word[mgr->pending_len] = '\0';
        return;
    }

    /* không nối liền -> chốt pending cũ (nếu có), bắt đầu từ mới */
    ur_flush(mgr);
    mgr->pending_type = ACTION_INSERT;
    mgr->pending_offset = offset;
    mgr->pending_word[0] = c;
    mgr->pending_word[1] = '\0';
    mgr->pending_len = 1;
    mgr->has_pending = 1;
}

void ur_notify_delete(UndoRedoManager* mgr, int offset, char c) {
    /* Backspace xóa từ phải sang trái -> ký tự mới luôn đứng NGAY TRƯỚC
     * đầu pending word hiện tại, nên phải chèn vào đầu (prepend). */
    if (mgr->has_pending &&
        mgr->pending_type == ACTION_DELETE &&
        offset == mgr->pending_offset - 1 &&
        mgr->pending_len < UR_MAX_WORD_LEN - 1) {
        memmove(mgr->pending_word + 1, mgr->pending_word, mgr->pending_len + 1);
        mgr->pending_word[0] = c;
        mgr->pending_len++;
        mgr->pending_offset = offset;
        return;
    }

    ur_flush(mgr);
    mgr->pending_type = ACTION_DELETE;
    mgr->pending_offset = offset;
    mgr->pending_word[0] = c;
    mgr->pending_word[1] = '\0';
    mgr->pending_len = 1;
    mgr->has_pending = 1;
}

void ur_flush(UndoRedoManager* mgr) {
    if (!mgr->has_pending || mgr->pending_len == 0) {
        mgr->has_pending = 0;
        return;
    }

    Command* cmd = command_create(mgr->pending_type, mgr->pending_word, mgr->pending_offset);
    stack_push(mgr->undo_stack, cmd);

    /* Yêu cầu bắt buộc của đề: có hành động mới -> xóa sạch Redo Stack */
    stack_clear(mgr->redo_stack);

    mgr->has_pending = 0;
    mgr->pending_len = 0;
    mgr->pending_word[0] = '\0';
}

int ur_undo(UndoRedoManager* mgr) {
    /* chốt nốt từ đang gõ/xóa dở trước khi undo, tránh mất thao tác */
    ur_flush(mgr);

    if (stack_is_empty(mgr->undo_stack)) return 0;

    Command* cmd = stack_pop(mgr->undo_stack);
    if (cmd->type == ACTION_INSERT) {
        /* đảo ngược INSERT = xóa từ đó khỏi buffer */
        buffer_delete_range(mgr->buffer, cmd->offset, (int)strlen(cmd->word));
    } else {
        /* đảo ngược DELETE = chèn từ đó trở lại đúng vị trí cũ */
        buffer_insert_at(mgr->buffer, cmd->offset, cmd->word);
    }

    /* Command Pattern: đẩy nguyên Command sang Redo Stack, type không đổi */
    stack_push(mgr->redo_stack, cmd);
    return 1;
}

int ur_redo(UndoRedoManager* mgr) {
    if (stack_is_empty(mgr->redo_stack)) return 0;

    Command* cmd = stack_pop(mgr->redo_stack);
    if (cmd->type == ACTION_INSERT) {
        buffer_insert_at(mgr->buffer, cmd->offset, cmd->word);
    } else {
        buffer_delete_range(mgr->buffer, cmd->offset, (int)strlen(cmd->word));
    }

    stack_push(mgr->undo_stack, cmd);
    return 1;
}
