//
//  TTree.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/8/31.
//  Copyright © 2019 yuan. All rights reserved.
//

#include <stdlib.h>
#include <memory.h>
#include "TTree.h"
#include "Queue.h"

static int TTreeNodeChildCountBits_s = 8;

#define _PREV_CHECK_TREE(TREE, CR, ERR_RET)     do{ \
                                                    if (!TREE) return ERR_RET; \
                                                    _checkInitTTree(TREE, CR); \
                                                    if (!TREE->root) return ERR_RET; \
                                                }while(0);


static TTreeNode_S *_defaultTTreeNodeAlloc()
{
    TTreeNode_S *treeNode = calloc(1, sizeof(TTreeNode_S));
    return treeNode;
}

static void _defaultTTreeNodeInit(struct TTree *tree, TTreeNode_S *node)
{
}


static void _defaultTTreeNodeDealloc(struct TTree *tree, TTreeNode_S *node)
{
    if (node) {
        if (node->key.dealloc) {
            node->key.dealloc(&node->key);
        }
    }
}

static void _defaultTTreeNodeFree(struct TTree *tree, TTreeNode_S *node)
{
    if (node) {
        tree->dealloc(tree, node);
        free(node);
    }
}

static uint16_t _defaultTTreeNodeChildCountFunc(struct TTree *tree, uint16_t level)
{
    return (1 << TTreeNodeChildCountBits_s) - 1;
}

static uint16_t _defaultTTreeNodeChildIndexFunc(struct TTree *tree, struct TTreeNode *currentNode, TTreeNode_S *insertNode)
{
    uint16_t cnt = currentNode->childNodeCount;
    T *key = &currentNode->key;
    if (key->size == 0) {
        uint64_t val = key->V.val;
        val >> 
    }
    else if (key->size > 0) {
        return <#expression#>
    }
    else {
        
    }
    
    return 0;
}

static YZHComparisonResult_E _defaultTTreeNodeKeyCompareFunc(struct TTree *tree, struct TTreeNode *first, struct TTreeNode *second)
{
    return compare(&first->key, &second->key);
}

static struct TTreeNode * _createTTreeNode(struct TTree *tree, uint16_t level)
{
    TTreeNode_S *node = tree->alloc();
    if (node == NULL) {
        return NULL;
    }
    node->level = level;
    node->childNodeCount = tree->childCountFunc(tree, level);
    node->ptrChildNodeList = calloc(node->childNodeCount, sizeof(PTTreeNode_S));
    if (node->ptrChildNodeList == NULL) {
        return NULL;
    }
    return node;
}

static int8_t _setupTTreeNodeLevel(struct TTree *tree, struct TTreeNode *node, uint16_t level)
{
    uint8_t childCount = tree->childCountFunc(tree, level);
    if (node->childNodeCount != childCount) {
        node->level = level;
        node->childNodeCount = childCount;
        if (node->ptrChildNodeList) {
            free(node->ptrChildNodeList);
        }
        node->ptrChildNodeList = calloc(node->childNodeCount, sizeof(PTTreeNode_S));
        if (node->ptrChildNodeList == NULL) {
            return 0;
        }
    }
    return 1;
}

QueueNode_S *_queueNodeWithTTreeNode(struct TTreeNode *node)
{
    QueueNode_S *qnode = calloc(1,sizeof(QueueNode_S));
    if (qnode) {
        qnode->value = node;
    }
    return qnode;
}

static void _ZEnumerateTTree(struct TTree *tree, struct TTreeNode *node, TTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    Queue_S *queue = calloc(1, sizeof(Queue_S));
    Queue_S *queueTmp = calloc(1, sizeof(Queue_S));
    //申请一个缓存
    Queue_S *cache = calloc(1, sizeof(Queue_S));
    
    if (queue == NULL || queueTmp == NULL) {
        goto ZEnumerateTTree_END;
    }
    
    push(queue, _queueNodeWithTTreeNode(node));
    while (queue->count > 0 ) {
        QueueNode_S *qnode = pop(queue);
        TTreeNode_S *treeNode = qnode->value;
        
        //首先保存子节点
        for (uint8_t i = 0; i < treeNode->childNodeCount; ++i) {
            TTreeNode_S *child = treeNode->ptrChildNodeList[i];
            if (child) {
                QueueNode_S *qnodeTmp = pop(cache);
                if (!qnodeTmp) {
                    qnodeTmp = _queueNodeWithTTreeNode(child);
                }
                else {
                    qnodeTmp->value = child;
                }
                if (qnodeTmp->value) {
                    push(queueTmp, qnodeTmp);
                }
            }
        }
        
        //遍历回调
        enumerator(tree, treeNode);
        
        //回收qnode进入缓存
        qnode->value = NULL;
        push(cache, qnode);
        
        if (queue->count == 0 && queueTmp->count > 0) {
            Queue_S *q = queue;
            queue = queueTmp;
            queueTmp = q;
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
    
    
ZEnumerateTTree_END:
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

static int8_t _checkInitTTree(struct TTree *tree, int8_t createRoot) {
    if (!tree->alloc) {
        tree->alloc = _defaultTTreeNodeAlloc;
    }
    if (!tree->init) {
        tree->init = _defaultTTreeNodeInit;
    }
    if (!tree->dealloc) {
        tree->dealloc = _defaultTTreeNodeDealloc;
    }
    if (!tree->free) {
        tree->free = _defaultTTreeNodeFree;
    }
    if (!tree->keyCompare) {
        tree->keyCompare = _defaultTTreeNodeKeyCompareFunc;
    }
    if (!tree->compare) {
        tree->compare = _defaultTTreeNodeKeyCompareFunc;
    }
    if (!tree->childCountFunc) {
        tree->childCountFunc = _defaultTTreeNodeChildCountFunc;
    }
    if (!tree->childIndexFunc) {
        tree->childIndexFunc = _defaultTTreeNodeChildIndexFunc;
    }
    if (createRoot && tree->root == NULL) {
        tree->root = _createTTreeNode(tree, 0);
    }
    return 1;
}


//返回0表示失败，大于0表示成功
int8_t _insertTTreeWithNode(struct TTree *tree, struct TTreeNode *node)
{
    _PREV_CHECK_TREE(tree, 1, 0);

    int8_t ret = 1;
    struct TTreeNode *current = tree->root;
    while (current) {
        int8_t childIdx = tree->childIndexFunc(tree, current, node);
        YZHComparisonResult_E comp = tree->keyCompare(tree, current, node);
        if (comp == YZHOrderedEQ) {
            if (current->ptrKeySameLastNode) {
                current->keySameNodeCount += 1;
                struct TTreeNode *last = current->ptrKeySameLastNode;
                last->ptrKeySameNextNode = node;
                node->ptrKeySamePrevNode = last;
                current->ptrKeySameLastNode = node;
                current = last;
                continue;
            }
            node->ptrKeySamePrevNode = current;
            current->ptrKeySameLastNode = current->ptrKeySameNextNode = node;
            current->keySameNodeCount = 1;
        }
        if (childIdx >= 0 && childIdx < current->childNodeCount) {
            if (current->ptrChildNodeList[childIdx] == NULL) {
                current->ptrChildNodeList[childIdx] = node;
                _setupTTreeNodeLevel(tree, node, current->level + 1);
                break;
            }
            else {
                current = current->ptrChildNodeList[childIdx];
            }
        }
        else {
            ret = 0;
            break;
        }
    }
    if (ret) {
        ++tree->count;
    }
    return ret;
}

struct TTree *allocTTree(void)
{
    struct TTree *tree = calloc(1, sizeof(struct TTree));
    return tree;
}

struct TTreeNode *insertTTree(struct TTree *tree, T *key, void *val)
{
    _PREV_CHECK_TREE(tree, 1, NULL);
    
    TTreeNode_S *treeNode = tree->alloc();
    treeNode->key = *key;
    treeNode->value = val;
    
    _insertTTreeWithNode(tree, treeNode);
    
    return treeNode;
}

struct TTreeNode *deleteTTree(struct TTree *tree, T *key)
{
    _PREV_CHECK_TREE(tree, 1, NULL);
    
    struct TTreeNode *findNode = selectTTree(tree, key);
    if (findNode != NULL && findNode->state != TTreeNodeStateDelete) {
        findNode->state = TTreeNodeStateDelete;
        tree->dealloc(tree, findNode);
    }
    return findNode;
}

struct TTreeNode *selectTTree(struct TTree *tree, T *key)
{
    _PREV_CHECK_TREE(tree, 1, NULL);
    
    struct TTreeNode node = {.key = *key};
    
    struct TTreeNode *current = tree->root;
    
    while (current) {
        int8_t childIdx = tree->childIndexFunc(tree, current, &node);
        YZHComparisonResult_E comp = tree->keyCompare(tree, current, &node);
        if (comp == YZHOrderedEQ) {
            if (current->ptrKeySameLastNode) {
                do {
                    if (current->state != TTreeNodeStateDelete) {
                        YZHComparisonResult_E r = tree->compare(tree, current, &node);
                        if (r == YZHOrderedEQ) {
                            return current;
                        }
                    }
                    current = current->ptrKeySameNextNode;
                } while (current);
            }
            break;
        }
        if (childIdx >= 0 && childIdx < current->childNodeCount) {
            current = current->ptrChildNodeList[childIdx];
        }
        else {
            break;
        }
    }
    if (current && current->state != TTreeNodeStateDelete) {
        return current;
    }
    return NULL;
}

void enumerateTTree(struct TTree *tree, TTreeNodeEnumerateFunc enumerator)
{
    _PREV_CHECK_TREE(tree, 0, );
    
    if (enumerator == NULL) {
        enumerator = tree->enumerator;
    }
    _ZEnumerateTTree(tree, tree->root, enumerator);
}

void _TTreeClearEnumerate(struct TTree *tree, TTreeNode_S *node)
{
    if (node) {
        memset(node->ptrChildNodeList, 0 , node->childNodeCount * sizeof(struct TTreeNode *));
        node->ptrKeySameNextNode = NULL;
        node->ptrKeySamePrevNode = NULL;
        node->ptrKeySameLastNode = NULL;
    }
    if (tree->free) {
        tree->free(tree, node);
    }
}

void clearTTree(struct TTree *tree)
{
    _PREV_CHECK_TREE(tree, 0, );
    
    enumerateTTree(tree, _TTreeClearEnumerate);
}

//先清空，再free
void freeTTree(struct TTree *tree)
{
    _PREV_CHECK_TREE(tree, 0, );
    clearTTree(tree);
    if (tree) {
        memset(tree, 0, sizeof(struct TTree));
        free(tree);
    }
}

void printTTreeNode(struct TTreeNode *node)
{
    printf("levele:%d, key=%lld\n",node->level,node->key.V.val);
}
