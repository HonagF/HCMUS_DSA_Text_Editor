#include"autocomplete.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

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

    for (int i = 0; word[i]!='\0'; i++) {
        if(word[i]<'a'||word[i]>'z')continue;

        int index = word[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = create_node();
        }
        current = current->children[index];
    }
    current->is_end_of_word = 1;
}
void sanitize(const char  *in,char *out){
    int j = 0;
    for(int i = 0; in[i]!='\0';i++){
        if(isalpha(in[i])){
            out[j]=tolower(in[i]);
            j++;
        }
    }
    out[j]='\0';
    return ;
}
int suggest(TrieNode* root, const char* prefix, char results[MAX_SUGGESTIONS][MAX_WORD_LEN]){

    return 0;
}
void load_txt(TrieNode* root, const char* filename){
    FILE *fi = fopen(filename,"rt");
    if (fi==NULL) {
        return;
    }
    char buffer[256];
    while(fgets(buffer, sizeof(buffer), fi)){
        buffer[strcspn(buffer,"\r\n")]='\0';
        char clean[256];
        sanitize(buffer,clean);
        if(clean[0]!='\0') trie_insert(root,clean);
    }
    fclose(fi);
}