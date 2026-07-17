#ifndef AUTO_H
#define AUTO_H

#define ALPHABET_SIZE 256
#define MAX_SUGGESTIONS 10
#define MAX_WORD_LEN 256

typedef struct TrieNode {
  char data;
  int is_end_of_word;
  struct TrieNode *child;
  struct TrieNode *sibling;
} TrieNode;

TrieNode *create_node(char c);
void trie_insert(TrieNode *root, const char *word);
void find_words(struct TrieNode *node, char *current_word, int depth,
                char results[MAX_SUGGESTIONS][MAX_WORD_LEN], int *found_count);
int suggest(TrieNode *root, const char *prefix,
            char results[MAX_SUGGESTIONS][MAX_WORD_LEN]);
void load_txt(TrieNode *root, const char *filename);

#endif