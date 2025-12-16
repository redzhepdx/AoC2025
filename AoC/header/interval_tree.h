#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Interval {
    uint64_t low;
    uint64_t high;
} Interval;

typedef struct ITNode {
    Interval interval;  // [low, high]
    uint64_t max;       // max high in this subtree
    struct ITNode* left;
    struct ITNode* right;
} ITNode;

uint64_t max2(uint64_t a, uint64_t b) {
    return (a > b) ? a : b;
}

uint64_t max3(uint64_t a, uint64_t b, uint64_t c) {
    return max2(max2(a, b), c);
}

void recalcMax(ITNode* node) {
    if (!node) return;
    uint64_t leftMax = node->left ? node->left->max : 0;  // INT_MIN
    uint64_t rightMax = node->right ? node->right->max : 0;
    node->max = max3(node->interval.high, leftMax, rightMax);
}

ITNode* newNode(Interval i) {
    ITNode* node = (ITNode*)malloc(sizeof(ITNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->interval = i;
    node->max = i.high;
    node->left = node->right = NULL;
    return node;
}

bool doOverlap(Interval a, Interval b) {
    return (a.low <= b.high && b.low <= a.high);
}

ITNode* insert(ITNode* root, Interval i) {
    if (!root) return newNode(i);

    if (i.low < root->interval.low) {
        root->left = insert(root->left, i);
    } else {
        root->right = insert(root->right, i);
    }

    recalcMax(root);
    return root;
}

ITNode* overlapSearch(ITNode* root, Interval i) {
    if (!root) return NULL;

    if (doOverlap(root->interval, i))
        return root;

    if (root->left && root->left->max >= i.low)
        return overlapSearch(root->left, i);

    return overlapSearch(root->right, i);
}

ITNode* findMinNode(ITNode* root) {
    while (root && root->left)
        root = root->left;
    return root;
}

ITNode* deleteNode(ITNode* root, Interval i) {
    if (!root) return NULL;

    if (i.low < root->interval.low) {
        root->left = deleteNode(root->left, i);
    } else if (i.low > root->interval.low) {
        root->right = deleteNode(root->right, i);
    } else {
        // Found node with matching low; assume high matches as well
        if (!root->left) {
            ITNode* r = root->right;
            free(root);
            return r;
        } else if (!root->right) {
            ITNode* l = root->left;
            free(root);
            return l;
        } else {
            // Two children: replace with inorder successor
            ITNode* succ = findMinNode(root->right);
            root->interval = succ->interval;
            root->right = deleteNode(root->right, succ->interval);
        }
    }

    recalcMax(root);
    return root;
}

ITNode* insertAndMerge(ITNode* root, Interval i) {
    while (1) {
        ITNode* over = overlapSearch(root, i);
        if (!over) break;

        // Expand i to cover the union
        if (over->interval.low < i.low) i.low = over->interval.low;
        if (over->interval.high > i.high) i.high = over->interval.high;

        // Remove the overlapping interval from the tree
        root = deleteNode(root, over->interval);
    }

    // Insert the merged interval
    root = insert(root, i);
    return root;
}


bool containsPoint(ITNode* root, uint64_t x, Interval* out_interval) {
    Interval point = {x, x};
    ITNode* res = overlapSearch(root, point);
    if (res) {
        if (out_interval) *out_interval = res->interval;
        return true;
    }
    return false;
}


void inorder(ITNode* root) {
    if (!root) return;
    inorder(root->left);
    printf("[%" PRIu64 ", %" PRIu64 "] max=%" PRIu64 "\n",
           root->interval.low, root->interval.high, root->max);
    inorder(root->right);
}

void freeTree(ITNode* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}


// uint64_t main(void) {
//     ITNode *root = NULL;

//     Interval arr[] = {
//         {15, 20},
//         {10, 30},
//         {17, 19},
//         {5, 20},
//         {12, 15},
//         {30, 40},
//         {18, 35} 
//     };

//     uint64_t n = sizeof(arr) / sizeof(arr[0]);

//     for (uint64_t i = 0; i < n; ++i) {
//         root = insertAndMerge(root, arr[i]);
//     }

//     printf("Inorder traversal after insert-and-merge:\n");
//     inorder(root);
//     printf("\n");

//     uint64_t x = 14;
//     Interval found;
//     if (containsPoint(root, x, &found)) {
//         printf("Point %d is in interval [%d, %d]\n",
//                x, found.low, found.high);
//     } else {
//         printf("Point %d is not inside any interval\n", x);
//     }

//     freeTree(root);
//     return 0;
// }
