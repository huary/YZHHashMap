//
//  TTree.h
//  YZHHashMapDemo
//
//  Created by yuan on 2019/8/31.
//  Copyright © 2019 yuan. All rights reserved.
//

#ifndef TTree_h
#define TTree_h

#include <stdio.h>
#include "YZHType.h"


typedef enum TTreeNodeState
{
    TTreeNodeStateNormal     = 0,
    TTreeNodeStateDelete     = 1,
}TTreeNodeState_E;

//typedef enum TTreeNodeKeyType
//{
//    TTreeNodeKeyTypeLittleStart     = 0,
//    TTreeNodeKeyTypeBigStart        = 1,
//}TTreeNodeKeyType_E;


typedef struct TTreeNode {
    uint16_t level;
//    uint16_t childNodeCount;
    uint16_t childBitsIndex;
    uint8_t childBitsCount;
    uint32_t keySameNodeCount;
    TTreeNodeState_E state;
    T key;
    void *value;
    void *userInfo;
    struct TTreeNode **ptrChildNodeList;
    struct TTreeNode *ptrKeySameNextNode;
    struct TTreeNode *ptrKeySamePrevNode;
    struct TTreeNode *ptrKeySameLastNode;
}TTreeNode_S, *PTTreeNode_S;


struct TTree;
typedef struct TTreeNode *(*TTreeNodeAllocFunc)(void);
typedef void (*TTreeNodeInitFunc)(struct TTree *tree, TTreeNode_S *node);
typedef void (*TTreeNodeDeallocFunc)(struct TTree *tree, TTreeNode_S *node);
typedef void (*TTreeNodeFreeFunc)(struct TTree *tree, TTreeNode_S *node);

typedef YZHComparisonResult_E (*TTreeNodeCompareFunc)(struct TTree *tree, TTreeNode_S *first, TTreeNode_S *second);
typedef uint16_t (*TTreeNodeChildCountFunc)(struct TTree *tree, uint16_t level);
//如果返回小于0或者大于currentNode->childNodeCount时，挂载到ptrKeySameNextNode列表上
typedef uint16_t (*TTreeNodeChildIndexFunc)(struct TTree *tree, struct TTreeNode *currentNode, TTreeNode_S *insertNode);
typedef void (*TTreeNodeEnumerateFunc)(struct TTree *tree, struct TTreeNode *node);


typedef struct TTree {
    struct TTreeNode *root;
    int64_t count;
    void *userInfo;
    
    TTreeNodeAllocFunc alloc;
    TTreeNodeInitFunc init;
    TTreeNodeDeallocFunc dealloc;
    TTreeNodeFreeFunc free;
    
    TTreeNodeCompareFunc compare;
    TTreeNodeCompareFunc keyCompare;
    TTreeNodeChildCountFunc childCountFunc;
    TTreeNodeChildIndexFunc childIndexFunc;
    TTreeNodeEnumerateFunc enumerator;
}TTree_S, *PTTree_S;


struct TTree *allocTTree(void);
struct TTreeNode *insertTTree(struct TTree *tree, T *key, void *val);
struct TTreeNode *deleteTTree(struct TTree *tree, T *key);
struct TTreeNode *selectTTree(struct TTree *tree, T *key);

/*
 *遍历hash树,enumerator为NULL时取用RBTree里面的enumerator，都为NULL的话，直接返回
 */
void enumerateTTree(struct TTree *tree, TTreeNodeEnumerateFunc enumerator);


//清空TTree
void clearTTree(struct TTree *tree);
//先清空，再free
void freeTTree(struct TTree *tree);

void printTTreeNode(struct TTreeNode *node);



#endif /* TTree_h */
