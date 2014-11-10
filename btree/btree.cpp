#include <stdio.h>
#include "btree.h"

BTNode *insert_node(BTNode *btree, int data) {
    BTNode *pTmp = btree;
    BTNode node;

    node.left = node.right = NULL;
    node.data = data;

    /* TODO: */
    while (pTmp) {
        if (data < pTmp->data) {
            pTmp = pTmp->left; 
        }
        else if (data > pTmp->data) {
            pTmp = pTmp->right;
        }
    }
    pTmp = &node;
    return pTmp;
}

BTNode *create_btree(BTNode *btree, int data[], int num) {
    if ((0 >= num) || (NULL == data)) {
        return btree;
    }

    for (int ii = 0; ii < num; ii++) {
        insert_node(btree, num);
    }
    return btree;
}

int main(int argc, char *argv[]) {
    printf("Binary Tree Test Sample\n");

    /* Create BTree */
    BTNode *btree = NULL;
    return 0;
}
