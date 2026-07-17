#include "list.h"
#include "undo_redo.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main() {
  FILE *f = fopen("input.txt", "r");
  if (!f) {
    printf("Khong tim thay input.txt\n");
    return 1;
  }

  EditorList list;
  UndoRedoManager manager;
  initList(&list);
  initManager(&manager);

  char line[10];
  int operations = 0;

  printf("Bat dau Benchmark 1 trieu thao tac...\n");
  clock_t start = clock(); // Bấm đồng hồ bắt đầu!

  // Đọc từng dòng trong file
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == 'I') {
      char c = line[2];
      insertChar(&list, c);
      recordInsert(&manager, &list, &c, 1);
    } else if (line[0] == 'L') {
      moveCursorLeft(&list);
    } else if (line[0] == 'R') {
      moveCursorRight(&list);
    } else if (line[0] == 'D') {
      recordDeleteChar(&manager, &list);
      deleteChar(&list);
    } else if (line[0] == 'U') {
      if (canUndo(&manager)) {
        undo(&manager, &list);
      }
    } else if (line[0] == 'Y') {
      if (canRedo(&manager)) {
        redo(&manager, &list);
      }
    }
    operations++;
  }

  clock_t end = clock(); // Dừng đồng hồ!

  double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
  printf("Thuc thi %d thao tac trong %f giay!\n", operations, time_spent);

  fclose(f);
  destroyList(&list);
  return 0;
}
