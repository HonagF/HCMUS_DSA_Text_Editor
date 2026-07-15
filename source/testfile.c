#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "list.h"
#include "undo_redo.h"

static int failCount = 0;

/* Đọc nội dung list ra buffer để so sánh */
static void listToStr(EditorList *list, char *buf) {
  int i = 0;
  for (Node *n = list->head; n != NULL; n = n->next)
    buf[i++] = n->data;
  buf[i] = '\0';
}

static void check(const char *testName, EditorList *list, const char *expected) {
  char buf[256];
  listToStr(list, buf);
  if (strcmp(buf, expected) == 0) {
    printf("[PASS] %-30s -> \"%s\"\n", testName, buf);
  } else {
    printf("[FAIL] %-30s -> got \"%s\", expected \"%s\"\n", testName, buf, expected);
    failCount++;
  }
}

static void checkInt(const char *testName, int got, int expected) {
  if (got == expected) {
    printf("[PASS] %-30s -> %d\n", testName, got);
  } else {
    printf("[FAIL] %-30s -> got %d, expected %d\n", testName, got, expected);
    failCount++;
  }
}

/* ===================== Test 1: Insert + Undo/Redo cơ bản ===================== */
static void test_basic_insert_undo_redo(void) {
  printf("\n-- Test 1: Insert + Undo/Redo co ban --\n");
  EditorList list; initList(&list);
  UndoRedoManager mgr; initManager(&mgr);

  recordInsert(&mgr, &list, 'a');
  recordInsert(&mgr, &list, 'b');
  recordInsert(&mgr, &list, 'c');
  check("insert a,b,c", &list, "abc");

  undo(&mgr, &list);
  check("undo 1 lan", &list, "ab");

  undo(&mgr, &list);
  check("undo 2 lan", &list, "a");

  redo(&mgr, &list);
  check("redo 1 lan", &list, "ab");

  redo(&mgr, &list);
  check("redo 2 lan", &list, "abc");

  redo(&mgr, &list); /* redo stack rong -> khong doi gi */
  check("redo khi rong (khong doi)", &list, "abc");

  destroyManager(&mgr);
  destroyList(&list);
}

/* ===================== Test 2: Go moi sau khi undo -> xoa redo stack ===================== */
static void test_redo_cleared_on_new_insert(void) {
  printf("\n-- Test 2: Go moi phai xoa Redo stack --\n");
  EditorList list; initList(&list);
  UndoRedoManager mgr; initManager(&mgr);

  recordInsert(&mgr, &list, 'a');
  recordInsert(&mgr, &list, 'b');
  recordInsert(&mgr, &list, 'c');
  undo(&mgr, &list);
  undo(&mgr, &list);
  check("sau 2 lan undo", &list, "a");
  checkInt("canRedo truoc khi go moi", canRedo(&mgr), 1);

  recordInsert(&mgr, &list, 'x'); /* redo stack phai bi xoa */
  check("go 'x' de", &list, "ax");
  checkInt("canRedo sau khi go moi", canRedo(&mgr), 0);

  redo(&mgr, &list); /* khong co gi de redo */
  check("redo sau khi da bi xoa (khong doi)", &list, "ax");

  destroyManager(&mgr);
  destroyList(&list);
}

/* ===================== Test 3: Backspace (deleteChar / DELETE_LEFT) ===================== */
static void test_backspace_undo_redo(void) {
  printf("\n-- Test 3: Backspace (DELETE_LEFT) --\n");
  EditorList list; initList(&list);
  UndoRedoManager mgr; initManager(&mgr);

  recordInsert(&mgr, &list, 'a');
  recordInsert(&mgr, &list, 'b');
  recordInsert(&mgr, &list, 'c');
  check("insert a,b,c", &list, "abc");

  recordDeleteChar(&mgr, &list); /* backspace 'c' */
  check("backspace 'c'", &list, "ab");

  undo(&mgr, &list);
  check("undo backspace -> khoi phuc 'c'", &list, "abc");

  redo(&mgr, &list);
  check("redo backspace -> xoa lai 'c'", &list, "ab");

  destroyManager(&mgr);
  destroyList(&list);
}

/* ===================== Test 4: Delete phai (deleteRight / DELETE_RIGHT) ===================== */
static void test_delete_right_undo_redo(void) {
  printf("\n-- Test 4: Delete phai (DELETE_RIGHT) --\n");
  EditorList list; initList(&list);
  UndoRedoManager mgr; initManager(&mgr);

  recordInsert(&mgr, &list, 'a');
  recordInsert(&mgr, &list, 'b');
  recordInsert(&mgr, &list, 'c'); /* cursor dang o 'c', text = abc */
  check("insert a,b,c", &list, "abc");

  moveCursorLeft(&list); /* cursor -> 'b', dung truoc 'c' */
  moveCursorLeft(&list); /* cursor -> 'a', dung truoc 'b' */

  recordDeleteRight(&mgr, &list); /* xoa 'b' (ben phai cursor), cursor van o 'a' */
  check("deleteRight xoa 'b'", &list, "ac");
  checkInt("index_cursor khong doi sau deleteRight", list.index_cursor, 1);

  undo(&mgr, &list);
  check("undo deleteRight -> khoi phuc 'b'", &list, "abc");
  checkInt("index_cursor dung sau undo", list.index_cursor, 1);

  redo(&mgr, &list);
  check("redo deleteRight -> xoa lai 'b'", &list, "ac");

  destroyManager(&mgr);
  destroyList(&list);
}

/* ===================== Test 5: Hon hop nhieu hanh dong ===================== */
static void test_mixed_operations(void) {
  printf("\n-- Test 5: Hon hop insert/backspace/deleteRight --\n");
  EditorList list; initList(&list);
  UndoRedoManager mgr; initManager(&mgr);

  recordInsert(&mgr, &list, 'h');
  recordInsert(&mgr, &list, 'e');
  recordInsert(&mgr, &list, 'l');
  recordInsert(&mgr, &list, 'l');
  recordInsert(&mgr, &list, 'o');
  check("insert 'hello'", &list, "hello");

  recordDeleteChar(&mgr, &list); /* backspace 'o' */
  check("backspace 'o'", &list, "hell");

  moveCursorLeft(&list); moveCursorLeft(&list); /* cursor -> giua 'e' va 'l' dau */
  recordDeleteRight(&mgr, &list); /* xoa 'l' dau */
  check("deleteRight xoa 'l' dau", &list, "hel");

  recordInsert(&mgr, &list, 'X');
  check("chen 'X'", &list, "heXl");

  /* Undo 4 lan lien tiep, phai tra ve dung trinh tu nguoc */
  undo(&mgr, &list); check("undo chen 'X'", &list, "hel");
  undo(&mgr, &list); check("undo deleteRight 'l'", &list, "hell");
  undo(&mgr, &list); check("undo backspace 'o'", &list, "hello");
  undo(&mgr, &list); check("undo insert 'o'", &list, "hell");

  /* Redo lai het */
  redo(&mgr, &list); check("redo insert 'o'", &list, "hello");
  redo(&mgr, &list); check("redo backspace 'o'", &list, "hell");
  redo(&mgr, &list); check("redo deleteRight 'l'", &list, "hel");
  redo(&mgr, &list); check("redo chen 'X'", &list, "heXl");

  destroyManager(&mgr);
  destroyList(&list);
}

/* ===================== Test 6: Edge case - undo/redo khi rong ===================== */
static void test_empty_stack_edge_cases(void) {
  printf("\n-- Test 6: Edge case stack rong --\n");
  EditorList list; initList(&list);
  UndoRedoManager mgr; initManager(&mgr);

  checkInt("canUndo khi chua lam gi", canUndo(&mgr), 0);
  checkInt("canRedo khi chua lam gi", canRedo(&mgr), 0);

  undo(&mgr, &list); /* khong duoc crash */
  redo(&mgr, &list); /* khong duoc crash */
  check("undo/redo tren list rong (khong doi)", &list, "");

  /* backspace/deleteRight tren list rong khong duoc tao Command */
  recordDeleteChar(&mgr, &list);
  recordDeleteRight(&mgr, &list);
  checkInt("canUndo sau backspace/deleteRight tren list rong", canUndo(&mgr), 0);

  destroyManager(&mgr);
  destroyList(&list);
}

int main(void) {
  test_basic_insert_undo_redo();
  test_redo_cleared_on_new_insert();
  test_backspace_undo_redo();
  test_delete_right_undo_redo();
  test_mixed_operations();
  test_empty_stack_edge_cases();

  printf("\n============================\n");
  if (failCount == 0) {
    printf("TAT CA TEST PASS\n");
  } else {
    printf("%d TEST THAT BAI\n", failCount);
  }
  return failCount == 0 ? 0 : 1;
}