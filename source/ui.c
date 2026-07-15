#include "ui.h" // Always include its own header
#include "autocomplete.h"
#include <gdk/gdk.h> 

static gboolean on_key_pressed(GtkEventControllerKey *controller,
                               guint keyval,
                               guint keycode,
                               GdkModifierType modifiers,
                               gpointer user_data) 
{
    EditorState *state = (EditorState *)user_data; 
    if (keyval == GDK_KEY_Tab) {
        GtkTextIter start_bound, end_bound;
        gtk_text_buffer_get_bounds(state->buffer, &start_bound, &end_bound);
    
        // Remove the gray tag, making the text permanent and normal
        gtk_text_buffer_remove_tag_by_name(state->buffer, "ghost_text", &start_bound, &end_bound);
    
        // Move the cursor to the end of the accepted word
        gtk_text_buffer_place_cursor(state->buffer, &end_bound);
    
        return TRUE; // Stop the Tab key from actually typing a tab space
    }

    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(state->buffer);
    GtkTextTag *ghost_tag = gtk_text_tag_table_lookup(tag_table, "ghost_text");

    if (ghost_tag != NULL) {
        GtkTextIter iter;
        gtk_text_buffer_get_start_iter(state->buffer, &iter);
    
    // Hunt down the start of the gray text
        while (gtk_text_iter_forward_to_tag_toggle(&iter, ghost_tag)) {
            GtkTextIter del_start = iter;
        
            // Find the end of the gray text
            gtk_text_iter_forward_to_tag_toggle(&iter, ghost_tag); 
        
            // Delete it from the screen
            gtk_text_buffer_delete(state->buffer, &del_start, &iter);
        
            // The buffer changed, so we must reset our search iterator
            gtk_text_buffer_get_start_iter(state->buffer, &iter);
        }
    }

    //kiểm tra đánh thường
    if (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) {
        // add chương trình vào mà dự đoán
        
        // 1. Create a 2D array to hold the suggestions the Trie finds
        char out[MAX_SUGGESTIONS][MAX_WORD_LEN]={0};
        char in[MAX_WORD_LEN] = {0};
        char clean[MAX_WORD_LEN] = {0};
        
        GtkTextIter iter, start;
        gtk_text_buffer_get_iter_at_mark(state->buffer, &iter, gtk_text_buffer_get_insert(state->buffer));
        start = iter;

        while (gtk_text_iter_backward_char(&start)) {
        gunichar c = gtk_text_iter_get_char(&start);
            if (g_unichar_isspace(c)) {
            gtk_text_iter_forward_char(&start); // Move forward one to exclude the space itself
            break;
            }
        }   

        // 4. Extract the text already on the screen (e.g., "ap")
        char *existing_text = gtk_text_buffer_get_text(state->buffer, &start, &iter, FALSE);

        // 5. Combine the existing text with the new key pressed (e.g., "ap" + "p" = "app")
        if (existing_text != NULL) {
            snprintf(in, MAX_WORD_LEN, "%s%c", existing_text, (char)keyval);
            g_free(existing_text); // Always free memory provided by GTK!
        }else {
            // If there was no existing text, the prefix is just the key pressed
            snprintf(in, MAX_WORD_LEN, "%c", (char)keyval);
        }

        sanitize(in,clean);

        // 2. Capture the actual number of suggestions found
        int num_suggestions = suggest(state->dictionary_root, clean, out);

        // 3. Only proceed if the Trie actually found a matching word
        if (num_suggestions > 0) {
    
            // a. Get the cursor position
            GtkTextIter insert_iter;
            gtk_text_buffer_get_iter_at_mark(state->buffer, &insert_iter, gtk_text_buffer_get_insert(state->buffer));

            // b. CRITICAL FIX: Manually insert the key the user JUST typed first!
            char typed_str[2] = {(char)keyval, '\0'};
            gtk_text_buffer_insert(state->buffer, &insert_iter, typed_str, -1);

            // c. Calculate the ghost text
            int input_length = strlen(clean);
            const char *remainder = out[0] + input_length; 

            // d. Insert the text first (this moves insert_iter to the very end of the word)
            gtk_text_buffer_insert(state->buffer, &insert_iter, remainder, -1);

            // CRITICAL FIX: Recalculate start_grey by moving backwards!
            GtkTextIter start_grey = insert_iter;
            gtk_text_iter_backward_chars(&start_grey, strlen(remainder));

            // Now apply the tag using the fresh, valid iterators
            gtk_text_buffer_apply_tag_by_name(state->buffer, "ghost_text", &start_grey, &insert_iter);

            // e. CRITICAL FIX: Return TRUE so GTK doesn't type the key a second time!
            return TRUE; 
        }
        return FALSE; // trả false để hiển thị
    }
    return FALSE; // cho hiển thị các kí tự ko quan tâm
}

// Here is the actual code that builds the GUI
void setup_ui(EditorState *state, GtkApplication *app) {

    GtkWidget *window = gtk_application_window_new(app);//gọi biến cửa sổ

    gtk_window_set_title (GTK_WINDOW (window), "Pad"); // đặt tên tiêu đề ngu!
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600); // kích thước window
    
    
    GtkWidget *scrolled_window = gtk_scrolled_window_new();// tạo cửa sổ lướt đc
    gtk_window_set_child (GTK_WINDOW (window), scrolled_window); //cho cửa sổ mới tạo làm con, đặt vào cửa sổ mẹ
    
    GtkWidget *text_view = gtk_text_view_new();// để điền kí tự

    // liên kết ô điền với struct
    state->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));// tương tự với lướt
    gtk_text_buffer_create_tag(state->buffer, "ghost_text","foreground", "#888888", NULL);

    //tạo bộ dò phím
    GtkEventController *key_ctrl = gtk_event_controller_key_new();

    g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed), state);//gửi thông tin tới hàm crtl phím

    gtk_widget_add_controller(text_view, key_ctrl);//thêm kiểm tra vào ô điền

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_window_present(GTK_WINDOW(window));
}