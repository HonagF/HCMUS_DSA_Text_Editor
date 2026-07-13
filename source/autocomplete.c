#include"autocomplete.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct TrieNode *create_node() {
    struct TrieNode *node = (struct TrieNode *)malloc(sizeof(struct TrieNode));
    node->is_end_of_word = false;
    for (int i = 0; i < 26; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void trie_insert(TrieNode* root, const char* word){
    struct TrieNode *current = root;

    for (int i = 0; i < strlen(word); i++) {
        int index = (unsigned char)word[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = create_node();
        }
        current = current->children[index];
    }
    current->is_end_of_word = 1;
}

int suggest(TrieNode* root, const char* prefix, char results[MAX_SUGGESTIONS][MAX_WORD_LEN]){

    return 0;
}
void load_txt(TrieNode* root, const char* filename){
    FILE *fi = fopen(filename,"rt");
    if (!fi) {
        return;
    }
    fclose(fi);
}