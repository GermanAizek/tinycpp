#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct HNode {
    uint8_t ch;
    int freq;
    struct HNode *left, *right;
} HNode;

static HNode* build_tree(const int freq[256]) {
    HNode* heap[256];
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HNode *n = (HNode*)malloc(sizeof(HNode));
            n->ch = (uint8_t)i; n->freq = freq[i]; n->left = n->right = NULL;
            heap[size++] = n;
        }
    }
    while (size > 1) {
        int min1 = 0, min2 = 1;
        if (heap[min1]->freq > heap[min2]->freq) { int t = min1; min1 = min2; min2 = t; }
        for (int i = 2; i < size; i++) {
            if (heap[i]->freq < heap[min1]->freq) { min2 = min1; min1 = i; }
            else if (heap[i]->freq < heap[min2]->freq) { min2 = i; }
        }
        HNode *parent = (HNode*)malloc(sizeof(HNode));
        parent->ch = 0; parent->freq = heap[min1]->freq + heap[min2]->freq;
        parent->left = heap[min1]; parent->right = heap[min2];
        
        heap[min1] = parent;
        heap[min2] = heap[size - 1];
        size--;
    }
    return size > 0 ? heap[0] : NULL;
}

static void free_tree(HNode *n) {
    if (!n) return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

int main(void) {
    int freq[256] = {0};
    const char *sample = "the quick brown fox jumps over the lazy dog repeatedly for compression testing";
    size_t len = strlen(sample);
    for (size_t i = 0; i < len; i++) freq[(uint8_t)sample[i]]++;
    
    int total_weight = 0;
    for (int iter = 0; iter < 10000; iter++) {
        freq[97] = (iter % 50) + 1; // 'a' = 97
        HNode *root = build_tree(freq);
        if (root) {
            total_weight += root->freq;
            free_tree(root);
        }
    }
    printf("CompressHuffman: TotalWeight=%d\n", total_weight);
    return 0;
}
