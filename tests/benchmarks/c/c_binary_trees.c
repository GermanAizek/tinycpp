#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int item;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

static TreeNode *create_tree(int item, int depth) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->item = item;
    if (depth > 0) {
        node->left = create_tree(2 * item - 1, depth - 1);
        node->right = create_tree(2 * item, depth - 1);
    } else {
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

static long long check_tree(TreeNode *node) {
    if (!node) return 0;
    if (!node->left) return node->item;
    return node->item + check_tree(node->left) - check_tree(node->right);
}

static void free_tree(TreeNode *node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        free(node);
    }
}

int main(void) {
    int max_depth = 16;
    long long checksum = 0;

    TreeNode *stretch = create_tree(0, max_depth + 1);
    checksum += check_tree(stretch);
    free_tree(stretch);

    TreeNode *long_lived = create_tree(0, max_depth);

    for (int d = 4; d <= max_depth; d += 2) {
        int iterations = 1 << (max_depth - d + 4);
        for (int i = 1; i <= iterations; i++) {
            TreeNode *t1 = create_tree(i, d);
            TreeNode *t2 = create_tree(-i, d);
            checksum += check_tree(t1) + check_tree(t2);
            free_tree(t1);
            free_tree(t2);
        }
    }

    checksum += check_tree(long_lived);
    free_tree(long_lived);

    printf("BinaryTrees: Depth=%d Checksum=%lld\n", max_depth, checksum);
    return 0;
}
