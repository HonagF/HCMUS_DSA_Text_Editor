#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  FILE *f = fopen("input.txt", "w");
  if (!f)
    return 1;
  srand(time(NULL));
  // Sinh 1 triệu thao tác ngẫu nhiên
  for (int i = 0; i < 1000000; i++) {
    int r = rand() % 100;
    if (r < 50)
      fprintf(f, "I %c\n", 'a' + rand() % 26); // 50% là lệnh Insert
    else if (r < 65)
      fprintf(f, "L\n"); // 15% Move Left
    else if (r < 75)
      fprintf(f, "R\n"); // 10% Move Right
    else if (r < 85)
      fprintf(f, "D\n"); // 10% Delete
    else if (r < 95)
      fprintf(f, "U\n"); // 10% Undo
    else
      fprintf(f, "Y\n"); // 5% Redo
  }
  fclose(f);
  printf("Da sinh file input.txt (1 trieu dong)\n");
  return 0;
}
