typedef struct Node {
  char data;
  struct Node *next;
  struct Node *prev;
} Node;
typedef struct EditorList {
  struct Node *head;
  struct Node *tail;
  struct Node *cursor;
  int size;
  int index_cursor;
} EditorList;
void initList(EditorList *list);
Node *createNode(char c);
void insertChar(EditorList *list, char c);
void deleteChar(EditorList *list);
void moveCursorLeft(EditorList *list);
void moveCursorRight(EditorList *list);
void deleteRight(EditorList *list);
void moveCursorPrevWord(EditorList *list);
void moveCursorNextWord(EditorList *list);
void destroyList(EditorList *list);