#include "api.h"
#include "autocomplete.h"
#include "list.h"
#include "ui.h"
#include "undo_redo.h"
#include <gtk/gtk.h>
#include <stdlib.h>

static void activate(GtkApplication *app, gpointer user_data) {
  // gửi thông tin về struct
  EditorState *state = (EditorState *)user_data;

  setup_ui(state, app);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  // cung bộ nhớ
  EditorState *state = malloc(sizeof(EditorState));
  state->is_updating_gtk = 0;
  // state->undo_stack_top = NULL;
  state->dictionary_root = create_node('\0'); // khởi tạo cho auto
  load_txt(state->dictionary_root,
           "../data/google-10000-english-no-swears.txt");
  load_txt(state->dictionary_root, "../data/Viet11K.txt");

  state->list = malloc(sizeof(EditorList));
  initList(state->list);

  state->manager = (UndoRedoManager *)malloc(sizeof(UndoRedoManager));
  initManager(state->manager);

  app = gtk_application_new("org.gtk.dsa.editor", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(
      app, "activate", G_CALLBACK(activate),
      state); // địa chỉ gửi,tên tín hiệu ,gọi hàm, thông tin cần gửi

  status = g_application_run(G_APPLICATION(app), argc,
                             argv); // lấy thong tin khi chạy ứng dụng

  g_object_unref(app);
  destroyManager(state->manager);
  free(state->manager);
  destroyList(state->list);
  free(state->list);
  free(state); // xóa khi ko còn dùng
  return status;
}