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
void find_words(struct TrieNode* node, char* current_word, int depth, char results[MAX_SUGGESTIONS][MAX_WORD_LEN], int* found_count) {
    // Base Case 1: Stop if we hit a dead end, or if we already found enough suggestions
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
    for (int i = 0; i < 26; i++) {
        if (node->children[i] != NULL) {
            // Append this child's character to our running word buffer
            current_word[depth] = 'a' + i;
            
            // Plunge one level deeper into the tree
            find_words(node->children[i], current_word, depth + 1, results, found_count);
        }
    }
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
int suggest(struct TrieNode* root, const char* prefix, char results[MAX_SUGGESTIONS][MAX_WORD_LEN]) {
    if (root == NULL || prefix == NULL) {
        return 0;
    }

    struct TrieNode* current = root;
    int prefix_len = strlen(prefix);

    // STEP 1: Navigate to the end of the prefix
    for (int i = 0; i < prefix_len; i++) {
        int index = prefix[i] - 'a';
        
        // If the path breaks before the prefix finishes, the word doesn't exist
        if (current->children[index] == NULL) {
            return 0; 
        }
        current = current->children[index];
    }

    // STEP 2: Prepare variables for the recursive search
    int found_count = 0;
    char word_buffer[MAX_WORD_LEN];
    
    // Copy the original prefix into our working buffer so we can append to it
    strcpy(word_buffer, prefix);

    // STEP 3: Launch the search starting from the node where the prefix ended
    find_words(current, word_buffer, prefix_len, results, &found_count);

    // Return the actual number of suggestions found (between 0 and MAX_SUGGESTIONS)
    return found_count;
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