#ifndef API_H
#define API_H

#include "list.h"
#include "undo_redo.h"
#include <gtk/gtk.h>

// Forward declarations of your custom DSA structs
typedef struct TrieNode TrieNode;

// The Master Struct
typedef struct {
  GtkTextBuffer *buffer; // GTK handles the text display
  UndoRedoManager *manager;
  TrieNode *dictionary_root; // You handle the autocomplete
  EditorList *list;
  int is_updating_gtk;

  // --- THÊM CÁC BIẾN CHO TÍNH NĂNG TÌM KIẾM (SEARCH & REPLACE) ---
  GtkWidget *text_view;         // Khung soạn thảo văn bản
  GtkWidget *search_panel;      // Bảng (Khung) tìm kiếm
  GtkWidget *find_entry;        // Ô nhập từ khóa tìm kiếm
  GtkWidget *replace_entry;     // Ô nhập từ khóa thay thế
  GtkWidget *chk_match_case;    // Nút phân biệt chữ Hoa/thường (Match Case)
  GtkWidget *status_label;      // Dòng thông báo kết quả (VD: "No results")
  gboolean search_visible;      // Trạng thái ẩn/hiện khung tìm kiếm (TRUE/FALSE)
} EditorState;

#endif