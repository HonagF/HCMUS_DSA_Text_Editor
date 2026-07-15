# Module Undo/Redo — Nhánh `undo-redo`

Phần đảm nhiệm: chức năng Undo/Redo không giới hạn cho công cụ soạn thảo văn bản mini (đồ án DSA), dùng **Command Pattern** trên nền **2 stack (linked list)**.

File liên quan: `undoredo.h`, `undoredo.c`, `test_undoredo.c`. Phụ thuộc `list.h`/`list.c` (nhánh `linked-list`).

---

## 1. Ý tưởng thiết kế

Mỗi hành động sửa văn bản (chèn, xoá trái/backspace, xoá phải/delete) được đóng gói thành một **Command**, chứa đủ thông tin để **làm lại** hoặc **đảo ngược** hành động đó mà không cần giữ con trỏ (`Node*`) trực tiếp — vì node có thể đã bị `free()` bởi một lần undo trước đó.

```c
typedef struct Command {
  ActionType type;     // ACTION_INSERT | ACTION_DELETE_LEFT | ACTION_DELETE_RIGHT
  char data;            // ký tự liên quan
  int position;          // index_cursor NGAY TRƯỚC khi hành động xảy ra
  struct Command *next;
} Command;
```

`position` là mấu chốt: trước khi undo/redo, cursor được đưa về đúng vị trí bằng hàm nội bộ `moveCursorToIndex()` (di chuyển `moveCursorLeft`/`moveCursorRight` lặp lại tới khi `index_cursor` khớp `target`), sau đó mới gọi lại hàm gốc trong `list.c` để thực thi/đảo ngược.

Hai stack (dạng linked list, LIFO):

```c
typedef struct CommandStack {
  Command *top;
  int size;
} CommandStack;

typedef struct UndoRedoManager {
  CommandStack undoStack;
  CommandStack redoStack;
} UndoRedoManager;
```

Command node được **tái sử dụng**: khi `undo()` pop từ `undoStack`, node đó được đẩy thẳng sang `redoStack` (và ngược lại) thay vì free/malloc lại.

---

## 2. API

### Quản lý stack (nếu cần test riêng)
| Hàm | Chức năng |
|---|---|
| `initStack(CommandStack*)` | Khởi tạo stack rỗng |
| `push(CommandStack*, Command*)` | Đẩy command vào đỉnh stack |
| `pop(CommandStack*)` | Lấy và gỡ command khỏi đỉnh stack |
| `isEmptyStack(CommandStack*)` | Kiểm tra rỗng |
| `clearStack(CommandStack*)` | Free toàn bộ command trong stack |

### Manager (dùng chính)
| Hàm | Chức năng |
|---|---|
| `initManager(UndoRedoManager*)` | Khởi tạo 2 stack rỗng |
| `destroyManager(UndoRedoManager*)` | Free toàn bộ command còn lại ở cả 2 stack |
| `recordInsert(mgr, list, c)` | Chèn ký tự `c` **thay cho** gọi thẳng `insertChar`; lưu Command, xoá `redoStack` |
| `recordDeleteChar(mgr, list)` | Backspace — xoá ký tự tại cursor, lưu Command, xoá `redoStack` |
| `recordDeleteRight(mgr, list)` | Delete — xoá ký tự bên phải cursor, lưu Command, xoá `redoStack` |
| `undo(mgr, list)` | Đảo ngược hành động gần nhất; không làm gì nếu `undoStack` rỗng |
| `redo(mgr, list)` | Làm lại hành động vừa undo; không làm gì nếu `redoStack` rỗng |
| `canUndo(mgr)` / `canRedo(mgr)` | Trả về 1/0, dùng để bật/tắt nút Undo/Redo trên UI |

**Nguyên tắc bắt buộc:** mọi thao tác sửa văn bản trong `main-ui` phải gọi qua `recordInsert` / `recordDeleteChar` / `recordDeleteRight` — **không** gọi thẳng `insertChar`/`deleteChar`/`deleteRight` của `list.c`, nếu không undo/redo sẽ mất đồng bộ.

### Logic quan trọng: xoá `redoStack` khi gõ mới
Đúng yêu cầu đồ án — nếu đang ở giữa lịch sử (đã undo vài lần) mà người dùng gõ ký tự mới, toàn bộ `redoStack` bị xoá sạch (giống Word/Notepad). Cả 3 hàm `record*` đều gọi `clearStack(&mgr->redoStack)` sau khi push vào `undoStack`.

---

## 3. Cách hoạt động của Undo/Redo với từng loại hành động

| Hành động | Khi ghi lại | Khi Undo | Khi Redo |
|---|---|---|---|
| `ACTION_INSERT` | Lưu `position` = index trước khi chèn, `data` = ký tự chèn | Di chuyển cursor tới `position+1`, gọi `deleteChar` | Di chuyển cursor tới `position`, gọi `insertChar` |
| `ACTION_DELETE_LEFT` (backspace) | Lưu `position` = index trước khi xoá, `data` = ký tự bị xoá | Di chuyển cursor tới `position-1`, gọi `insertChar` | Di chuyển cursor tới `position`, gọi `deleteChar` |
| `ACTION_DELETE_RIGHT` (delete) | Lưu `position` = index cursor (không đổi khi xoá phải), `data` = ký tự bị xoá | Di chuyển cursor tới `position`, gọi `insertChar` rồi `moveCursorLeft` (để cursor không bị dịch phải) | Di chuyển cursor tới `position`, gọi `deleteRight` |

---

## 4. Test case (`test_undoredo.c`)

Đã compile + chạy, **27/27 assertion pass**. Chạy bằng:
```powershell
gcc list.c undoredo.c test_undoredo.c -o test_undoredo.exe
.\test_undoredo.exe
```

| # | Nhóm test | Kiểm tra |
|---|---|---|
| 1 | Insert + Undo/Redo cơ bản | Chèn 3 ký tự → undo 2 lần → redo 2 lần → khớp lại đúng chuỗi ban đầu; redo khi stack rỗng không đổi gì |
| 2 | Gõ mới xoá `redoStack` | Undo 2 lần (còn redo khả dụng) → gõ ký tự mới → `canRedo()` phải về 0, redo sau đó không có tác dụng |
| 3 | Backspace (`DELETE_LEFT`) | Backspace 1 ký tự → undo khôi phục đúng ký tự, đúng vị trí → redo xoá lại đúng ký tự |
| 4 | Delete phải (`DELETE_RIGHT`) | Xoá ký tự bên phải cursor → kiểm tra `index_cursor` **không đổi** sau xoá và sau undo → redo xoá lại đúng |
| 5 | Chuỗi hành động hỗn hợp | Xen kẽ insert/backspace/deleteRight → undo 4 lần liên tiếp phải trả về đúng thứ tự ngược (LIFO) → redo 4 lần phải trả lại đúng thứ tự xuôi |
| 6 | Edge case stack rỗng | `canUndo`/`canRedo` = 0 khi chưa thao tác gì; `undo()`/`redo()` trên list rỗng không crash; `recordDeleteChar`/`recordDeleteRight` trên list rỗng không tạo Command rác (không làm `undoStack` tăng) |

---

## 5. Lưu ý khi tích hợp với nhóm

- `list.h` cần có **include guard** (`#ifndef/#define/#endif`) — bản gốc từ nhánh `linked-list` hiện chưa có, sẽ gây lỗi `redefinition` khi 4 nhánh gộp lại.
- File `.c` của các nhánh (`linked-list`, `undo-redo`, `trie-complete`, `main-ui`) phải được compile/link cùng nhau (không phải build riêng lẻ từng file) — nếu dùng VS Code, cấu hình `tasks.json` liệt kê đủ tất cả file `.c` liên quan trong `args`.
- `main-ui` chỉ nên thao tác văn bản qua `recordInsert`/`recordDeleteChar`/`recordDeleteRight`, không gọi thẳng hàm trong `list.c`.