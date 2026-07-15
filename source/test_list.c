#include "list.h"
#include <stdio.h>

void printList(EditorList *list) {
  Node *temp = list->head;
  while (temp != NULL) {
    printf("%c", temp->data);
    temp = temp->next;
  }
  printf("\n");
}

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

  insertChar(&list, 'A');
  insertChar(&list, 'B');
  insertChar(&list, 'C');
  insertChar(&list, '\n');
  insertChar(&list, 'D');
  insertChar(&list, 'E');
  insertChar(&list, 'F');

  printf("Van ban hien tai: ");
  printListWithCursor(&list);

  printf("\n2. Lui trai hai lan: \n");
  moveCursorLeft(&list);
  moveCursorLeft(&list);
  printListWithCursor(&list);

  printf("\n3. Up: \n");
  moveCursorUp(&list);
  printListWithCursor(&list);

  printf("\n4. Down: \n");
  moveCursorDown(&list);
  printListWithCursor(&list);

  destroyList(&list);
  return 0;
}