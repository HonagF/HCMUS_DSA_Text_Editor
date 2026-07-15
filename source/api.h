#ifndef API_H
#define API_H

#include <gtk/gtk.h>
#include "list.h"
// Forward declarations of your custom DSA structs (defined in your dsa headers)
//typedef struct UndoNode UndoNode;
typedef struct TrieNode TrieNode;

// The Master Struct
typedef struct {
    GtkTextBuffer *buffer;      // GTK handles the text display
    //UndoNode *undo_stack_top;   
    TrieNode *dictionary_root;  // You handle the autocomplete
    EditorList *list;
    int is_updating_gtk;
} EditorState;

#endif