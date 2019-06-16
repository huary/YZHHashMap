//
//  RBTree.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/1.
//  Copyright © 2019年 yuan. All rights reserved.
//

#include <stdlib.h>
#include <memory.h>
#include "RBTree.h"
#include "Queue.h"

#define NODE_PARENT(PTR_NODE)   (PTR_NODE ? PTR_NODE->parent : NULL)

#define NODE_BROTHER(PTR_NODE)   (PTR_NODE ? (PTR_NODE->parent ? (PTR_NODE->parent->left == PTR_NODE ? PTR_NODE->parent->right : PTR_NODE->parent->left) : NULL) : (NULL))


#define LEFT_ROTATE(TREE,PTR_NODE)  do {\
                                        if(TREE && PTR_NODE && PTR_NODE->right) { \
                                            struct RBTreeNode *node_T = PTR_NODE;\
                                            struct RBTreeNode *parent_T = node_T->parent;\
                                            struct RBTreeNode *right_T = node_T->right;\
                                            /*1、先修改right的left节点*/\
                                            node_T->right = right_T->left; \
                                            if (right_T->left) right_T->left->parent = node_T;\
                                            /*2、修改right节点*/\
                                            right_T->parent = parent_T;\
                                            right_T->left = node_T;\
                                            if (node_T == TREE->root) {TREE->root = right_T;}\
                                            else if (parent_T->left == node_T) {parent_T->left = right_T;}\
                                            else {parent_T->right = right_T;} \
                                            /*3、修改PTR_NODE节点*/\
                                            node_T->parent = right_T;\
                                        }\
                                    }while(0)

#define RIGHT_ROTATE(TREE,PTR_NODE)  do {\
                                        if(TREE && PTR_NODE && PTR_NODE->left) { \
                                            struct RBTreeNode *node_T = PTR_NODE;\
                                            struct RBTreeNode *parent_T = node_T->parent;\
                                            struct RBTreeNode *left_T = node_T->left;\
                                            /*1、先修改left的right节点*/\
                                            node_T->left = left_T->right; \
                                            if (left_T->right) left_T->right->parent = node_T;\
                                            /*2、修改left节点*/\
                                            left_T->parent = parent_T;\
                                            left_T->right = node_T;\
                                            if (node_T == TREE->root) {TREE->root = left_T;}\
                                            else if (parent_T->left == node_T) {parent_T->left = left_T;}\
                                            else {parent_T->right = left_T;}\
                                            /*3、修改PTR_NODE节点*/\
                                            node_T->parent = left_T;\
                                        }\
                                    }while(0)

#define IS_RED_NODE(PTR_NODE)       (PTR_NODE && PTR_NODE->color == RED)
#define IS_BLACK_NODE(PTR_NODE)     (!IS_RED_NODE(PTR_NODE))
#define IS_NON_NULL_BLACK_NODE(PTR_NODE)    (PTR_NODE && PTR_NODE->color = BLACK)

static RBTreeNode_S *defaultRBTreeNodeAlloc()
{
    RBTreeNode_S *treeNode = calloc(1, sizeof(RBTreeNode_S));
    return treeNode;
}

static void defaultRBTreeNodeFree(struct RBTree *tree, RBTreeNode_S *node)
{
    if (node) {
        free(node);
    }
}

static RBTreeNodeComparisonResult_E defaultRBTreeCompare(struct RBTree *tree, struct RBTreeNode *first,struct RBTreeNode *second)
{
    return compare(&first->key, &second->key);
}

static void defaultCopyValue(struct RBTree *tree, RBTreeNode_S *src, RBTreeNode_S *dst)
{
    if (src == NULL || dst == NULL) {
        return;
    }
    dst->key = src->key;
    dst->value = src->value;
    dst->userInfo = src->userInfo;
}

static void defaultSwapValue(struct RBTree *tree, RBTreeNode_S *first, RBTreeNode_S *second)
{
    if (first == NULL || second == NULL) {
        return;
    }
//    INTEGER_SWAP(first->key, second->key);
    swap(&first->key, &second->key);
    PTR_SWAP(first->value, second->value);
    PTR_SWAP(first->userInfo, second->userInfo);
}

inline static RBTreeNode_S *loopupInsertNode(struct RBTree *tree, struct RBTreeNode *node, int8_t condition)
{
    if (tree->root == NULL) {
        return NULL;
    }
    struct RBTreeNode *cur = tree->root;
    while (cur) {
        RBTreeNodeComparisonResult_E result = tree->compare(tree, node, cur);
        if (result == YZHOrderedEQ) {
            return cur;
        }
        else if (result == YZHOrderedASC) {
            if (cur->left) {
                cur = cur->left;
            }
            else {
                return condition ? NULL : cur;
            }
        }
        else if (result == YZHOrderedDES) {
            if (cur->right) {
                cur = cur->right;
            }
            else {
                return condition ? NULL : cur;
            }
        }
    }
    return NULL;
}


inline static RBTreeNode_S *loopupNearNode(struct RBTreeNode *node, uint8_t next)
{
    if (!node) {
        return NULL;
    }
    if (next) {
        RBTreeNode_S *right = node->right;
//        if (!right) {
//            return node;
//        }
        while (right && right->left) {
            right = right->left;
        }
        return right;
    }
    else {
        RBTreeNode_S *left = node->left;
//        if (!left) {
//            return node;
//        }
        while (left && left->right) {
            left = left->right;
        }
        return left;
    }
}

inline static void checkRBTree(struct RBTree *tree)
{
    if (!tree->alloc) {
        tree->alloc = defaultRBTreeNodeAlloc;
    }
    if (!tree->free) {
        tree->free = defaultRBTreeNodeFree;
    }
    if (!tree->compare) {
        tree->compare = defaultRBTreeCompare;
    }
    if (!tree->copy) {
        tree->copy = defaultCopyValue;
    }
    if (!tree->swap) {
        tree->swap = defaultSwapValue;
    }
}

inline static void deleteRedLeafNode(struct RBTreeNode *redNode)
{
    if (redNode == NULL || redNode->left || redNode->right) {
        return;
    }
    RBTreeNode_S *parent = redNode->parent;
    if (parent) {
        if (redNode == parent->left) {
            parent->left = NULL;
        }
        else if (redNode == parent->right) {
            parent->right = NULL;
        }
    }
}


int8_t insertRBTree(struct RBTree *tree, struct RBTreeNode *node)
{
    if (tree == NULL || node == NULL) {
        return 0;
    }
    checkRBTree(tree);
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;

    //1、如果是根节点，把修改为黑色，直接返回
    if (tree->root == NULL) {
        tree->root = node;
        node->color = BLACK;
        ++tree->count;
        return 1;
    }
    
    node->color = RED;
    RBTreeNode_S *parent = loopupInsertNode(tree, node, 0);
    if (!parent) {
        //这里就返回失败
        return 0;
    }
    //这里返回成功
    RBTreeNodeComparisonResult_E result = tree->compare(tree, node, parent);
    if (result == YZHOrderedASC) {
        parent->left = node;
        node->parent = parent;
    }
    else if (result == YZHOrderedEQ) {
        tree->copy(tree, node, parent);
    }
    else {
        parent->right = node;
        node->parent = parent;
    }
    ++tree->count;
    //2、如果父节点是黑色,直接返回
    if (IS_BLACK_NODE(parent)) {
        return 1;
    }
    
    RBTreeNode_S *uncle = NODE_BROTHER(parent);
    RBTreeNode_S *cur = node;
    //3、如果父节点是红色,且叔叔也为红色
    while (1) {
        
        int8_t haveAdjust = 0;
        
        if (IS_RED_NODE(parent) && IS_RED_NODE(uncle)) {
            parent->color = BLACK;
            uncle->color = BLACK;
            
            cur = parent->parent;
            cur->color = RED;
            
            parent = cur->parent;
            uncle = NODE_BROTHER(parent);
            
            haveAdjust = 1;
        }
        
        /*
         *当前节点的父节点是红色,叔叔节点是黑色，当前节点是其父节点的右子,cur，parent，parent->parent不在一条直线上,
         *这里父节点是红色，肯定存在祖父节点
         *方法：当前节点的父节点做为新的当前节点，以新当前节点为支点左旋，，成一条直线上
         */
        if (IS_RED_NODE(parent) && IS_BLACK_NODE(uncle) && cur == parent->right && parent == parent->parent->left) {
            cur = parent;
            LEFT_ROTATE(tree, cur);
            
            parent = cur->parent;
            uncle = NODE_BROTHER(parent);
            
            haveAdjust = 1;
        }
        
        /*
         *当前节点的父节点是红色,叔叔节点是黑色，当前节点是其父节点的左子,cur，parent，parent->parent不在一条直线上,
         *这里父节点是红色，肯定存在祖父节点
         *方法：当前节点的父节点做为新的当前节点，以新当前节点为支点右旋，成一条直线上
         */
        if (IS_RED_NODE(parent) && IS_BLACK_NODE(uncle) && cur == parent->left && parent == parent->parent->right) {
            cur = parent;
            RIGHT_ROTATE(tree, cur);
            
            parent = cur->parent;
            uncle = NODE_BROTHER(parent);
            
            haveAdjust = 1;
        }
        
        /*
         *当前节点的父节点是红色,叔叔节点是黑色，当前节点是其父节点的左子,cur，parent，parent->parent在一条直线上
         *这里父节点是红色，肯定存在祖父节点
         *父节点变为黑色，祖父节点变为红色，在祖父节点为支点右旋
         */
        if (IS_RED_NODE(parent) && IS_BLACK_NODE(uncle) && cur == parent->left && parent == parent->parent->left) {
            parent->color = BLACK;
            parent->parent->color = RED;

            RIGHT_ROTATE(tree,parent->parent);
            
            parent = cur->parent;
            uncle = NODE_BROTHER(parent);
            
            haveAdjust = 1;
            
            break;
        }
        
        /*
         *当前节点的父节点是红色,叔叔节点是黑色，当前节点是其父节点的右子,cur，parent，parent->parent在一条直线上
         *这里父节点是红色，肯定存在祖父节点
         *父节点变为黑色，祖父节点变为红色，在祖父节点为支点左旋
         */
        if (IS_RED_NODE(parent) && IS_BLACK_NODE(uncle) && cur == parent->right && parent == parent->parent->right) {
            parent->color = BLACK;
            parent->parent->color = RED;
            
            LEFT_ROTATE(tree, parent->parent);
            
            parent = cur->parent;
            uncle = NODE_BROTHER(parent);
            
            haveAdjust = 1;
            
            break;
        }
        
        if (cur == tree->root || parent->parent == NULL || haveAdjust == 0) {
            break;
        }
    }
    
    tree->root->color = BLACK;
    return 1;
}


RBTreeNode_S * deleteRBTree(struct RBTree *tree, struct RBTreeNode *node)
{
    if (tree == NULL || tree->root == NULL || node == NULL) {
        return NULL;
    }
    checkRBTree(tree);

    struct RBTreeNode *delete = loopupInsertNode(tree, node, 1);
    
    //是红色节点，没有叶子节点
    if (IS_RED_NODE(delete)) {
        if (delete->left && delete->right == NULL) {
            //这是不存在的情况，违背红黑树的平衡（到达任一结束节点的黑子节点数目相同）
        }
        else if (delete->left == NULL && delete->right) {
            //这是不存在的情况，违背红黑树的平衡（到达任一结束节点的黑子节点数目相同）
        }
        else if (delete->left == NULL && delete->right == NULL) {
            deleteRedLeafNode(delete);
            tree->free(tree, delete);
            --tree->count;
            return delete;
        }
    }
    //如果是根节点
    else if (delete == tree->root && delete->left == NULL && delete->right == NULL) {
        tree->free(tree, delete);
        tree->root = NULL;
        --tree->count;
        if (tree->count != 0) {
            printf("出现严重的错误了，需要检查算法");
        }
        return delete;
    }
    
    RBTreeNode_S *nearNode = NULL;
    RBTreeDeleteType_E type = RBTreeDeleteTypeNext;
    if (tree->deleteType == RBTreeDeleteTypeNext) {
        nearNode = loopupNearNode(delete, 1);
        type = RBTreeDeleteTypeNext;
        if (nearNode == NULL) {
            nearNode = loopupNearNode(delete, 0);
            type = RBTreeDeleteTypePrev;
        }
    }
    else if (tree->deleteType == RBTreeDeleteTypePrev) {
        nearNode = loopupNearNode(delete, 0);
        type = RBTreeDeleteTypePrev;
        if (nearNode == NULL) {
            nearNode = loopupNearNode(delete, 1);
            type = RBTreeDeleteTypeNext;
        }
    }
    else {
        RBTreeNode_S *next = loopupNearNode(delete, 1);
        if (IS_RED_NODE(next)) {
            nearNode = next;
            type = RBTreeDeleteTypeNext;
        }
        else {
            RBTreeNode_S *prev = loopupNearNode(delete, 0);
            if (IS_RED_NODE(prev)) {
                nearNode = prev;
                type = RBTreeDeleteTypePrev;
            }
            else {
                nearNode = next;
                type = RBTreeDeleteTypeNext;
            }
        }
    }
    if (nearNode == NULL) {
        nearNode = delete;
    }
    /*
     *将后继节点和删除节点的值进行交换,真正需要删除的是nearNode，nearNode存放着实际要删除的内容
     *nearNode需要先调整
     */
    if (nearNode != delete) {
        tree->swap(tree, nearNode, delete);
    }
    if (IS_RED_NODE(nearNode)) {
        //只有红色叶子节点一种情况,直接进行删除
        deleteRedLeafNode(nearNode);
        tree->free(tree, nearNode);
    }
    else {
        //因为没有黑色的nil节点，所以先对nearNode进行调整
        RBTreeNode_S *X = nearNode;
        if ((type == RBTreeDeleteTypeNext && IS_RED_NODE(X->right)) ||
            (type == RBTreeDeleteTypePrev && IS_RED_NODE(X->left))) {
            if (type == RBTreeDeleteTypeNext) {
                X->right->color = BLACK;
            }
            else {
                X->left->color = BLACK;
            }
        }
        else {
            //这里的nearNode其实就是删除的节点
            while (1) {
                /**************************************************************************************************
                 //case1,delete的兄弟为红色,改变p与w的颜色，
                 同时对p做一次左旋。这样就将情况1转变为情况2,3,4的一种。
                 ***************************************************************************************************/
                RBTreeNode_S *brotherX = NODE_BROTHER(X);
                RBTreeNode_S *parentX = NODE_PARENT(X);
                if (IS_RED_NODE(brotherX)) {
                    parentX->color = RED;
                    brotherX->color = BLACK;
                    
                    //这里的旋转需要判断下
                    if (brotherX == parentX->right) {
                        LEFT_ROTATE(tree, parentX);
                    }
                    else {
                        RIGHT_ROTATE(tree, parentX);
                    }
                    brotherX = NODE_BROTHER(X);
                    parentX = NODE_PARENT(X);
                }
                
                /**************************************************************************************************
                 //case2,delete的兄弟为黑色,w的两个儿子也都是黑的，出现双黑的情况,
                 因为x子树相对于其兄弟w子树少一个黑色节点，可以将w置为红色，这样，x子树与w子树黑色节点一致，保持了平衡。
                 newX为x与w的父亲。newX相对于它的兄弟节点newW少一个黑色节点。如果newX为红色，则将newX置为黑，则整棵树平衡。
                 否则，情况2转变为情况1,2,3,4
                 ***************************************************************************************************/
                if (IS_BLACK_NODE(brotherX)) {
                    if (IS_BLACK_NODE(brotherX->left) && IS_BLACK_NODE(brotherX->right)) {
                        brotherX->color = RED;
                        if (IS_RED_NODE(parentX)) {
                            parentX->color = BLACK;
                            break;
                        }
                        else {
                            X = parentX;
                            brotherX = NODE_BROTHER(X);
                            parentX = NODE_PARENT(X);
                            if (X == tree->root) {
                                break;
                            }
                        }
                    }
                    //右分支的情况
                    if (brotherX == parentX->right) {
                        //case3,brotherX为右分支
                        /**************************************************************************************************
                         //case3,delete的兄弟w为黑色,w的左孩子是红色，右孩子黑色或者为空
                         交换w与左孩子的颜色，对w进行右旋，情况3转变为情况4
                         ***************************************************************************************************/
                        if (IS_RED_NODE(brotherX->left) && IS_BLACK_NODE(brotherX->right)) {
                            brotherX->color = RED;
                            brotherX->left->color = BLACK;
                            RIGHT_ROTATE(tree, brotherX);
                            brotherX = NODE_BROTHER(X);
                        }
                        
                        //case4,brotherX为右分支
                        /**************************************************************************************************
                         //case4,delete的兄弟w为黑色,右孩子红色
                         交换w与父亲p颜色，同时对p做左旋，将w的右儿子置黑。这样左边缺失的黑色就补回来了，这样左右都达到平衡。
                         ***************************************************************************************************/
                        if (IS_RED_NODE(brotherX->right) /*&& IS_BLACK_NODE(brotherX->left) */) {
                            brotherX->right->color = BLACK;
                            brotherX->color = parentX->color;
                            parentX->color = BLACK;
                            LEFT_ROTATE(tree, parentX);
                            break;
                        }
                    }
                    //左分支的情况
                    else if (brotherX == parentX->left) {
                        //case3
                        /**************************************************************************************************
                         //case3,delete的兄弟w为黑色,w的右孩子是红色，左孩子黑色或者为空
                         交换w与左孩子的颜色，对w进行左旋，情况3转变为情况4
                         ***************************************************************************************************/
                        if (IS_RED_NODE(brotherX->right) && IS_BLACK_NODE(brotherX->left)) {
                            brotherX->color = RED;
                            brotherX->right->color = BLACK;
                            LEFT_ROTATE(tree, brotherX);
                            brotherX = NODE_BROTHER(X);
                        }
                        //case4
                        /**************************************************************************************************
                         //case4,delete的兄弟w为黑色,左孩子红色
                         交换w与父亲p颜色，同时对p做右旋，将w的左儿子置黑。这样右边缺失的黑色就补回来了，这样左右都达到平衡。
                         ***************************************************************************************************/
                        if (IS_RED_NODE(brotherX->left) /*&& IS_BLACK_NODE(brotherX->right)*/) {
                            brotherX->left->color = BLACK;
                            brotherX->color = parentX->color;
                            parentX->color = BLACK;
                            RIGHT_ROTATE(tree, parentX);
                            break;
                        }
                    }
                }
            }
        }
        //调整后删除nearNode
        if (type == RBTreeDeleteTypeNext) {
            X = nearNode->right;
            if (X) {
                X->parent = nearNode->parent;
            }
            //修改nearNode所指向的left或者是right
            if (nearNode == nearNode->parent->left) {
                nearNode->parent->left = X;
            }
            else {
                nearNode->parent->right = X;
            }
        }
        else {
            X = nearNode->left;
            if (X) {
                X->parent = nearNode->parent;
            }
            //修改nearNode所指向的left或者right
            if (nearNode == nearNode->parent->left) {
                nearNode->parent->left = X;
            }
            else {
                nearNode->parent->right = X;                
            }
        }
        //删除nearNode节点
        tree->free(tree, nearNode);
    }
    --tree->count;
    return nearNode;
}


RBTreeNode_S *selectRBTree(struct RBTree *tree, struct RBTreeNode *node)
{
    RBTreeNode_S *find = loopupInsertNode(tree, node, 1);
    return find;
}

RBTreeNode_S *insertRBTreeWithKey(struct RBTree *tree, int64_t key)
{
    RBTreeNode_S *insert = tree->alloc();
    if (insert) {
        T t = {.V.val = key};
        insert->key = t;
        insertRBTree(tree, insert);
    }
    return insert;
}

RBTreeNode_S *deleteRBTreeWithKey(struct RBTree *tree, int64_t key)
{
    RBTreeNode_S node;
    memset(&node, 0, sizeof(RBTreeNode_S));
    T t = {.V.val = key};
    node.key = t;
    return deleteRBTree(tree, &node);
}

RBTreeNode_S *selectRBTreeWithKey(struct RBTree *tree, int64_t key)
{
    RBTreeNode_S node;
    memset(&node, 0, sizeof(RBTreeNode_S));
    T t = {.V.val = key};
    node.key = t;
    return selectRBTree(tree, &node);
}

//前序
static void NLREnumerateRBTree(struct RBTree *tree, struct RBTreeNode *node, RBTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    enumerator(tree, node, 0);
    NLREnumerateRBTree(tree, node->left, enumerator);
    NLREnumerateRBTree(tree, node->right, enumerator);
}

//中序遍历(L-N-R),从小到大的顺序
static void LNREnumerateRBTree(struct RBTree *tree, struct RBTreeNode *node, RBTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    LNREnumerateRBTree(tree, node->left, enumerator);
    enumerator(tree, node, 0);
    LNREnumerateRBTree(tree, node->right, enumerator);
}

//中序遍历(R-N-L)，从大到小的顺序
static void RNLEnumerateRBTree(struct RBTree *tree, struct RBTreeNode *node, RBTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    RNLEnumerateRBTree(tree, node->right, enumerator);
    enumerator(tree, node, 0);
    RNLEnumerateRBTree(tree, node->left, enumerator);
}
//后序遍历
static void LRNEnumerateRBTree(struct RBTree *tree, struct RBTreeNode *node, RBTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    LRNEnumerateRBTree(tree, node->left, enumerator);
    LRNEnumerateRBTree(tree, node->right, enumerator);
    enumerator(tree, node, 0);
}

QueueNode_S *queueNodeWithRBTreeNode(struct RBTreeNode *node)
{
    QueueNode_S *qnode = calloc(1,sizeof(QueueNode_S));
    if (qnode) {
        qnode->value = node;
    }
    return qnode;
}

static void ZEnumerateRBTree(struct RBTree *tree, struct RBTreeNode *node, RBTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    Queue_S *queue = calloc(1, sizeof(Queue_S));
    Queue_S *queueTmp = calloc(1, sizeof(Queue_S));
    //申请一个缓存
    Queue_S *cache = calloc(1, sizeof(Queue_S));
    
    if (queue == NULL || queueTmp == NULL) {
        goto ZEnumerateRBTree_END;
    }
    
    int32_t level = 1;
    push(queue, queueNodeWithRBTreeNode(node));
    while (queue->count > 0) {
        QueueNode_S *qnode = pop(queue);
        RBTreeNode_S *treeNode = qnode->value;
        
        //保存left
        QueueNode_S *nl = pop(cache);
        if (!nl) {
            nl = queueNodeWithRBTreeNode(treeNode->left);
        }
        else {
            nl->value = treeNode->left;
        }
        if (nl->value) {
            push(queueTmp, nl);
        }
        
        //保存right
        QueueNode_S *nr = pop(cache);
        if (!nr) {
            nr = queueNodeWithRBTreeNode(treeNode->right);
        }
        else {
            nr->value = treeNode->right;
        }
        if (nr->value) {
            push(queueTmp, nr);
        }
        
        //遍历回调
        enumerator(tree, treeNode, level);
        
        //回收qnode进入缓存
        qnode->value = NULL;
        push(cache, qnode);
        
        if (queue->count == 0 && queueTmp->count > 0) {
            Queue_S *t = queue;
            queue = queueTmp;
            queueTmp = t;
            ++level;
        }
    }
    
    //释放缓存
    while (cache->count > 0) {
        QueueNode_S *qnode = pop(cache);
        if (qnode) {
            free(qnode);
            qnode = NULL;            
        }
    }

ZEnumerateRBTree_END:
    if (cache) {
        free(cache);
        cache = NULL;
    }
    if (queue) {
        free(queue);
        queue = NULL;
    }
    if (queueTmp) {
        free(queueTmp);
        queueTmp = NULL;
    }
}


void enumerateRBTree(struct RBTree *tree, BTreeEnumerateType_E enumerateType, RBTreeNodeEnumerateFunc enumerator)
{
    if (tree == NULL || tree->root == NULL) {
        return;
    }
    if (enumerator == NULL) {
        enumerator = tree->enumerator;
    }
    if (enumerateType == BTreeEnumerateTypeNLR) {
        NLREnumerateRBTree(tree, tree->root, enumerator);
    }
    else if (enumerateType == BTreeEnumerateTypeLNR) {
        LNREnumerateRBTree(tree, tree->root, enumerator);
    }
    else if (enumerateType == BTreeEnumerateTypeRNL) {
        RNLEnumerateRBTree(tree, tree->root, enumerator);
    }
    else if (enumerateType == BTreeEnumerateTypeLRN) {
        LRNEnumerateRBTree(tree, tree->root, enumerator);
    }
    else if (enumerateType == BTreeEnumerateTypeZ) {
        ZEnumerateRBTree(tree, tree->root, enumerator);
    }
}


void RBTreeClearEnumerate(struct RBTree *tree, RBTreeNode_S *node, int32_t level)
{
    if (node) {
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
    }
    if (tree->free) {
        tree->free(tree, node);
    }
}

void clearTree(struct RBTree *tree)
{
    enumerateRBTree(tree, BTreeEnumerateTypeZ, RBTreeClearEnumerate);
}






