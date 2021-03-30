//
//  HashTree.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/8/24.
//  Copyright © 2019 yuan. All rights reserved.
//

#include <stdlib.h>
#include <memory.h>
#include "HashTree.h"
#include "YZHHash.h"
#include "Queue.h"

static uint8_t hashTreePrimeList_s[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 100};
static int32_t hashSeed_s = 4;

#define _PREV_CHECK_TREE(TREE, CR, ERR_RET)     do{ \
                                                    if (!TREE) return ERR_RET; \
                                                    _checkInitHashTree(TREE, CR); \
                                                    if (!TREE->root) return ERR_RET; \
                                                }while(0);

#define _PREV_CHECK(TREE, NODE, CR, ERR_RET)        do{ \
                                                    _PREV_CHECK_TREE(TREE, CR, ERR_RET); \
                                                    _checkHashTreeNode(TREE, NODE); \
                                                }while(0);


static HashTreeNode_S *_defaultHashTreeNodeAlloc()
{
    HashTreeNode_S *treeNode = calloc(1, sizeof(HashTreeNode_S));
    return treeNode;
}

static void _defaultHashTreeNodeInit(struct HashTree *tree, HashTreeNode_S *node)
{
    
}


static void _defaultHashTreeNodeDealloc(struct HashTree *tree, HashTreeNode_S *node)
{
    if (node) {
        if (node->key.dealloc) {
            node->key.dealloc(&node->key);
        }
        if (node->value.dealloc) {
            node->value.dealloc(&node->value);
        }
    }
}

static void _defaultHashTreeNodeFree(struct HashTree *tree, HashTreeNode_S *node)
{
    if (node) {
        tree->dealloc(tree, node);
        free(node);
    }
}

static uint8_t _defaultHashTreeNodeChildCountFunc(struct HashTree *tree, uint8_t level)
{
    return hashTreePrimeList_s[level];
}

static int8_t _defaultHashTreeNodeChildIndexFunc(struct HashTree *tree, struct HashTreeNode *currentNode, HashTreeNode_S *insertNode)
{
    uint8_t prime = hashTreePrimeList_s[currentNode->level];
    uint8_t index = insertNode->hashValue % prime;
    return index;
}

static YZHComparisonResult_E _defaultHashTreeNodeCompareFunc(struct HashTree *tree, struct HashTreeNode *first, struct HashTreeNode *second)
{
    return compare(&first->key, &second->key);
}

static struct HashTreeNode * _createHashTreeNode(struct HashTree *tree, uint8_t level)
{
    HashTreeNode_S *node = tree->alloc();
    if (node == NULL) {
        return NULL;
    }
    node->level = level;
    node->childNodeCount = tree->childCountFunc(tree, level);
    node->ptrChildNodeList = calloc(node->childNodeCount, sizeof(PHashTreeNode_S));
    if (node->ptrChildNodeList == NULL) {
        return NULL;
    }
    return node;
}

static int8_t _setupHashTreeNodeLevel(struct HashTree *tree, struct HashTreeNode *node, uint8_t level)
{
    uint8_t childCount = tree->childCountFunc(tree, level);
    if (node->childNodeCount != childCount) {
        node->level = level;
        node->childNodeCount = childCount;
        if (node->ptrChildNodeList) {
            free(node->ptrChildNodeList);
        }
        node->ptrChildNodeList = calloc(node->childNodeCount, sizeof(PHashTreeNode_S));
        if (node->ptrChildNodeList == NULL) {
            return 0;
        }
    }
    return 1;
}

QueueNode_S *_queueNodeWithHashTreeNode(struct HashTreeNode *node)
{
    QueueNode_S *qnode = calloc(1,sizeof(QueueNode_S));
    if (qnode) {
        qnode->value = node;
    }
    return qnode;
}

static void _ZEnumerateHashTree(struct HashTree *tree, struct HashTreeNode *node, HashTreeNodeEnumerateFunc enumerator)
{
    if (!node || enumerator == NULL) {
        return;
    }
    Queue_S *queue = calloc(1, sizeof(Queue_S));
    Queue_S *queueTmp = calloc(1, sizeof(Queue_S));
    //申请一个缓存
    Queue_S *cache = calloc(1, sizeof(Queue_S));
    
    if (queue == NULL || queueTmp == NULL) {
        goto ZEnumerateHashTree_END;
    }
    
    push(queue, _queueNodeWithHashTreeNode(node));
    while (queue->count > 0 ) {
        QueueNode_S *qnode = pop(queue);
        HashTreeNode_S *treeNode = qnode->value;
        
        //首先保存子节点
        for (uint8_t i = 0; i < treeNode->childNodeCount; ++i) {
            HashTreeNode_S *child = treeNode->ptrChildNodeList[i];
            if (child) {
                QueueNode_S *qnodeTmp = pop(cache);
                if (!qnodeTmp) {
                    qnodeTmp = _queueNodeWithHashTreeNode(child);
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
    
    
ZEnumerateHashTree_END:
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

static int8_t _checkInitHashTree(struct HashTree *tree, int8_t createRoot) {
//    if (tree->root) {
//        return 1;
//    }
    if (!tree->alloc) {
        tree->alloc = _defaultHashTreeNodeAlloc;
    }
    if (!tree->init) {
        tree->init = _defaultHashTreeNodeInit;
    }
    if (!tree->dealloc) {
        tree->dealloc = _defaultHashTreeNodeDealloc;
    }
    if (!tree->free) {
        tree->free = _defaultHashTreeNodeFree;
    }
    if (!tree->hashFunc) {
        tree->hashFunc = BKDRHash;
    }
    if (!tree->compareFunc) {
        tree->compareFunc = _defaultHashTreeNodeCompareFunc;
    }
    if (!tree->childCountFunc) {
        tree->childCountFunc = _defaultHashTreeNodeChildCountFunc;
    }
    if (!tree->childIndexFunc) {
        tree->childIndexFunc = _defaultHashTreeNodeChildIndexFunc;
    }
    if (createRoot && tree->root == NULL) {
        tree->root = _createHashTreeNode(tree, 0);
    }
    return 1;
}

static int8_t _checkHashTreeNode(struct HashTree *tree, struct HashTreeNode *node) {
    if (node->hashValue == 0) {
        node->hashValue = tree->hashFunc(&node->key);
    }
    return 1;
}


//返回0表示失败，大于0表示成功
int8_t _insertHashTreeWithNode(struct HashTree *tree, struct HashTreeNode *node)
{
    _PREV_CHECK(tree, node, 1, 0);
    
    int8_t ret = 1;
    struct HashTreeNode *current = tree->root;
    while (current) {
        int8_t childIdx = tree->childIndexFunc(tree, current, node);
        YZHComparisonResult_E comp = (node->hashValue == current->hashValue) ? YZHOrderedEQ : !YZHOrderedEQ;
        if (comp == YZHOrderedEQ) {
            if (current->ptrKeySameLastNode) {
                current->keySameNodeCount += 1;
                struct HashTreeNode *last = current->ptrKeySameLastNode;
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
                _setupHashTreeNodeLevel(tree, node, current->level + 1);
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

struct HashTree *allocHashTree(void)
{
    struct HashTree *tree = calloc(1, sizeof(struct HashTree));
    return tree;
}

HashTreeNode_S *insertHashTree(struct HashTree *tree, T *key, T *val){
    _PREV_CHECK_TREE(tree, 1, NULL);

    HashTreeNode_S *treeNode = tree->alloc();
    treeNode->key = *key;
    treeNode->value = *val;
        
    _insertHashTreeWithNode(tree, treeNode);
    
    return treeNode;
}

HashTreeNode_S *deleteHashTree(struct HashTree *tree, T *key)
{
    _PREV_CHECK_TREE(tree, 1, NULL);

    struct HashTreeNode *findNode = selectHashTree(tree, key);
    if (findNode != NULL && findNode->state != HashTreeNodeStateDelete) {
        findNode->state = HashTreeNodeStateDelete;
        tree->dealloc(tree, findNode);
    }
    return findNode;
}

HashTreeNode_S *selectHashTree(struct HashTree *tree, T *key)
{
    _PREV_CHECK_TREE(tree, 1, NULL);

    struct HashTreeNode node = {.key = *key, .hashValue = 0};
    _checkHashTreeNode(tree, &node);

    struct HashTreeNode *current = tree->root;
    
    while (current) {
        int8_t childIdx = tree->childIndexFunc(tree, current, &node);
        YZHComparisonResult_E comp = node.hashValue == current->hashValue ? YZHOrderedEQ : !YZHOrderedEQ;;
        if (comp == YZHOrderedEQ) {
            if (current->ptrKeySameLastNode) {
                do {
                    if (current->state != HashTreeNodeStateDelete) {
                        YZHComparisonResult_E r = tree->compareFunc(tree, current, &node);
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
    if (current && current->state != HashTreeNodeStateDelete) {
        return current;
    }
    return NULL;
}

void enumerateHashTree(struct HashTree *tree, HashTreeNodeEnumerateFunc enumerator)
{
    _PREV_CHECK_TREE(tree, 0, );

    if (enumerator == NULL) {
        enumerator = tree->enumerator;
    }
    _ZEnumerateHashTree(tree, tree->root, enumerator);
}

void _HashTreeClearEnumerate(struct HashTree *tree, HashTreeNode_S *node)
{
    if (node) {
        memset(node->ptrChildNodeList, 0 , node->childNodeCount * sizeof(struct HashTreeNode *));
        node->ptrKeySameNextNode = NULL;
        node->ptrKeySamePrevNode = NULL;
        node->ptrKeySameLastNode = NULL;
    }
    if (tree->free) {
        tree->free(tree, node);
    }
}

void clearHashTree(struct HashTree *tree)
{
    _PREV_CHECK_TREE(tree, 0, );
    
    enumerateHashTree(tree, _HashTreeClearEnumerate);
}

//先清空，再free
void freeHashTree(struct HashTree *tree)
{
    _PREV_CHECK_TREE(tree, 0, );
    clearHashTree(tree);
    if (tree) {
        memset(tree, 0, sizeof(struct HashTree));
        free(tree);
    }
}

void printTreeNode(struct HashTreeNode *node)
{
    printf("levele:%d, key=%lld,value=%lld, hashValue=%lld \n",node->level,node->key.V.val,node->value.V.val, node->hashValue);
}
