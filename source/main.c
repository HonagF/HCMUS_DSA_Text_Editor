#include "autocomplete.h"
#include "list.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int last_was_tab = 0;
int current_suggestion_index = 0;
int inserted_len = 0;
int suggestion_count = 0;
char tab_results[10][256];
int original_prefix_len = 0;

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

  printf("\n Gợi ý (Bấm TAB để chọn từ): \n");
  for (int i = 0; i < count; i++) {
    printf("%d. %s\n", i + 1, results[i]);
  }
  printf("\n");
}

int main() {
  system("chcp 65001 > nul");
  EditorList list;
  initList(&list);
  TrieNode *dictRoot = create_node('\0');

  load_txt(dictRoot, "../data/google-10000-english-no-swears.txt");
  load_txt(dictRoot, "../data/Viet11K.txt");

  while (1) {
    system("cls");
    printListWithCursor(&list);
    printSuggestions(&list, dictRoot);

    char ch = _getch();

    if (ch != 9) {
      last_was_tab = 0;
    }
    if (ch == 27) {
      break;
    } else if (ch == 9) {
      if (last_was_tab == 0) {
        char prefix[256] = "";
        int i = 0;
        Node *p = list.cursor;
        while (p != NULL && p->data != '\n' && p->data != ' ') {
          p = p->prev;
        }
        Node *startNode = (p == NULL) ? list.head : p->next;
        while (startNode != NULL && startNode != list.cursor->next) {
          prefix[i++] = startNode->data;
          startNode = startNode->next;
        }
        prefix[i] = '\0';
        original_prefix_len = i;

        suggestion_count = suggest(dictRoot, prefix, tab_results);

        if (suggestion_count > 0) {
          current_suggestion_index = 0;
          inserted_len = strlen(tab_results[0]) - original_prefix_len;
          for (int k = original_prefix_len; k < strlen(tab_results[0]); k++) {
            insertChar(&list, tab_results[0][k]);
          }
          last_was_tab = 1;
        }
      } else if (last_was_tab == 1) {
        if (suggestion_count > 0) {
          for (int k = 0; k < inserted_len; k++) {
            deleteChar(&list);
          }
          current_suggestion_index =
              (current_suggestion_index + 1) % suggestion_count;
          inserted_len = strlen(tab_results[current_suggestion_index]) -
                         original_prefix_len;
          for (int k = original_prefix_len;
               k < strlen(tab_results[current_suggestion_index]); k++) {
            insertChar(&list, tab_results[current_suggestion_index][k]);
          }
        }
      }
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
