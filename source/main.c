#include "autocomplete.h"
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

void printSuggestions(EditorList *list, TrieNode *dictRoot) {
  char prefix[256] = "";
  int i = 0;

  Node *p = list->cursor;
  while (p != NULL && p->data != '\n' && p->data != ' ') {
    p = p->prev;
  }
  Node *startNode = (p == NULL) ? list->head : p->next;
  while (startNode != NULL && startNode != list->cursor->next) {
    prefix[i] = startNode->data;
    startNode = startNode->next;
    i++;
  }
  prefix[i] = '\0';
  char results[10][256];
  int count = suggest(dictRoot, prefix, results);

  printf("\n Goi y (Bam TAB de chon tu 1): \n");
  for (int i = 0; i < count; i++) {
    printf("%d. %s\n", i + 1, results[i]);
  }
  printf("\n");
}

int main() {
  EditorList list;
  initList(&list);
  TrieNode *dictRoot = create_node();

  load_txt(dictRoot, "../data/google-10000-english-no-swears.txt");

  while (1) {
    system("cls");
    printListWithCursor(&list);
    printSuggestions(&list, dictRoot);

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
