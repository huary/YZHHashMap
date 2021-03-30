//
//  HashTree.h
//  YZHHashMapDemo
//
//  Created by yuan on 2019/8/24.
//  Copyright © 2019 yuan. All rights reserved.
//

#ifndef HashTree_h
#define HashTree_h
#include <stdio.h>
#include "YZHType.h"

typedef enum HashTreeNodeState
{
    HashTreeNodeStateNormal     = 0,
    HashTreeNodeStateDelete     = 1,
}HashTreeNodeState_E;


typedef struct HashTreeNode {
    uint8_t level;
    uint8_t childNodeCount;
    uint32_t keySameNodeCount;
    HashTreeNodeState_E state;
    T key;
    T value;
    uint64_t hashValue;
    void *userInfo;
    struct HashTreeNode **ptrChildNodeList;
    struct HashTreeNode *ptrKeySameNextNode;
    struct HashTreeNode *ptrKeySamePrevNode;
    struct HashTreeNode *ptrKeySameLastNode;
}HashTreeNode_S, *PHashTreeNode_S;


struct HashTree;
typedef HashTreeNode_S *(*HashTreeNodeAllocFunc)(void);
typedef void (*HashTreeNodeInitFunc)(struct HashTree *tree, HashTreeNode_S *node);
typedef void (*HashTreeNodeDeallocFunc)(struct HashTree *tree, HashTreeNode_S *node);
typedef void (*HashTreeNodeFreeFunc)(struct HashTree *tree, HashTreeNode_S *node);


typedef uint64_t (*HashTreeHashFunc)(T *key);
typedef YZHComparisonResult_E (*HashTreeNodeCompareFunc)(struct HashTree *tree, HashTreeNode_S *first, HashTreeNode_S *second);
typedef uint8_t (*HashTreeNodeChildCountFunc)(struct HashTree *tree, uint8_t level);
//如果返回小于0或者大于currentNode->childNodeCount时，挂载到ptrKeySameNextNode列表上
typedef int8_t (*HashTreeNodeChildIndexFunc)(struct HashTree *tree, struct HashTreeNode *currentNode, HashTreeNode_S *insertNode);
typedef void (*HashTreeNodeEnumerateFunc)(struct HashTree *tree, struct HashTreeNode *node);


typedef struct HashTree {
    HashTreeNode_S *root;
    int32_t count;
    void *userInfo;

    HashTreeNodeAllocFunc alloc;
    HashTreeNodeInitFunc init;
    HashTreeNodeDeallocFunc dealloc;
    HashTreeNodeFreeFunc free;
    
    HashTreeHashFunc hashFunc;
    HashTreeNodeCompareFunc compareFunc;
    HashTreeNodeChildCountFunc childCountFunc;
    HashTreeNodeChildIndexFunc childIndexFunc;
    HashTreeNodeEnumerateFunc enumerator;
}HashTree_S, *PHashTree_S;


struct HashTree *allocHashTree(void);
HashTreeNode_S *insertHashTree(struct HashTree *tree, T *key, T *val);
HashTreeNode_S *deleteHashTree(struct HashTree *tree, T *key);
HashTreeNode_S *selectHashTree(struct HashTree *tree, T *key);

/*
 *遍历hash树,enumerator为NULL时取用RBTree里面的enumerator，都为NULL的话，直接返回
 */
void enumerateHashTree(struct HashTree *tree, HashTreeNodeEnumerateFunc enumerator);


//清空HashTree
void clearHashTree(struct HashTree *tree);
//先清空，再free
void freeHashTree(struct HashTree *tree);

void printTreeNode(struct HashTreeNode *node);


#endif /* HashTree_h */
