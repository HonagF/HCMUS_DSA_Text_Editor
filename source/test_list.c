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

int main() {
  EditorList list;
  initList(&list);

  printf("1. Go vao chu 'H', 'C', 'M', 'U', 'S':\n");
  insertChar(&list, 'H');
  insertChar(&list, 'C');
  insertChar(&list, 'M');
  insertChar(&list, 'U');
  insertChar(&list, 'S');

  printf("Van ban hien tai: ");
  printList(&list);

  printf("\n2. Nhan backspace 2 lan (Xoa chu U, S): \n");
  deleteChar(&list);
  deleteChar(&list);

  printf("Van ban sau khi xoa: ");
  printList(&list);

  printf("\n3. Go them chu 'K', 'H', 'T', 'N'):\n");
  insertChar(&list, 'K');
  insertChar(&list, 'H');
  insertChar(&list, 'T');
  insertChar(&list, 'N');

  printf("Van ban sau khi them: ");
  printList(&list);
  return 0;
}