#include "bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bst_insert(BSTNode **root, Patient *patient) {
    if (*root == NULL) {
        BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
        if (!node) return 0;
        strcpy(node->id, patient->id);
        node->patient = patient; node->left = node->right = NULL;
        *root = node; return 1;
    }
    int cmp = strcmp(patient->id, (*root)->id);
    if (cmp == 0) return 0;
    return cmp < 0 ? bst_insert(&(*root)->left, patient)
                   : bst_insert(&(*root)->right, patient);
}

Patient *bst_search(BSTNode *root, const char *id) {
    while (root) {
        int cmp = strcmp(id, root->id);
        if (cmp == 0) return root->patient;
        root = cmp < 0 ? root->left : root->right;
    }
    return NULL;
}

void bst_inorder(BSTNode *root) {
    if (!root) return;
    bst_inorder(root->left); patient_print(root->patient); bst_inorder(root->right);
}

void bst_free(BSTNode *root) {
    if (!root) return;
    bst_free(root->left); bst_free(root->right); free(root);
}
