#include <gtk/gtk.h>
#include <stdlib.h>
#include "api.h"
#include "ui.h" // thư viện các thứ
#include "autocomplete.h"

static void activate(GtkApplication *app, gpointer user_data) {
    // gửi thông tin về struct
    EditorState *state = (EditorState *)user_data;

    setup_ui(state,app);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // cung bộ nhớ
    EditorState *state = malloc(sizeof(EditorState));
    //state->undo_stack_top = NULL;   
    state->dictionary_root = NULL;  // khởi tạo cho auto

    app = gtk_application_new("org.gtk.dsa.editor", G_APPLICATION_DEFAULT_FLAGS);
     
    g_signal_connect(app, "activate", G_CALLBACK(activate), state);// địa chỉ gửi,điều kiện bật,gọi hàm, thông tin cần gửi
    
    status = g_application_run(G_APPLICATION(app), argc, argv);// lấy thong tin khi chạy ứng dụng
    
    g_object_unref(app);
    free(state); // xóa khi ko còn dùng
    return status;
}