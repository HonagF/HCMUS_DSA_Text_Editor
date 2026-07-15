#include "ui.h"
#include "autocomplete.h"
#include "list.h"
#include "undo_redo.h"
#include <gdk/gdk.h>
#include <string.h>

static char *get_rec_text_buff(GtkTextBuffer *buffer) {
  GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);
  GtkTextTag *ghost_tag = gtk_text_tag_table_lookup(tag_table, "ghost_text");
  if (ghost_tag == NULL)
    return NULL; // Không có rec tồn tại
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buffer, &start); // Bắt đầu tìm từ đầu văn bản
  // Nhảy con trỏ tới vị trí BẮT ĐẦU của đoạn chữ xám
  if (gtk_text_iter_forward_to_tag_toggle(&start, ghost_tag)) {
    end = start;
    // Nhảy con trỏ tới vị trí KẾT THÚC của đoạn chữ xám
    gtk_text_iter_forward_to_tag_toggle(&end, ghost_tag);
    // Trích xuất chữ và trả về (Lưu ý: GTK sẽ cấp phát bộ nhớ cho chuỗi này)
    return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  }
  return NULL;
}

// Hàm tạm thời duyệt Danh sách liên kết và gom thành một chuỗi ký tự
static char *get_text_from_list(EditorList *list) {
  // Nếu danh sách rỗng, trả về một chuỗi rỗng để GTK không in ra rác
  if (list == NULL || list->size == 0) {
    char *empty_str = g_malloc(1); // Dùng g_malloc của GTK
    empty_str[0] = '\0';
    return empty_str;
  }

  // Cấp phát bộ nhớ: Kích thước của chuỗi bằng số lượng node + 1 ký tự kết thúc
  // '\0'
  char *full_text = g_malloc(list->size + 1);
  int i = 0;

  // Duyệt từ Node đầu tiên (head) đến cuối danh sách
  Node *current = list->head;
  while (current != NULL) {
    full_text[i] = current->data; // Lấy ký tự từ node
    current = current->next;      // Nhảy sang node tiếp theo
    i++;
  }

  // Đóng chuỗi lại chuẩn C
  full_text[i] = '\0';

  return full_text;
}

// Hàm này dò ngược từ vị trí con trỏ hiện tại để lấy từ đang gõ dở
static char *get_current_word_from_list(EditorList *list) {
  // Nếu danh sách rỗng hoặc con trỏ không trỏ vào đâu, trả về NULL
  if (list == NULL || list->cursor == NULL)
    return NULL;

  int len = 0;
  Node *temp = list->cursor;

  // BƯỚC 1: Đếm độ dài của từ
  // Đi lùi (temp->prev) cho đến khi gặp khoảng trắng, xuống dòng hoặc chạm đầu
  // list
  while (temp != NULL && temp->data != ' ' && temp->data != '\n') {
    len++;
    temp = temp->prev;
  }

  // Nếu độ dài = 0 (tức là con trỏ đang đứng ngay tại khoảng trắng), không có
  // từ nào để auto-complete
  if (len == 0)
    return NULL;

  // BƯỚC 2: Cấp phát bộ nhớ cho chuỗi
  char *word =
      g_malloc(len + 1); // Cấp phát thêm 1 byte cho ký tự kết thúc chuỗi '\0'
  word[len] = '\0';

  // BƯỚC 3: Điền các ký tự vào chuỗi
  // Vì ta đang đi lùi từ đuôi của từ, ta phải điền vào mảng từ vị trí cuối lên
  // vị trí đầu (len - 1 về 0)
  temp = list->cursor;
  for (int i = len - 1; i >= 0; i--) {
    word[i] = temp->data;
    temp = temp->prev;
  }

  return word; // Trả về chuỗi chứa từ hiện tại
}

static void update_autocomplete(EditorState *state) {
  char *cur = get_current_word_from_list(state->list);
  if (cur != NULL && strlen(cur) > 0) {
    char out[MAX_SUGGESTIONS][MAX_WORD_LEN] = {0};
    int num_suggestions = suggest(state->dictionary_root, cur, out);

    if (num_suggestions > 0) {
      GtkTextIter insert_iter;
      gtk_text_buffer_get_iter_at_mark(
          state->buffer, &insert_iter,
          gtk_text_buffer_get_insert(state->buffer));

      int input_length = strlen(cur);
      const char *remainder = out[0] + input_length;

      state->is_updating_gtk = 1; // Ngăn chặn on_insert_text bắt chữ xám
      gtk_text_buffer_insert(state->buffer, &insert_iter, remainder, -1);
      state->is_updating_gtk = 0; // Mở lại

      GtkTextIter start_grey = insert_iter;
      // Dùng g_utf8_strlen thay vì strlen vì GTK đếm theo số lượng KÝ TỰ
      // (character), không phải số byte (strlen)
      long num_chars = g_utf8_strlen(remainder, -1);
      gtk_text_iter_backward_chars(&start_grey, num_chars);

      gtk_text_buffer_apply_tag_by_name(state->buffer, "ghost_text",
                                        &start_grey, &insert_iter);
    }
    g_free(cur);
  }
}

static void sync_gtk_with_list(EditorState *state) {
  state->is_updating_gtk = 1;

  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(state->buffer, &start, &end);
  gtk_text_buffer_delete(state->buffer, &start, &end);
  char *full_text = get_text_from_list(state->list);

  if (full_text != NULL) {
    gtk_text_buffer_set_text(state->buffer, full_text, -1);
    long char_offset = g_utf8_pointer_to_offset(
        full_text, full_text + state->list->index_cursor);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset(state->buffer, &iter, char_offset);
    gtk_text_buffer_place_cursor(state->buffer, &iter);
    g_free(full_text);
  }

  state->is_updating_gtk = 0;
}

static gboolean deferred_sync_and_autocomplete(gpointer user_data) {
  EditorState *state = (EditorState *)user_data;
  sync_gtk_with_list(state);
  update_autocomplete(state);
  return G_SOURCE_REMOVE;
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType modifiers,
                               gpointer user_data) {
  EditorState *state = (EditorState *)user_data;
  gboolean handled = FALSE;

  if (keyval == GDK_KEY_z && (modifiers & GDK_CONTROL_MASK)) {
    if (canUndo(state->manager)) {
      undo(state->manager, state->list);
      sync_gtk_with_list(state);
      update_autocomplete(state);
    }
    return TRUE;
  }
  if (keyval == GDK_KEY_y && (modifiers & GDK_CONTROL_MASK)) {
    if (canRedo(state->manager)) {
      redo(state->manager, state->list);
      sync_gtk_with_list(state);
      update_autocomplete(state);
    }
    return TRUE;
  }

  if (keyval == GDK_KEY_BackSpace) {
    if (modifiers & GDK_CONTROL_MASK)
      recordDeleteWord(state->manager, state->list);
    else
      recordDeleteChar(state->manager, state->list);
    handled = TRUE;
  } else if (keyval == GDK_KEY_Left) {
    if (modifiers & GDK_CONTROL_MASK)
      moveCursorPrevWord(state->list);
    else
      moveCursorLeft(state->list);
    handled = TRUE;
  } else if (keyval == GDK_KEY_Right) {
    if (modifiers & GDK_CONTROL_MASK)
      moveCursorNextWord(state->list);
    else
      moveCursorRight(state->list);
    handled = TRUE;
  } else if (gdk_keyval_to_unicode(keyval) != 0 &&
             !(modifiers & GDK_CONTROL_MASK) && keyval != GDK_KEY_Return &&
             keyval != GDK_KEY_KP_Enter && keyval != GDK_KEY_Tab &&
             keyval != GDK_KEY_Escape) {
    guint32 unicode = gdk_keyval_to_unicode(keyval);
    char utf8[7] = {0};
    int bytes = g_unichar_to_utf8(unicode, utf8);
    recordInsert(state->manager, state->list, utf8, bytes);
    handled = TRUE;
  } else if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
    insertChar(state->list, '\n');
    handled = TRUE;
  } else if (keyval == GDK_KEY_Down) {
    moveCursorDown(state->list);
    handled = TRUE;
  } else if (keyval == GDK_KEY_Up) {
    moveCursorUp(state->list);
    handled = TRUE;
  } else if (keyval == GDK_KEY_Tab) {
    char *cur = get_current_word_from_list(state->list);
    if (cur != NULL && strlen(cur) > 0) {
      char out[MAX_SUGGESTIONS][MAX_WORD_LEN] = {0};
      int num_suggestions = suggest(state->dictionary_root, cur, out);
      if (num_suggestions > 0) {
        int input_length = strlen(cur);
        const char *remainder = out[0] + input_length;
        recordInsert(state->manager, state->list, remainder, strlen(remainder));
      }
      g_free(cur);
    }
    handled = TRUE;
  } else if (keyval == GDK_KEY_Delete) {
    recordDeleteRight(state->manager, state->list);
    handled = TRUE;
  }

  // kiểm tra đánh thường
  if (handled) {
    sync_gtk_with_list(state);

    update_autocomplete(state);
    return TRUE; // trả false để hiển thị
  }
  return FALSE; // cho hiển thị các kí tự ko quan tâm
}

static void on_insert_text(GtkTextBuffer *buffer, GtkTextIter *location,
                           gchar *text, gint len, gpointer user_data) {
  EditorState *state = (EditorState *)user_data;
  if (state->is_updating_gtk)
    return;

  if (len < 0) {
    len = strlen(text);
  }

  recordInsert(state->manager, state->list, text, len);

  // Gọi autocomplete ngay sau khi Unikey nhét chữ Tiếng Việt vào
  // Nhưng phải hoãn lại (deferred) để không làm hỏng Iterator của GTK
  g_idle_add(deferred_sync_and_autocomplete, state);
}

void setup_ui(EditorState *state, GtkApplication *app) {

  GtkWidget *window = gtk_application_window_new(app); // gọi biến cửa sổ

  gtk_window_set_title(GTK_WINDOW(window), "Pad"); // đặt tên tiêu đề ngu!
  gtk_window_set_default_size(GTK_WINDOW(window), 800,
                              600); // kích thước window

  GtkWidget *scrolled_window = gtk_scrolled_window_new(); // tạo cửa sổ lướt đc
  gtk_window_set_child(
      GTK_WINDOW(window),
      scrolled_window); // cho cửa sổ mới tạo làm con, đặt vào cửa sổ mẹ

  GtkWidget *text_view = gtk_text_view_new(); // để điền kí tự

  // liên kết ô điền với struct
  state->buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)); // tương tự với lướt

  g_signal_connect(state->buffer, "insert-text", G_CALLBACK(on_insert_text),
                   state);

  gtk_text_buffer_create_tag(state->buffer, "ghost_text", "foreground",
                             "#888888", NULL);

  // tạo bộ dò phím
  GtkEventController *key_ctrl = gtk_event_controller_key_new();

  gtk_event_controller_set_propagation_phase(key_ctrl, GTK_PHASE_CAPTURE);

  g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed),
                   state); // gửi thông tin tới hàm crtl phím

  gtk_widget_add_controller(text_view, key_ctrl); // thêm kiểm tra vào ô điền

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window),
                                text_view);
  gtk_window_present(GTK_WINDOW(window));
}