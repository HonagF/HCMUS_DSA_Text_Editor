#include "list.h"
#include <stdlib.h>

void initList(EditorList *list) {
  list->head = NULL;
  list->tail = NULL;
  list->cursor = NULL;
  list->size = 0;
  list->index_cursor = 0;
}

Node *createNode(char c) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (newNode == NULL) {
    return NULL;
  }
  newNode->data = c;
  newNode->next = NULL;
  newNode->prev = NULL;
  return newNode;
}

void insertChar(EditorList *list, char c) {
  Node *newNode = createNode(c);
  if (newNode == NULL)
    return;
  if (list->size == 0) {
    list->head = newNode;
    list->tail = newNode;
  } else if (list->cursor == NULL && list->size > 0) {
    newNode->next = list->head;
    list->head->prev = newNode;
    list->head = newNode;

  } else {
    newNode->next = list->cursor->next;
    newNode->prev = list->cursor;
    list->cursor->next = newNode;
    if (newNode->next != NULL) {
      newNode->next->prev = newNode;
    } else {
      list->tail = newNode;
    }
  }
  list->cursor = newNode;
  list->size++;
  list->index_cursor++;
}

void deleteChar(EditorList *list) {
  if (list->size == 0 || list->cursor == NULL)
    return;
  do {
    int is_continuation = ((list->cursor->data & 0xc0) == 0x80);

    Node *delNode = list->cursor;
    Node *prevNode = delNode->prev;
    Node *nextNode = delNode->next;

    if (prevNode != NULL) {
      prevNode->next = nextNode;
    } else {
      list->head = nextNode;
    }
    if (nextNode != NULL) {
      nextNode->prev = prevNode;
    } else {
      list->tail = prevNode;
    }
    list->cursor = prevNode;
    free(delNode);
    list->size--;
    list->index_cursor--;
    if (!is_continuation)
      break;
  } while (list->cursor != NULL);
}

void deleteRight(EditorList *list) {
  if (list->size == 0 || list->cursor == list->tail)
    return;
  do {
    Node *delNode;
    if (list->cursor == NULL)
      delNode = list->head;
    else
      delNode = list->cursor->next;
    if (delNode == NULL)
      return;

    Node *prevNode = delNode->prev;
    Node *nextNode = delNode->next;

    if (prevNode != NULL) {
      prevNode->next = nextNode;
    } else {
      list->head = nextNode;
    }
    if (nextNode != NULL) {
      nextNode->prev = prevNode;
    } else {
      list->tail = prevNode;
    }
    free(delNode);
    list->size--;

    if (nextNode == NULL || (nextNode->data & 0xC0) != 0x80)
      break;
  } while (1);
}

void moveCursorLeft(EditorList *list) {
  if (list->cursor == NULL)
    return;
  do {
    list->cursor = list->cursor->prev;
    list->index_cursor--;
  } while (list->cursor != NULL && (list->cursor->next->data & 0xc0) == 0x80);
}

void moveCursorRight(EditorList *list) {
  if (list->cursor == list->tail)
    return;
  do {
    if (list->cursor == NULL)
      list->cursor = list->head;
    else
      list->cursor = list->cursor->next;
    list->index_cursor++;
  } while (list->cursor != list->tail && list->cursor->next != NULL &&
           (list->cursor->next->data & 0xC0) == 0x80);
}

void destroyList(EditorList *list) {
  Node *current = list->head;
  while (current != NULL) {
    Node *temp = current;
    current = current->next;
    free(temp);
  }
  list->head = NULL;
  list->tail = NULL;
  list->cursor = NULL;
  list->size = 0;
  list->index_cursor = 0;
}

void moveCursorPrevWord(EditorList *list) {
  while (list->cursor != NULL && list->cursor->data == ' ') {
    moveCursorLeft(list);
  }
  while (list->cursor != NULL && list->cursor->data != ' ') {
    moveCursorLeft(list);
  }
}

void moveCursorNextWord(EditorList *list) {
  while (list->cursor != list->tail) {
    Node *rightNode = (list->cursor == NULL) ? list->head : list->cursor->next;
    if (rightNode->data != ' ')
      break;
    moveCursorRight(list);
  }
  while (list->cursor != list->tail) {
    Node *rightNode = (list->cursor == NULL) ? list->head : list->cursor->next;
    if (rightNode->data == ' ')
      break;
    moveCursorRight(list);
  }
}

int getColumn(EditorList *list) {
  int col = 0;
  Node *temp = list->cursor;

  while (temp != NULL && temp->data != '\n') {
    col++;
    temp = temp->prev;
  }
  return col;
}

void moveCursorUp(EditorList *list) {
  int targetCol = getColumn(list);
  while (list->cursor != NULL && list->cursor->data != '\n') {
    moveCursorLeft(list);
  }
  if (list->cursor == NULL) {
    return;
  }
  if (list->cursor->data == '\n') {
    moveCursorLeft(list);
  }
  while (list->cursor != NULL && list->cursor->data != '\n') {
    moveCursorLeft(list);
  }
  for (int i = 0; i < targetCol; i++) {
    Node *rightNode = (list->cursor == NULL) ? list->head : list->cursor->next;
    if (rightNode == NULL || rightNode->data == '\n')
      break;
    moveCursorRight(list);
  }
}

void moveCursorDown(EditorList *list) {
  int targetCol = getColumn(list);
  while (1) {
    Node *rightNode = (list->cursor == NULL) ? list->head : list->cursor->next;
    if (rightNode == NULL)
      return;
    if (rightNode->data == '\n') {
      moveCursorRight(list);
      break;
    }
    moveCursorRight(list);
  }
  for (int i = 0; i < targetCol; i++) {
    Node *rightNode = (list->cursor == NULL) ? list->head : list->cursor->next;
    if (rightNode == NULL || rightNode->data == '\n')
      break;
    moveCursorRight(list);
  }
}

void deleteWord(EditorList *list) {
  while (list->cursor != NULL && list->cursor->data == ' ') {
    deleteChar(list);
  }
  while (list->cursor != NULL && list->cursor->data != ' ') {
    deleteChar(list);
  }
}

void moveCursorToIndex(EditorList *list, int target) {
  int prev_index;
  while (list->index_cursor < target) {
    prev_index = list->index_cursor;
    moveCursorRight(list);
    if (list->index_cursor == prev_index)
      break;
  }
  while (list->index_cursor > target) {
    prev_index = list->index_cursor;
    moveCursorLeft(list);
    if (list->index_cursor == prev_index)
      break;
  }
}