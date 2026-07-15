#include "autocomplete.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_dictionary(const char *filename) {
  printf("Testing dictionary: %s\n", filename);
  TrieNode *root = create_node('\0');
  load_txt(root, filename);

  FILE *fi = fopen(filename, "rt");
  if (!fi) {
    printf("Failed to open %s\n", filename);
    return 0;
  }

  char buffer[256];
  int total_words = 0;
  int passed = 0;

  while (fgets(buffer, sizeof(buffer), fi)) {
    buffer[strcspn(buffer, "\r\n")] = '\0';
    if (buffer[0] == '\0')
      continue;

    total_words++;

    char out[MAX_SUGGESTIONS][MAX_WORD_LEN] = {0};
    // Kiểm tra xem gợi ý có hoạt động khi gõ chữ này không
    int num_suggestions = suggest(root, buffer, out);

    int found = 0;
    // Kiểm tra xem chính chữ đó có nằm trong kết quả gợi ý không
    for (int i = 0; i < num_suggestions; i++) {
      if (strcmp(out[i], buffer) == 0) {
        found = 1;
        break;
      }
    }

    if (found) {
      passed++;
    } else {
      printf("FAIL: Target \"%s\" not in suggestions!\n", buffer);
    }
  }
  fclose(fi);

  printf("Result: %d / %d passed.\n\n", passed, total_words);
  return passed == total_words;
}

int main() {
  printf("===================================\n");
  printf("   AUTO-COMPLETE UNIT TEST SUITE   \n");
  printf("===================================\n\n");

  int eng = test_dictionary("../data/google-10000-english-no-swears.txt");
  int vie = test_dictionary("../data/Viet11K.txt");

  if (eng && vie) {
    printf(">>> ALL TESTS PASSED SUCCESSFULLY! <<<\n");
  } else {
    printf(">>> SOME TESTS FAILED! <<<\n");
  }
  return 0;
}
