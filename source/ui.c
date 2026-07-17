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
    full_text[i] = current->data; // Lấy kÛ tự từ node
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
      g_malloc(len + 1); // Cấp phát thêm 1 byte cho kÛ tự kết thúc chuỗi '\0'
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
  }
  if (cur != NULL) {
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

  if ((modifiers & GDK_CONTROL_MASK) && keyval == GDK_KEY_f) {
    state->search_visible = !state->search_visible;
    gtk_widget_set_visible(state->search_panel, state->search_visible);
    if (state->search_visible)
        gtk_widget_grab_focus(state->find_entry);
    return TRUE;
  }

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
          recordInsert(state->manager, state->list, (char *)remainder,
                       strlen(remainder));
          sync_gtk_with_list(state);
        }
      }
      if (cur != NULL) {
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




int total_matches = 0;
int current_match = 0;
gboolean match_case = FALSE;
GtkTextTag *highlight_tag = NULL;
GtkTextTag *current_tag = NULL;

static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".header { background: #2d2d2d; border-bottom: 1px solid #1a1a1a; }"
        ".header button { padding: 4px 12px; margin: 4px; border-radius: 4px; background: transparent; color: #cccccc; border: none; font-weight: 500; }"
        ".header button:hover { background: #3d3d3d; color: white; }"
        ".header button.active { background: #0078d4; color: white; }"
        ".toolbar { background: #252526; padding: 6px; border-bottom: 1px solid #1a1a1a; }"
        ".toolbar button { padding: 6px; margin: 0 2px; border-radius: 4px; background: transparent; color: #cccccc; border: none; }"
        ".toolbar button:hover { background: #3d3d3d; color: white; }"
        ".sidebar { background: #252526; border-right: 1px solid #1a1a1a; }"
        ".sidebar button { margin: 2px 8px; padding: 6px 12px; border-radius: 4px; text-align: left; background: transparent; color: #cccccc; border: none; }"
        ".sidebar button:hover { background: #3d3d3d; color: white; }"
        ".sidebar button.active { background: #37373d; color: white; }"
        ".statusbar { background: #007acc; color: white; padding: 2px 10px; font-size: 12px; }"
        "textview { background: #1e1e1e; color: #d4d4d4; font-family: monospace; font-size: 14px; }"
        ".glass-panel { background: rgba(30,30,30,0.95); border-radius: 8px; border: 1px solid #404040; box-shadow: 0 4px 12px rgba(0,0,0,0.5); padding: 12px; }"
        ".search-panel entry { background: #3c3c3c; color: white; border: 1px solid #555; padding: 4px 8px; border-radius: 4px; }"
        ".search-panel button { padding: 4px 12px; border-radius: 4px; }"
        ".search-panel button.suggested { background: #0078d4; color: white; border: none; }"
        ".search-panel button.suggested:hover { background: #006abc; }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void sanitize(const char *src, char *dst) {
  int j = 0;
  for (int i = 0; src[i] != '\0'; i++) {
    if ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z')) {
      dst[j++] = src[i];
    }
  }
  dst[j] = '\0';
}


// ========================= CALLBACKS TOOLBAR =========================
static void on_undo_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    if (canUndo(state->manager)) {
        undo(state->manager, state->list);
        sync_gtk_with_list(state);
        update_autocomplete(state);
    }
}

static void on_redo_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    if (canRedo(state->manager)) {
        redo(state->manager, state->list);
        sync_gtk_with_list(state);
        update_autocomplete(state);
    }
}

static void on_clear_clicked(GtkButton *button, gpointer user_data) {
    EditorState *state = (EditorState *)user_data;
    // Xoá toàn bộ văn bản trên giao diện
    gtk_text_buffer_set_text(state->buffer, "", -1);
    // TODO: đồng bộ lại list nếu cần (có thể thêm hàm clearList)
}

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

// ========================= SEARCH FUNCTIONS  =========================
static void highlight_all(GtkTextBuffer *buffer, const char *text) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_remove_tag(buffer, highlight_tag, &start, &end);
    total_matches = 0;
    current_match = 0;

    if (!text || !*text) return;

    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_VISIBLE_ONLY;
    if (!match_case) flags |= GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    GtkTextIter iter = start;
    while (TRUE) {
        GtkTextIter mstart, mend;
        if (gtk_text_iter_forward_search(&iter, text, flags, &mstart, &mend, NULL)) {
            gtk_text_buffer_apply_tag(buffer, highlight_tag, &mstart, &mend);
            total_matches++;
            iter = mend;
        } else break;
    }
    if (total_matches > 0) current_match = 1;
}

static void goto_match(EditorState *state, const char *text) {
    if (!text || total_matches == 0) return;
    GtkTextBuffer *buffer = state->buffer;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);

    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_VISIBLE_ONLY;
    if (!match_case) flags |= GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    int count = 0;
    GtkTextIter iter = start;
    while (TRUE) {
        GtkTextIter mstart, mend;
        if (gtk_text_iter_forward_search(&iter, text, flags, &mstart, &mend, NULL)) {
            count++;
            if (count == current_match) {
                gtk_text_buffer_remove_tag(buffer, current_tag, &start, &end);
                gtk_text_buffer_apply_tag(buffer, current_tag, &mstart, &mend);
                gtk_text_buffer_select_range(buffer, &mstart, &mend);
                GtkTextView *view = GTK_TEXT_VIEW(state->text_view);
                if (view) gtk_text_view_scroll_to_iter(view, &mstart, 0.0, FALSE, 0.5, 0.5);
                break;
            }
            iter = mend;
        } else break;
    }
}

static void update_status(EditorState *state) {
    if (!state->status_label) return;
    char buf[64];
    if (total_matches == 0)
        snprintf(buf, sizeof(buf), "No results");
    else
        snprintf(buf, sizeof(buf), "%d of %d matches", current_match, total_matches);
    gtk_label_set_text(GTK_LABEL(state->status_label), buf);
}

static void on_find_changed(GtkEditable *editable, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    const char *text = gtk_editable_get_text(editable);
    highlight_all(state->buffer, text);
    goto_match(state, text);
    update_status(state);
}

static void on_find_next(GtkButton *btn, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    const char *text = gtk_editable_get_text(GTK_EDITABLE(state->find_entry));
    if (total_matches > 0) {
        current_match = (current_match % total_matches) + 1;
        goto_match(state, text);
        update_status(state);
    }
}

static void on_replace(GtkButton *btn, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    const char *search = gtk_editable_get_text(GTK_EDITABLE(state->find_entry));
    const char *replace = gtk_editable_get_text(GTK_EDITABLE(state->replace_entry));
    if (!search || !*search) return;

    GtkTextIter sel_start, sel_end;
    if (gtk_text_buffer_get_selection_bounds(state->buffer, &sel_start, &sel_end)) {
        gchar *sel_text = gtk_text_buffer_get_text(state->buffer, &sel_start, &sel_end, FALSE);
        if ((match_case && g_strcmp0(sel_text, search) == 0) ||
            (!match_case && g_ascii_strcasecmp(sel_text, search) == 0)) {
            gtk_text_buffer_delete(state->buffer, &sel_start, &sel_end);
            gtk_text_buffer_insert(state->buffer, &sel_start, replace, -1);
        }
        g_free(sel_text);
    }
    highlight_all(state->buffer, search);
    if (total_matches > 0) {
        current_match = 1;
        goto_match(state, search);
    }
    update_status(state);
}

static void on_replace_all(GtkButton *btn, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    const char *search = gtk_editable_get_text(GTK_EDITABLE(state->find_entry));
    const char *replace = gtk_editable_get_text(GTK_EDITABLE(state->replace_entry));
    if (!search || !*search) return;

    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(state->buffer, &start, &end);
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_VISIBLE_ONLY;
    if (!match_case) flags |= GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    int count = 0;
    GtkTextIter iter = start;
    while (TRUE) {
        GtkTextIter mstart, mend;
        if (gtk_text_iter_forward_search(&iter, search, flags, &mstart, &mend, NULL)) {
            gtk_text_buffer_delete(state->buffer, &mstart, &mend);
            gtk_text_buffer_insert(state->buffer, &mstart, replace, -1);
            iter = mstart;
            gtk_text_iter_forward_chars(&iter, strlen(replace));
            count++;
        } else break;
    }
    highlight_all(state->buffer, search);
    update_status(state);
    g_print("Replaced %d occurrences\n", count);
}

static void on_match_case_toggled(GtkWidget *btn, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    match_case = !match_case;
    if (match_case)
        gtk_widget_add_css_class(btn, "active");
    else
        gtk_widget_remove_css_class(btn, "active");
    on_find_changed(GTK_EDITABLE(state->find_entry), state);
}

static void on_close_panel(GtkButton *btn, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    gtk_widget_set_visible(state->search_panel, FALSE);
    state->search_visible = FALSE;
}

static GtkWidget* create_search_panel(EditorState *state) {
    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(panel, "glass-panel");
    gtk_widget_set_halign(panel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(panel, GTK_ALIGN_START);
    gtk_widget_set_margin_top(panel, 32);
    gtk_widget_set_size_request(panel, 420, -1);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *title = gtk_label_new("Search and Replace");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_widget_set_hexpand(title, TRUE);
    GtkWidget *close = gtk_button_new_from_icon_name("window-close-symbolic");
    g_signal_connect(close, "clicked", G_CALLBACK(on_close_panel), state);
    gtk_box_append(GTK_BOX(header), title);
    gtk_box_append(GTK_BOX(header), close);
    gtk_box_append(GTK_BOX(panel), header);

    GtkWidget *find_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *find_icon = gtk_image_new_from_icon_name("system-search-symbolic");
    GtkWidget *find_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(find_entry), "Find");
    state->find_entry = find_entry;
    g_signal_connect(find_entry, "changed", G_CALLBACK(on_find_changed), state);

    state->chk_match_case = gtk_button_new_from_icon_name("format-text-bold-symbolic");
    gtk_widget_add_css_class(state->chk_match_case, "toggle-btn");
    gtk_widget_set_tooltip_text(state->chk_match_case, "Match Case");
    g_signal_connect(state->chk_match_case, "clicked", G_CALLBACK(on_match_case_toggled), state);

    GtkWidget *regex_btn = gtk_button_new_from_icon_name("media-playlist-shuffle-symbolic");
    gtk_widget_add_css_class(regex_btn, "toggle-btn");
    gtk_widget_set_tooltip_text(regex_btn, "Use Regex");

    gtk_box_append(GTK_BOX(find_row), find_icon);
    gtk_box_append(GTK_BOX(find_row), find_entry);
    gtk_box_append(GTK_BOX(find_row), state->chk_match_case);
    gtk_box_append(GTK_BOX(find_row), regex_btn);
    gtk_box_append(GTK_BOX(panel), find_row);

    GtkWidget *replace_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *replace_icon = gtk_image_new_from_icon_name("document-edit-symbolic");
    GtkWidget *replace_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(replace_entry), "Replace with");
    state->replace_entry = replace_entry;
    gtk_box_append(GTK_BOX(replace_row), replace_icon);
    gtk_box_append(GTK_BOX(replace_row), replace_entry);
    gtk_box_append(GTK_BOX(panel), replace_row);

    GtkWidget *action_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    state->status_label = gtk_label_new("No results");
    gtk_widget_set_halign(state->status_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(state->status_label, TRUE);

    GtkWidget *replace_all_btn = gtk_button_new_with_label("Replace All");
    GtkWidget *replace_btn = gtk_button_new_with_label("Replace");
    GtkWidget *find_next_btn = gtk_button_new_with_label("Find Next");
    gtk_widget_add_css_class(find_next_btn, "suggested");

    g_signal_connect(replace_all_btn, "clicked", G_CALLBACK(on_replace_all), state);
    g_signal_connect(replace_btn, "clicked", G_CALLBACK(on_replace), state);
    g_signal_connect(find_next_btn, "clicked", G_CALLBACK(on_find_next), state);

    gtk_box_append(GTK_BOX(action_row), state->status_label);
    gtk_box_append(GTK_BOX(action_row), replace_all_btn);
    gtk_box_append(GTK_BOX(action_row), replace_btn);
    gtk_box_append(GTK_BOX(action_row), find_next_btn);
    gtk_box_append(GTK_BOX(panel), action_row);

    return panel;
}

// ========================= SETUP_UI TÍCH HỢP =========================

static void on_search_menu_clicked(GtkButton *btn, gpointer user_data) {
    EditorState *state = (EditorState*)user_data;
    state->search_visible = !state->search_visible;
    gtk_widget_set_visible(state->search_panel, state->search_visible);
    if (state->search_visible)
        gtk_widget_grab_focus(state->find_entry);
}

void setup_ui(EditorState *state, GtkApplication *app) {
    apply_css();

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Pad");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    // Main layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // ===== HEADER =====
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(header, "header");
    gtk_widget_set_size_request(header, -1, 48);

    GtkWidget *logo = gtk_label_new("Pad");
    gtk_widget_set_margin_start(logo, 12);
    gtk_box_append(GTK_BOX(header), logo);

    const char *menu[] = {"File", "Edit", "Search", "View", NULL};
    for (int i = 0; menu[i]; i++) {
        GtkWidget *btn = gtk_button_new_with_label(menu[i]);
        if (i == 2) {
            gtk_widget_add_css_class(btn, "active");
            g_signal_connect(btn, "clicked", G_CALLBACK(on_search_menu_clicked), state);
        }
        gtk_box_append(GTK_BOX(header), btn);
    }

    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(header), spacer);

    GtkWidget *undo_btn = gtk_button_new_from_icon_name("edit-undo-symbolic");
    gtk_widget_set_tooltip_text(undo_btn, "Undo (Ctrl+Z)");
    g_signal_connect(undo_btn, "clicked", G_CALLBACK(on_undo_clicked), state);
    gtk_box_append(GTK_BOX(header), undo_btn);

    GtkWidget *redo_btn = gtk_button_new_from_icon_name("edit-redo-symbolic");
    gtk_widget_set_tooltip_text(redo_btn, "Redo (Ctrl+Y)");
    g_signal_connect(redo_btn, "clicked", G_CALLBACK(on_redo_clicked), state);
    gtk_box_append(GTK_BOX(header), redo_btn);

    GtkWidget *save_btn = gtk_button_new_from_icon_name("document-save-symbolic");
    gtk_box_append(GTK_BOX(header), save_btn);

    GtkWidget *settings_btn = gtk_button_new_from_icon_name("emblem-system-symbolic");
    gtk_box_append(GTK_BOX(header), settings_btn);
    gtk_box_append(GTK_BOX(vbox), header);

    // ===== TOOLBAR (Cut/Copy/Paste/Clear) =====
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_add_css_class(toolbar, "toolbar");

    GtkWidget *cut_btn = gtk_button_new_from_icon_name("edit-cut-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(cut_btn), FALSE);
    g_signal_connect(cut_btn, "clicked", G_CALLBACK(on_cut_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), cut_btn);

    GtkWidget *copy_btn = gtk_button_new_from_icon_name("edit-copy-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(copy_btn), FALSE);
    g_signal_connect(copy_btn, "clicked", G_CALLBACK(on_copy_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), copy_btn);

    GtkWidget *paste_btn = gtk_button_new_from_icon_name("edit-paste-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(paste_btn), FALSE);
    g_signal_connect(paste_btn, "clicked", G_CALLBACK(on_paste_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), paste_btn);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_append(GTK_BOX(toolbar), sep);

    GtkWidget *clear_btn = gtk_button_new_from_icon_name("edit-delete-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(clear_btn), FALSE);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_clicked), state);
    gtk_box_append(GTK_BOX(toolbar), clear_btn);
    gtk_box_append(GTK_BOX(vbox), toolbar);

    // ===== BODY (Sidebar + Editor) =====
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_vexpand(hbox, TRUE);
    gtk_box_append(GTK_BOX(vbox), hbox);

    // Sidebar
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(sidebar, "sidebar");
    gtk_widget_set_size_request(sidebar, 220, -1);
    gtk_widget_set_margin_top(sidebar, 8);
    gtk_widget_set_margin_bottom(sidebar, 8);

    const char *side_items[] = {"Main", NULL};
    for (int i = 0; side_items[i]; i++) {
        GtkWidget *btn = gtk_button_new_with_label(side_items[i]);
        if (i == 1) gtk_widget_add_css_class(btn, "active");
        gtk_widget_set_halign(btn, GTK_ALIGN_FILL);
        gtk_box_append(GTK_BOX(sidebar), btn);
    }
    GtkWidget *spacer2 = gtk_label_new("");
    gtk_widget_set_vexpand(spacer2, TRUE);
    gtk_box_append(GTK_BOX(sidebar), spacer2);
    gtk_box_append(GTK_BOX(hbox), sidebar);

    // Overlay (editor + search panel)
    GtkWidget *overlay = gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);
    gtk_box_append(GTK_BOX(hbox), overlay);

    // ScrolledWindow + TextView
    GtkWidget *scrolled = gtk_scrolled_window_new();
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 15);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 15);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 12);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), TRUE);

    state->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    state->text_view = text_view;

    // DEV BRANCH LOGIC: Tag for autocomplete
    gtk_text_buffer_create_tag(state->buffer, "ghost_text", "foreground", "#888888", NULL);

    // DEV BRANCH LOGIC: Tag for search
    highlight_tag = gtk_text_buffer_create_tag(state->buffer, "highlight", "background", "rgba(0,120,212,0.4)", NULL);
    current_tag = gtk_text_buffer_create_tag(state->buffer, "current-highlight", "background", "#f0ad4e", NULL);

    // DEV BRANCH LOGIC: Signals
    g_signal_connect(state->buffer, "insert-text", G_CALLBACK(on_insert_text), state);

    GtkEventController *key_ctrl = gtk_event_controller_key_new();
    gtk_event_controller_set_propagation_phase(key_ctrl, GTK_PHASE_CAPTURE);
    g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed), state);
    gtk_widget_add_controller(text_view, key_ctrl);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text_view);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), scrolled);

    // Search panel
    state->search_panel = create_search_panel(state);
    gtk_widget_set_visible(state->search_panel, FALSE);
    state->search_visible = FALSE;
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), state->search_panel);

    // ===== STATUSBAR =====
    GtkWidget *status = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(status, "statusbar");
    GtkWidget *ln = gtk_label_new("Ln 1, Col 1");
    GtkWidget *enc = gtk_label_new("UTF-8");
    GtkWidget *lang = gtk_label_new("C");
    GtkWidget *words = gtk_label_new("0 WORDS");
    gtk_widget_set_hexpand(words, TRUE);
    gtk_widget_set_halign(words, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(status), ln);
    gtk_box_append(GTK_BOX(status), enc);
    gtk_box_append(GTK_BOX(status), lang);
    gtk_box_append(GTK_BOX(status), words);
    gtk_box_append(GTK_BOX(vbox), status);

    gtk_window_present(GTK_WINDOW(window));
}
