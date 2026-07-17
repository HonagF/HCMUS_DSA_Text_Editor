#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include "api.h" // Needed so it knows what EditorState is

// We just declare the function here. No { brackets } or actual code.
void setup_ui(EditorState *state, GtkApplication *app);

#endif