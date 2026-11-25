#ifndef BST_H
#define BST_H
#include "patient.h"

typedef struct BSTNode {
    char id[ID_LEN];
    Patient *patient;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

int bst_insert(BSTNode **root, Patient *patient);
Patient *bst_search(BSTNode *root, const char *id);
void bst_inorder(BSTNode *root);
void bst_free(BSTNode *root);

#endif
