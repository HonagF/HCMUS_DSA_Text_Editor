#include "autocomplete.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TrieNode *create_node(char c) {
  struct TrieNode *node = (struct TrieNode *)malloc(sizeof(struct TrieNode));
  node->data = c;
  node->is_end_of_word = 0;
  node->child = NULL;
  node->sibling = NULL;
  return node;
}
void find_words(struct TrieNode *node, char *current_word, int depth,
                char results[MAX_SUGGESTIONS][MAX_WORD_LEN], int *found_count) {
  // Base Case 1: Stop if we hit a dead end, or if we already found enough
  // suggestions
  if (node == NULL || *found_count >= MAX_SUGGESTIONS) {
    return;
  }

  // Base Case 2: If the current node marks the end of a valid word, save it
  if (node->is_end_of_word) {
    current_word[depth] = '\0'; // Properly null-terminate the string
    strcpy(results[*found_count], current_word); // Copy to the 2D array
    (*found_count)++; // Increment our successful find counter
  }

  // Recursive Step: Check all 26 possible alphabetical children
  TrieNode *child = node->child;
  while (child != NULL) {
    current_word[depth] = child->data;
    find_words(child, current_word, depth + 1, results, found_count);
    child = child->sibling;
  }
}
void trie_insert(TrieNode *root, const char *word) {
  struct TrieNode *current = root;

  for (int i = 0; word[i] != '\0'; i++) {
    char c = word[i];
    if (current->child == NULL) {
      current->child = create_node(c);
      current = current->child;
      continue;
    }
    TrieNode *temp = current->child;
    while (temp != NULL) {
      if (temp->data == c) {
        current = temp;
        break;
      }
      if (temp->sibling == NULL) {
        temp->sibling = create_node(c);
        current = temp->sibling;
        break;
      }
      temp = temp->sibling;
    }
  }
  current->is_end_of_word = 1;
}

int suggest(struct TrieNode *root, const char *prefix,
            char results[MAX_SUGGESTIONS][MAX_WORD_LEN]) {
  if (root == NULL || prefix == NULL) {
    return 0;
  }

  struct TrieNode *current = root;
  int prefix_len = strlen(prefix);

  // STEP 1: Navigate to the end of the prefix
  for (int i = 0; i < prefix_len; i++) {
    char c = prefix[i];
    current = current->child;
    while (current != NULL && current->data != c) {
      current = current->sibling;
    }
    if (current == NULL) {
      return 0;
    }
  }

  // STEP 2: Prepare variables for the recursive search
  int found_count = 0;
  char word_buffer[MAX_WORD_LEN];

  // Copy the original prefix into our working buffer so we can append to it
  strcpy(word_buffer, prefix);

  // STEP 3: Launch the search starting from the node where the prefix ended
  find_words(current, word_buffer, prefix_len, results, &found_count);

  // Return the actual number of suggestions found (between 0 and
  // MAX_SUGGESTIONS)
  return found_count;
}
void load_txt(TrieNode *root, const char *filename) {
  FILE *fi = fopen(filename, "rt");
  if (fi == NULL) {
    return;
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), fi)) {
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (buffer[0] != '\0')
      trie_insert(root, buffer);
  }
  fclose(fi);
}