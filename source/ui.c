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

    //kiểm tra đánh thường
    if (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) {
        // add chương trình vào mà dự đoán
        
        // 1. Create a 2D array to hold the suggestions the Trie finds
        char out[MAX_SUGGESTIONS][MAX_WORD_LEN];
        
        char in[MAX_WORD_LEN] = {0};

        char clean[MAX_WORD_LEN] = {0};

        sanitize(in,clean);

        
        suggest(state->dictionary_root, clean, out);
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
    
    //tạo bộ dò phím
    GtkEventController *key_ctrl = gtk_event_controller_key_new();

    g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed), state);//gửi thông tin tới hàm crtl phím

    gtk_widget_add_controller(text_view, key_ctrl);//thêm kiểm tra vào ô điền


    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_window_present(GTK_WINDOW(window));
}