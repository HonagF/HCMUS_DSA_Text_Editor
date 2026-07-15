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
}

void deleteRight(EditorList *list) {
  if (list->size == 0 || list->cursor == list->tail)
    return;
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
}

void moveCursorLeft(EditorList *list) {
  if (list->cursor == NULL)
    return;
  list->cursor = list->cursor->prev;
  list->index_cursor--;
}

void moveCursorRight(EditorList *list) {
  if (list->cursor == list->tail)
    return;
  if (list->cursor == NULL) {
    list->cursor = list->head;
    list->index_cursor++;
    return;
  }
  list->cursor = list->cursor->next;
  list->index_cursor++;
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