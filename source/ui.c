#include "ui.h"                 // Always include its own header
#include "autocomplete.h"
#include <gdk/gdk.h>
#include <string.h>
#include <gtk/gtk.h>

// -------------------- Callbacks cho các nút chức năng --------------------
static void on_undo_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    // TODO: Gọi hàm undo (sẽ thêm sau)
    g_print("Undo clicked\n");
}

static void on_redo_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    // TODO: Gọi hàm redo
    g_print("Redo clicked\n");
}

static void on_clear_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    gtk_text_buffer_set_text(state->buffer, "", -1);
}

// Các hàm Cut/Copy/Paste sử dụng clipboard của GTK4
static void on_cut_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    GdkClipboard *clip = gdk_display_get_clipboard(gdk_display_get_default());
    gtk_text_buffer_cut_clipboard(state->buffer, clip, TRUE);
}

static void on_copy_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    GdkClipboard *clip = gdk_display_get_clipboard(gdk_display_get_default());
    gtk_text_buffer_copy_clipboard(state->buffer, clip);
}

static void on_paste_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    GdkClipboard *clip = gdk_display_get_clipboard(gdk_display_get_default());
    gtk_text_buffer_paste_clipboard(state->buffer, clip, NULL, TRUE);
}

// -------------------- Xử lý phím (đã thêm Ctrl+Z và Ctrl+Y) --------------------
static gboolean on_key_pressed(GtkEventControllerKey *controller,
                               guint keyval,
                               guint keycode,
                               GdkModifierType modifiers,
                               gpointer user_data) 
{
    EditorState *state = (EditorState *)user_data;

    // Phím tắt Undo/Redo
    if ((modifiers & GDK_CONTROL_MASK) && keyval == GDK_KEY_z) {
        on_undo_clicked(NULL, state);
        return TRUE;
    }
    if ((modifiers & GDK_CONTROL_MASK) && keyval == GDK_KEY_y) {
        on_redo_clicked(NULL, state);
        return TRUE;
    }

    if (keyval == GDK_KEY_Tab) {
        GtkTextIter start_bound, end_bound;
        gtk_text_buffer_get_bounds(state->buffer, &start_bound, &end_bound);
        gtk_text_buffer_remove_tag_by_name(state->buffer, "ghost_text", &start_bound, &end_bound);
        gtk_text_buffer_place_cursor(state->buffer, &end_bound);
        return TRUE;
    }

    // Xóa ghost_text nếu có
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(state->buffer);
    GtkTextTag *ghost_tag = gtk_text_tag_table_lookup(tag_table, "ghost_text");
    if (ghost_tag != NULL) {
        GtkTextIter iter;
        gtk_text_buffer_get_start_iter(state->buffer, &iter);
        while (gtk_text_iter_forward_to_tag_toggle(&iter, ghost_tag)) {
            GtkTextIter del_start = iter;
            gtk_text_iter_forward_to_tag_toggle(&iter, ghost_tag); 
            gtk_text_buffer_delete(state->buffer, &del_start, &iter);
            gtk_text_buffer_get_start_iter(state->buffer, &iter);
        }
    }

    // Tự động gợi ý khi gõ chữ cái
    if (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) {
        char out[MAX_SUGGESTIONS][MAX_WORD_LEN] = {0};
        char in[MAX_WORD_LEN] = {0};
        char clean[MAX_WORD_LEN] = {0};
        
        GtkTextIter iter, start;
        gtk_text_buffer_get_iter_at_mark(state->buffer, &iter, gtk_text_buffer_get_insert(state->buffer));
        start = iter;
        while (gtk_text_iter_backward_char(&start)) {
            gunichar c = gtk_text_iter_get_char(&start);
            if (g_unichar_isspace(c)) {
                gtk_text_iter_forward_char(&start);
                break;
            }
        }   

        char *existing_text = gtk_text_buffer_get_text(state->buffer, &start, &iter, FALSE);
        if (existing_text != NULL) {
            snprintf(in, MAX_WORD_LEN, "%s%c", existing_text, (char)keyval);
            g_free(existing_text);
        } else {
            snprintf(in, MAX_WORD_LEN, "%c", (char)keyval);
        }
        sanitize(in, clean);

        int num_suggestions = suggest(state->dictionary_root, clean, out);
        if (num_suggestions > 0) {
            GtkTextIter insert_iter;
            gtk_text_buffer_get_iter_at_mark(state->buffer, &insert_iter, gtk_text_buffer_get_insert(state->buffer));
            char typed_str[2] = {(char)keyval, '\0'};
            gtk_text_buffer_insert(state->buffer, &insert_iter, typed_str, -1);

            int input_length = strlen(clean);
            const char *remainder = out[0] + input_length; 
            gtk_text_buffer_insert(state->buffer, &insert_iter, remainder, -1);

            GtkTextIter start_grey = insert_iter;
            gtk_text_iter_backward_chars(&start_grey, strlen(remainder));
            gtk_text_buffer_apply_tag_by_name(state->buffer, "ghost_text", &start_grey, &insert_iter);
            return TRUE; 
        }
        return FALSE;
    }
    return FALSE;
}

// -------------------- Áp dụng CSS (dùng load_from_string) --------------------
static void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background-color: #1e1e1e; }"
        "textview { background-color: #2d2d30; color: #ffffff; font-family: 'Monospace', 'Courier New', monospace; font-size: 14pt; padding: 12px; }"
        "textview text { background-color: #2d2d30; color: #ffffff; }"
        "scrollbar { background-color: #1e1e1e; }"
        "scrollbar slider { background-color: #4a4a4a; border-radius: 10px; border: none; min-width: 12px; min-height: 12px; }"
        "scrollbar slider:hover { background-color: #6a6a6a; }"
        /* Toolbar styles */
        ".toolbar { background-color: #252526; padding: 4px 8px; border-bottom: 1px solid #3a3a3a; }"
        ".toolbar button {"
        "  background-color: #3a3a3a;"
        "  color: #ffffff;"
        "  border: 1px solid #555555;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  margin: 2px;"
        "  font-size: 12pt;"
        "}"
        ".toolbar button:hover {"
        "  background-color: #5a5a5a;"
        "  border-color: #888888;"
        "}"
        ".toolbar button:active {"
        "  background-color: #2a2a2a;"
        "  border-color: #aaaaaa;"
        "}"
        /* Style cho icon bên trong nút */
        ".toolbar button image {"
        "  color: #ffffff;"
        "}"
        ".toolbar button:hover image {"
        "  color: #ffffff;"
        "}"
        ".toolbar button:active image {"
        "  color: #aaaaaa;"
        "}";
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

// -------------------- Xây dựng giao diện chính (có thanh công cụ) --------------------
void setup_ui(EditorState *state, GtkApplication *app) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Pad");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    apply_css(window);

    // ---- Tạo thanh công cụ ----
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    // Thêm style class "toolbar" để CSS nhận diện
    gtk_widget_add_css_class(toolbar, "toolbar");
    gtk_widget_set_halign(toolbar, GTK_ALIGN_FILL);
    gtk_widget_set_margin_start(toolbar, 0);
    gtk_widget_set_margin_end(toolbar, 0);
    gtk_widget_set_margin_top(toolbar, 0);
    gtk_widget_set_margin_bottom(toolbar, 0);

    // Nút Undo
    GtkWidget *undo_btn = gtk_button_new_from_icon_name("edit-undo-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(undo_btn), FALSE);
    gtk_widget_set_tooltip_text(undo_btn, "Undo (Ctrl+Z)");
    g_signal_connect(undo_btn, "clicked", G_CALLBACK(on_undo_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), undo_btn);

    // Nút Redo
    GtkWidget *redo_btn = gtk_button_new_from_icon_name("edit-redo-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(redo_btn), FALSE);
    gtk_widget_set_tooltip_text(redo_btn, "Redo (Ctrl+Y)");
    g_signal_connect(redo_btn, "clicked", G_CALLBACK(on_redo_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), redo_btn);

    // Separator
    GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_append(GTK_BOX(toolbar), sep1);

    // Nút Cut
    GtkWidget *cut_btn = gtk_button_new_from_icon_name("edit-cut-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(cut_btn), FALSE);
    gtk_widget_set_tooltip_text(cut_btn, "Cut");
    g_signal_connect(cut_btn, "clicked", G_CALLBACK(on_cut_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), cut_btn);

    // Nút Copy
    GtkWidget *copy_btn = gtk_button_new_from_icon_name("edit-copy-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(copy_btn), FALSE);
    gtk_widget_set_tooltip_text(copy_btn, "Copy");
    g_signal_connect(copy_btn, "clicked", G_CALLBACK(on_copy_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), copy_btn);

    // Nút Paste
    GtkWidget *paste_btn = gtk_button_new_from_icon_name("edit-paste-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(paste_btn), FALSE);
    gtk_widget_set_tooltip_text(paste_btn, "Paste");
    g_signal_connect(paste_btn, "clicked", G_CALLBACK(on_paste_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), paste_btn);

    // Separator
    GtkWidget *sep2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_append(GTK_BOX(toolbar), sep2);

    // Nút Clear
    GtkWidget *clear_btn = gtk_button_new_from_icon_name("edit-delete-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(clear_btn), FALSE);
    gtk_widget_set_tooltip_text(clear_btn, "Clear all text");
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), clear_btn);

    // ---- Tạo ScrolledWindow và TextView ----
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scrolled_window), TRUE);
    gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scrolled_window), TRUE);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 15);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 15);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), TRUE);

    state->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_create_tag(state->buffer, "ghost_text", "foreground", "#888888", NULL);

    GtkEventController *key_ctrl = gtk_event_controller_key_new();
    g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed), state);
    gtk_widget_add_controller(text_view, key_ctrl);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);

    // ---- Bố trí tổng thể ----
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(main_box), toolbar);
    gtk_box_append(GTK_BOX(main_box), scrolled_window);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    gtk_window_present(GTK_WINDOW(window));
}