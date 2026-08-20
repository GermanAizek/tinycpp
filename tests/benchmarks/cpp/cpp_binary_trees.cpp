#include <stdio.h>
#include <stdlib.h>

class TreeNode {
public:
    int item;
    TreeNode *left;
    TreeNode *right;
};

TreeNode *create_tree_cpp(int item, int depth) {
    TreeNode *node = new TreeNode;
    node->item = item;
    if (depth > 0) {
        node->left = create_tree_cpp(2 * item - 1, depth - 1);
        node->right = create_tree_cpp(2 * item, depth - 1);
    } else {
        node->left = nullptr;
        node->right = nullptr;
    }
    return node;
}

long long check_tree_cpp(TreeNode *node) {
    if (node == nullptr) return 0;
    if (node->left == nullptr) return node->item;
    return node->item + check_tree_cpp(node->left) - check_tree_cpp(node->right);
}

void destroy_tree_cpp(TreeNode *node) {
    if (node != nullptr) {
        destroy_tree_cpp(node->left);
        destroy_tree_cpp(node->right);
        delete node;
    }
}

int main() {
    int max_depth = 16;
    long long checksum = 0;

    TreeNode *stretch = create_tree_cpp(0, max_depth + 1);
    checksum += check_tree_cpp(stretch);
    destroy_tree_cpp(stretch);

    TreeNode *long_lived = create_tree_cpp(0, max_depth);

    for (int d = 4; d <= max_depth; d += 2) {
        int iterations = 1 << (max_depth - d + 4);
        for (int i = 1; i <= iterations; i++) {
            TreeNode *t1 = create_tree_cpp(i, d);
            TreeNode *t2 = create_tree_cpp(-i, d);
            checksum += check_tree_cpp(t1) + check_tree_cpp(t2);
            destroy_tree_cpp(t1);
            destroy_tree_cpp(t2);
        }
    }

    checksum += check_tree_cpp(long_lived);
    destroy_tree_cpp(long_lived);

    printf("CppBinaryTrees: Depth=%d Checksum=%lld\n", max_depth, checksum);
    return 0;
}
