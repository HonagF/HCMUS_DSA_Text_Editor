#include "list.h"
#include <conio.h>
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
    system("cls");
    printListWithCursor(&list);

    char ch = _getch();

    if (ch == 27) {
      break;
    } else if (ch == 8) {
      deleteChar(&list);
    } else if (ch == 13) {
      insertChar(&list, '\n');
    } else if (ch == 23) {
      deleteWord(&list);
    } else if (ch == -32 || ch == 224) {
      char arrow = _getch();
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
