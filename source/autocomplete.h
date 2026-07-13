#ifndef AUTO_H
#define AUTO_H

#define ALPHABET_SIZE 256
#define MAX_SUGGESTIONS 10
#define MAX_WORD_LEN 256

#include <gtk/gtk.h>
#include "api.h" 

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int is_end_of_word;
} TrieNode;
TrieNode *create_node();

void trie_insert(TrieNode* root, const char* word);
int suggest(TrieNode* root, const char* prefix, char results[MAX_SUGGESTIONS][MAX_WORD_LEN]);
void load_txt(TrieNode* root, const char* filename);

#endif