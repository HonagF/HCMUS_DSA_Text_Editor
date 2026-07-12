#include "list.h"
#include <conio.h> // Thu vien chua _getch()
#include <stdio.h>
#include <stdlib.h>

void printListWithCursor(EditorList *list) {
  if (list->cursor == NULL)
    printf("|");
  for (Node *temp = list->head; temp != NULL; temp = temp->next) {
    printf("%c", temp->data);
    if (list->cursor == temp)
      printf("|");
  }
  printf("\n====================\n");
}

int main() {
  EditorList list;
  initList(&list);

  while (1) {
    system("cls"); // Xóa màn hình cũ (Windows)
    printListWithCursor(&list);

    char ch = _getch(); // Bắt phím (Không cần nhấn Enter)

    if (ch == 27) { // Phím ESC để thoát
      break;
    } else if (ch == 8) { // Phím Backspace
      deleteChar(&list);
    } else if (ch == 13) {
      insertChar(&list, '\n');
    } else if (ch == -32 || ch == 224) {
      // PHÍM ĐẶC BIỆT (Mũi tên): Nó trả về 2 byte. Byte đầu là -32 hoặc 224.
      // Phải gọi _getch() thêm 1 lần nữa để lấy mã thật.
      char arrow = _getch();
      // TODO: Bắt mũi tên Lên (72), Xuống (80), Trái (75), Phải (77)
      switch (arrow) {
      case 72:
        moveCursorUp(&list);
        break;
      case 80:
        moveCursorDown(&list);
        break;
      case 75:
        moveCursorLeft(&list);
        break;
      case 77:
        moveCursorRight(&list);
        break;
      case 115:
        moveCursorPrevWord(&list);
        break;
      case 116:
        moveCursorNextWord(&list);
        break;
      case 83:
        deleteRight(&list);
        break;
      }
    } else {
      insertChar(&list, ch);
    }
  }

  destroyList(&list);
  return 0;
}
