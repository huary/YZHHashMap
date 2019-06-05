//
//  RBTree.h
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/1.
//  Copyright © 2019年 yuan. All rights reserved.
//

#ifndef RBTree_h
#define RBTree_h
#include <stdio.h>

#define INTEGER_SWAP(X,Y)       (X==Y ?:(X=X^Y, Y=X^Y, X=X^Y))

typedef enum RBTreeNodeComparisonResult{
    ASC     = -1,
    SAME    = 0,
    DESC    = 1,
}RBTreeNodeComparisonResult_E;

typedef enum RBTreeDeleteType{
    //后继点
    RBTreeDeleteTypeNext    = 0,
    //前继点
    RBTreeDeleteTypePrev    = 1,
    //程序自动判断判断取后继还是前继
    RBTreeDeleteTypeAuto    = 2,
}RBTreeDeleteType_E;

typedef enum RBTreeNodeColor{
    RED             = 0,
    BLACK           = 1,
}RBTreeNodeColor_E;

//二叉树的遍历方法
typedef enum BTreeEnumerateType{
    //前序遍历
    BTreeEnumerateTypeNLR  = 0,
    //中序遍历(L-N-R),从小到大的顺序
    BTreeEnumerateTypeLNR  = 1,
    //中序遍历(R-N-L)，从大到小的顺序
    BTreeEnumerateTypeRNL  = 2,
    //后序遍历
    BTreeEnumerateTypeLRN  = 3,
    //Z形遍历
    BTreeEnumerateTypeZ    = 4,
}BTreeEnumerateType_E;



typedef struct RBTreeNode {
    struct RBTreeNode *parent;
    struct RBTreeNode *left;
    struct RBTreeNode *right;
    RBTreeNodeColor_E color;
    uint64_t key;
    void *value;
    void *userInfo;
}RBTreeNode_S,*PRBTreeNode_S;



typedef RBTreeNodeComparisonResult_E (*RBTreeNodeCompareFunc)(RBTreeNode_S *first, RBTreeNode_S *second);
typedef void (*RBTreeNodeCopyValueFunc)(RBTreeNode_S *src, RBTreeNode_S *dst);
typedef void (*PBTreeNodeSwapValueFunc)(RBTreeNode_S *first, RBTreeNode_S *second);
typedef void (*RBTreeNodeReleaseFunc)(RBTreeNode_S *node);
typedef void (*RBTreeNodeEnumerateFunc)(RBTreeNode_S *node, int32_t level);

typedef struct RBTree {
    RBTreeNode_S *root;
    int32_t count;
    RBTreeDeleteType_E deleteType;
    RBTreeNodeCompareFunc compare;
    RBTreeNodeCopyValueFunc copy;
    PBTreeNodeSwapValueFunc swap;
    RBTreeNodeReleaseFunc release;
    RBTreeNodeEnumerateFunc enumerator;
}RBTree_S,*PRBTree_S;

void insertRBTree(struct RBTree *tree, struct RBTreeNode *node);
void deleteRBTree(struct RBTree *tree, struct RBTreeNode *node);
struct RBTreeNode *selectRBTree(struct RBTree *tree, struct RBTreeNode *node);

/*
 *下面以Key的进行操作的话，需要在compare中以key进行比较
 */
//insertRBTreeWithKey返回的RBTreeNode需要进行free();
struct RBTreeNode *insertRBTreeWithKey(struct RBTree *tree, int64_t key);
void deleteRBTreeWithKey(struct RBTree *tree, int64_t key);
struct RBTreeNode *selectRBTreeWithKey(struct RBTree *tree, int64_t key);

/*
 *遍历红黑树,enumerator为NULL时取用RBTree里面的enumerator，都为NULL的话，直接返回
 */
void enumerateRBTree(struct RBTree *tree, BTreeEnumerateType_E enumerateType, RBTreeNodeEnumerateFunc enumerator);

#endif /* RBTree_h */
