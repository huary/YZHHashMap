//
//  YZHHashMap.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/7.
//  Copyright © 2019年 yuan. All rights reserved.
//

#include <stdlib.h>
#include <memory.h>
#include "YZHHashMap.h"
#include "macro.h"
#include "YZHHash.h"

static int32_t hashTableCapcity_s[] = {8, 32, 128, 8192, 131072, 524288};

static int32_t hashTableCapcityIncrement_s = 32;

static float hashAdjustCapcityFactory_s = 0.75;
//2^n + 1
//17,31,257
//static int32_t hashSeed[] = {4, 8};
//static int32_t hashSeed_s = 4;

//static uint64_t defaultHashFunc(T *key)
//{
//    if (key == NULL) {
//        return 0;
//    }
//    uint64_t hash = 0;
//    if (key->size == 0) {
//        uint64_t val = key->V.val;
//        return val;
////        while (val > 0) {
////            hash = TYPE_LS(hash, hashSeed_s) + hash + TYPE_AND(val, NUM_8_POWOFTWO_MASK);
////            val = TYPE_RS(val, 8);
////        }
//    }
//    else {
//        uint8_t *ptr = key->V.ptr;
//        while (*ptr) {
//            hash = TYPE_LS(hash, hashSeed_s) + hash + *ptr;
//            ++ptr;
//        }
//    }
//    return hash;
//}

static int8_t defaultShouldAdjustFunc(struct YZHHashMap *hashMap)
{
    if (hashMap == NULL || hashMap->count <= 0) {
        return 0;
    }
    float r = hashMap->count * 1.0/hashMap->capacity;
    return (r - hashAdjustCapcityFactory_s) > 0.001 ? 1 : 0;
}

static int32_t hashTableIndex(uint64_t hashValue, int32_t capacity)
{
    int32_t hashIdx = 0;
    if (capacity == 0) {
        return 0;
    }
    
    if (TYPE_POS_IS_POWOFTWO(capacity)) {
        hashIdx = TYPE_AND(hashValue, capacity-1);
    }
    else {
        hashIdx = hashValue % capacity;
    }
    return hashIdx;
}

//RBTree的
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

static RBTreeNodeComparisonResult_E RBTreeNodeCompare(struct RBTree *tree, RBTreeNode_S *first, RBTreeNode_S *second)
{
    return compare(&first->key, &second->key);
}

static void RBTreeNodeCopy(struct RBTree *tree, RBTreeNode_S *src, RBTreeNode_S *dst)
{
    dst->key = src->key;
    dst->value = src->value;
    dst->userInfo = src->userInfo;
}

static void RBTreeNodeSwap(struct RBTree *tree, RBTreeNode_S *first, RBTreeNode_S *second)
{
//    INTEGER_SWAP(first->key, second->key);
    swap(&first->key, &second->key);
    PTR_SWAP(first->value, second->value);
    PTR_SWAP(first->userInfo, second->userInfo);
}

static void RBTreeEnumerator(struct RBTree *tree, RBTreeNode_S *treeNode, int32_t level)
{
    if (tree == NULL || tree->userInfo == NULL || treeNode == NULL) {
        return;
    }
    YZHMapEntry_S *mapEntry = tree->userInfo;
    YZHHashMapState_E *info = mapEntry->enumerateInfo;
    YZHHashMapState_E state = (YZHHashMapState_E)(*info);
    
    if (state == YZHHashMapStateNormal) {
        YZHMapNode_S *value = treeNode->value;
        static int32_t lv = -1;
        if (lv != level) {
            printf("\n%d====:%lld(%s) ",level, value->key.V.val, treeNode->color ? "B" : "R");
        }
        else {
            printf("%lld(%s) ", value->key.V.val, treeNode->color ? "B" : "R");
        }
        lv = level;
    }
    else if (state == YZHHashMapStateAdjust) {
        if (mapEntry->enumerateCallback) {
            //在这里把这个节点从原来的tree中迁移出来
            treeNode->parent = NULL;
            treeNode->left = NULL;
            treeNode->right = NULL;
            YZHMapNode_S *mapNode = treeNode->value;
            int8_t r = mapEntry->enumerateCallback(mapEntry, mapNode, treeNode);
            if (r > 0) {
                if (treeNode == tree->root) {
                    tree->root = NULL;
                }
                --tree->count;
            }
        }
    }
    else if (state == YZHHashMapStateClearMapEntry || state == YZHHashMapStateClear || state == YZHHashMapStateFree) {
        if (mapEntry->enumerateCallback) {
            YZHMapNode_S *mapNode = treeNode->value;
            mapEntry->enumerateCallback(mapEntry, mapNode, treeNode);
        }
        if (tree->free) {
            tree->free(tree, treeNode);
        }
    }
    
}


static inline struct RBTreeNode *selectTreeNodeFromMapEntry(struct YZHMapEntry *mapEntry, T *key)
{
    if (mapEntry->type != YZHMapEntryTypeRBTree || mapEntry->entry.tree == NULL || key == NULL) {
        return NULL;
    }
    RBTree_S *tree = mapEntry->entry.tree;
    struct YZHMapNode mapNode = {.key = *key};
    struct RBTreeNode conditionNode = {.key = *key, .value = &mapNode};
    struct RBTreeNode *treeNode = selectRBTree(tree, &conditionNode);
    return treeNode;
}


//YZHMapEntry
static struct YZHMapNode* defaultMapEntryNodeAlloc(struct YZHMapEntry *mapEntry)
{
    struct YZHMapNode *node = calloc(1, sizeof(YZHMapNode_S));
    return node;
}

static void defaultMapEntryNodeFree(struct YZHMapEntry *mapEntry, struct YZHMapNode *node)
{
    if (node) {
        free(node);
    }
}

static int8_t defaultMapEntryInsert(struct YZHMapEntry *mapEntry, struct YZHMapNode *node, void *reusedNode)
{
    if (mapEntry->type != YZHMapEntryTypeRBTree || mapEntry->entry.tree == NULL) {
        return 0;
    }
    RBTree_S *tree = mapEntry->entry.tree;
    
    struct RBTreeNode *treeNode = reusedNode;
    if (treeNode == NULL) {
        treeNode = tree->alloc();
        if (!treeNode) {
            return 0;
        }
    }
    treeNode->value = node;
    treeNode->key = node->key;
    return insertRBTree(tree, treeNode);
}

static struct YZHMapNode *defaultMapEntryDelete(struct YZHMapEntry *mapEntry, T *key)
{
    struct RBTreeNode *treeNode = selectTreeNodeFromMapEntry(mapEntry, key);
    struct YZHMapNode *findMapNode = NULL;
    if (treeNode) {
        findMapNode = treeNode->value;
        deleteRBTree(mapEntry->entry.tree, treeNode);
    }
    return findMapNode;
}

static inline struct YZHMapNode *defaultMapEntrySelect(struct YZHMapEntry *mapEntry, T *key)
{
    struct RBTreeNode *treeNode = selectTreeNodeFromMapEntry(mapEntry, key);
    if (treeNode) {
        return (struct YZHMapNode *)treeNode->value;
    }
    return NULL;
}

static void defaultMapEntryEnumerator(struct YZHMapEntry *mapEntry, MapEntryEnumerateCallbackFunc callback, void *enumerateInfo)
{
    if (mapEntry->type != YZHMapEntryTypeRBTree || mapEntry->entry.tree == NULL) {
        return;
    }
    RBTree_S *tree = mapEntry->entry.tree;
    tree->userInfo = mapEntry;
    mapEntry->enumerateCallback = callback;
    mapEntry->enumerateInfo = enumerateInfo;
    enumerateRBTree(tree, BTreeEnumerateTypeZ, NULL);
}

//调整mapnode时，需要调用
static int8_t defaultMapEntryAdjustMapNode(struct YZHMapEntry *mapEntry, struct YZHMapNode *node, void *reusedNode)
{
    YZHHashNode_S *hashNodeOld = mapEntry->hashNode;
    YZHHashMap_S *hashMap = hashNodeOld->hashMap;
    if (hashMap == NULL || hashMap->adjustCtx == NULL || hashMap->adjustCtx->adjustTable == NULL || hashMap->state != YZHHashMapStateAdjust) {
        return 0;
    }
    
    YZHHashMapAdjustCTX_S *adjustCtx = hashMap->adjustCtx;
    uint32_t idx = hashTableIndex(node->hashValue, adjustCtx->adjustCapacity);
    struct YZHHashNode *hashNode = &(adjustCtx->adjustTable[idx]);

    hashNode->hashMap = hashMap;

    struct YZHMapEntry *mapEntryNew = hashNode->mapEntry;
    if (mapEntryNew == NULL) {
        mapEntryNew = hashMap->mapEntryConstructor(hashMap);
        if (mapEntryNew == NULL) {
            return 0;
        }
        hashNode->mapEntry = mapEntryNew;
    }
    mapEntryNew->hashNode = hashNode;

    mapEntryNew->insertFunc(mapEntryNew, node, reusedNode);
    ++adjustCtx->count;
    return 1;
}

//释放mapNode时需要,这个不用对红黑树进行调整的释放
static int8_t defaultMapEntryFreeMapNode(struct YZHMapEntry *mapEntry, struct YZHMapNode *node, void *reusedNode)
{
    YZHHashNode_S *hashNodeOld = mapEntry->hashNode;
    YZHHashMap_S *hashMap = hashNodeOld->hashMap;
    if (hashMap == NULL || hashMap->state != YZHHashMapStateClearMapEntry) {
        return 0;
    }
    if (mapEntry->free) {
        mapEntry->free(mapEntry, node);
    }
    return 1;
}

static struct YZHMapEntry *defaultMapEntryConstructor(struct YZHHashMap *hashMap)
{
    struct YZHMapEntry *mapEntry = calloc(1, sizeof(struct YZHMapEntry));
    if (!mapEntry) {
        goto DEFAULT_CALLOC_ERR_END;
    }
    
    mapEntry->type = YZHMapEntryTypeRBTree;
    if (mapEntry->type == YZHMapEntryTypeRBTree) {
        mapEntry->entry.tree = calloc(1, sizeof(RBTree_S));
        if (mapEntry->entry.tree == NULL) {
            goto DEFAULT_CALLOC_ERR_END;
        }
        mapEntry->entry.tree->userInfo = mapEntry;
        mapEntry->entry.tree->alloc = defaultRBTreeNodeAlloc;
        mapEntry->entry.tree->free = defaultRBTreeNodeFree;
        mapEntry->entry.tree->compare = RBTreeNodeCompare;
        mapEntry->entry.tree->copy = RBTreeNodeCopy;
        mapEntry->entry.tree->swap = RBTreeNodeSwap;
        mapEntry->entry.tree->enumerator = RBTreeEnumerator;
    }
    
    mapEntry->alloc = defaultMapEntryNodeAlloc;
    mapEntry->free = defaultMapEntryNodeFree;
    mapEntry->insertFunc = defaultMapEntryInsert;
    mapEntry->deleteFunc = defaultMapEntryDelete;
    mapEntry->selectFunc = defaultMapEntrySelect;
    mapEntry->enumerator = defaultMapEntryEnumerator;
    
    return mapEntry;
DEFAULT_CALLOC_ERR_END:
    if (mapEntry) {
        free(mapEntry);
    }
    if (mapEntry->entry.tree) {
        free(mapEntry->entry.tree);
        mapEntry->entry.tree = NULL;
    }
    return mapEntry;
}

static void defaultMapEntryDestructor(struct YZHHashMap *hashMap, struct YZHMapEntry *mapEntry)
{
    if (!mapEntry) {
        return;
    }
    
    //遍历MapEntry上的节点
    YZHHashMapState_E old = hashMap->state;
    hashMap->state = YZHHashMapStateClearMapEntry;
    if (mapEntry->enumerator) {
        mapEntry->enumerator(mapEntry, defaultMapEntryFreeMapNode, &hashMap->state);
    }
    
    if (mapEntry->type == YZHMapEntryTypeRBTree) {
        if (mapEntry->entry.tree) {
            mapEntry->entry.tree->alloc = NULL;
            mapEntry->entry.tree->free = NULL;
            mapEntry->entry.tree->compare = NULL;
            mapEntry->entry.tree->copy = NULL;
            mapEntry->entry.tree->swap = NULL;
            mapEntry->entry.tree->enumerator = NULL;
            
            free(mapEntry->entry.tree);
            mapEntry->entry.tree = NULL;
        }
    }
    mapEntry->alloc = NULL;
    mapEntry->free = NULL;
    
    mapEntry->insertFunc = NULL;
    mapEntry->deleteFunc = NULL;
    mapEntry->selectFunc = NULL;
    mapEntry->enumerator = NULL;
    if (mapEntry) {
        free(mapEntry);
        mapEntry = NULL;
    }
    hashMap->state = old;
}

static int32_t getNextCapcityWithOld(int32_t oldCapcity)
{
    int32_t capcity = oldCapcity;
    int32_t cnt = sizeof(hashTableCapcity_s)/sizeof(hashTableCapcity_s[0]);
    for (int32_t i = 0; i < cnt; ++i) {
        if (oldCapcity < hashTableCapcity_s[i]) {
            capcity = hashTableCapcity_s[i];
            break;
        }
    }
    if (capcity == oldCapcity) {
        capcity = oldCapcity + hashTableCapcityIncrement_s;
    }
    return capcity;
}

static void checkHashMap(struct YZHHashMap *hashMap) {
    
    if (!hashMap->hashFunc) {
        hashMap->hashFunc = BKDRHash;
    }
    if (!hashMap->shouldAdjustFunc) {
        hashMap->shouldAdjustFunc = defaultShouldAdjustFunc;
    }
    if (!hashMap->mapEntryConstructor) {
        hashMap->mapEntryConstructor = defaultMapEntryConstructor;
    }
    if (!hashMap->mapEntryDestructor) {
        hashMap->mapEntryDestructor = defaultMapEntryDestructor;
    }
    
    if (hashMap->capacity == 0) {
        hashMap->capacity = hashTableCapcity_s[0];
        if (hashMap->hashTable) {
            free(hashMap->hashTable);
            hashMap->hashTable = NULL;
        }
        if (hashMap->hashTable == NULL) {
            hashMap->hashTable = calloc(hashMap->capacity, sizeof(struct YZHHashNode));
        }
        
        hashMap->count = 0;
        return;
    }
    
    int8_t should = hashMap->shouldAdjustFunc(hashMap);
    if (should) {
        //扩容
        YZHHashMapAdjustCTX_S ctx;
        ctx.count = 0;
        ctx.adjustCapacity = getNextCapcityWithOld(hashMap->capacity);
        ctx.adjustTable = calloc(ctx.adjustCapacity, sizeof(struct YZHHashNode));
        if (ctx.adjustTable == NULL) {
            return;
        }
        ctx.adjustFunc = defaultMapEntryAdjustMapNode;
        //给adjustCtx复制，表示在进行调整
        hashMap->adjustCtx = &ctx;
        YZHHashMapState_E old = hashMap->state;
        hashMap->state = YZHHashMapStateAdjust;
        for (int32_t i = 0; i < hashMap->capacity; ++i) {
            struct YZHHashNode *hashNode = &(hashMap->hashTable[i]);
            struct YZHMapEntry *mapEntry = hashNode->mapEntry;
            if (mapEntry && mapEntry->enumerator) {
                mapEntry->enumerator(mapEntry, ctx.adjustFunc, &hashMap->state);
            }
            if (hashMap->mapEntryDestructor && mapEntry) {
                hashMap->mapEntryDestructor(hashMap, mapEntry);
            }
            hashNode->mapEntry = NULL;
        }
        hashMap->count = ctx.count;
        hashMap->capacity = ctx.adjustCapacity;
        if (hashMap->hashTable) {
            free(hashMap->hashTable);
        }
        hashMap->hashTable = ctx.adjustTable;
        //扩容完成，将转态还原回来为Normal
        hashMap->state = old;
        hashMap->adjustCtx = NULL;
    }
}

static struct YZHMapNode *newMapNode(struct YZHMapEntry *mapEntry, T *key, T *val, uint64_t hashValue)
{
    if (mapEntry == NULL || mapEntry->alloc == NULL) {
        return NULL;
    }
    struct YZHMapNode *node = mapEntry->alloc(mapEntry);
    if (node) {
        node->key = *key;
        node->val = *val;
        node->hashValue = hashValue;
    }
    return node;
    
}

static YZHHashNode_S *hashNodeFor(struct YZHHashMap *hashMap, T *key)
{
    uint64_t hashValue = hashMap->hashFunc(key);
    uint32_t idx = hashTableIndex(hashValue, hashMap->capacity);
    struct YZHHashNode *hashNode = &(hashMap->hashTable[idx]);
    return hashNode;
}

struct YZHHashMap *allocHashMapWithCapacity(int32_t capacity)
{
    if (capacity <= 0) {
        return NULL;
    }
    struct YZHHashMap *hashMap = calloc(1, sizeof(YZHHashMap_S));
    if (hashMap == NULL) {
        goto ALLOC_HASHMAP_ERR_END;
    }
    hashMap->hashTable = calloc(capacity, sizeof(struct YZHHashNode));
    if (hashMap->hashTable == NULL) {
        goto ALLOC_HASHMAP_ERR_END;
    }
    hashMap->count = 0;
    hashMap->capacity = capacity;
    hashMap->state = YZHHashMapStateNormal;
    checkHashMap(hashMap);
    return hashMap;
ALLOC_HASHMAP_ERR_END:
    if (hashMap->hashTable) {
        free(hashMap->hashTable);
        hashMap->hashTable = NULL;
    }
    if (hashMap) {
        free(hashMap);
    }
    return NULL;
}

YZHMapNode_S *insertHashMap(struct YZHHashMap *hashMap, T *key, T *val)
{
    if (hashMap == NULL) {
        return NULL;
    }
    
    checkHashMap(hashMap);
    
    uint64_t hashValue = hashMap->hashFunc(key);
    uint32_t idx = hashTableIndex(hashValue, hashMap->capacity);
    struct YZHHashNode *hashNode = &(hashMap->hashTable[idx]);
    hashNode->hashMap = hashMap;
    
    struct YZHMapEntry *mapEntry = hashNode->mapEntry;
    if (mapEntry == NULL) {
        mapEntry = hashMap->mapEntryConstructor(hashMap);
        if (mapEntry->type == YZHMapEntryTypeRBTree) {
            mapEntry->entry.tree->userInfo = mapEntry;
        }
        else {
            if (mapEntry->entry.ctner) {
                mapEntry->entry.ctner->userInfo = mapEntry;
            }
        }
        hashNode->mapEntry = mapEntry;
    }
    mapEntry->hashNode = hashNode;
    
    struct YZHMapNode *mapNode = newMapNode(mapEntry, key, val, hashValue);
    mapEntry->insertFunc(mapEntry, mapNode, NULL);
    ++hashMap->count;
    return mapNode;
}

T * deleteHashMap(struct YZHHashMap *hashMap, T *key)
{
    if (hashMap == NULL) {
        return NULL;
    }
    struct YZHHashNode *hashNode = hashNodeFor(hashMap, key);
    if (hashNode && hashNode->mapEntry && hashNode->mapEntry->deleteFunc) {
        //释放红黑树的node
        YZHMapNode_S *mapNode = hashNode->mapEntry->deleteFunc(hashNode->mapEntry, key);
        T *val = &mapNode->val;
        if (mapNode && hashNode->mapEntry->free) {
            hashNode->mapEntry->free(hashNode->mapEntry, mapNode);
        }
        return val;
    }
    return NULL;
}

T * selectHashMap(struct YZHHashMap *hashMap, T *key)
{
    if (hashMap == NULL) {
        return NULL;
    }
    struct YZHHashNode *hashNode = hashNodeFor(hashMap, key);
    if (LIKELY(hashNode->mapEntry)) {
        YZHMapNode_S *mapNode = hashNode->mapEntry->selectFunc(hashNode->mapEntry, key);
        return &mapNode->val;
    }
    return NULL;
}

void clearHashMap(struct YZHHashMap *hashMap)
{
    if (hashMap == NULL) {
        return;
    }
    for (int32_t i = 0; i < hashMap->capacity; ++i) {
        struct YZHHashNode *hashNode = &(hashMap->hashTable[i]);
        struct YZHMapEntry *mapEntry = hashNode->mapEntry;
        if (hashMap->mapEntryDestructor) {
            hashMap->mapEntryDestructor(hashMap, mapEntry);
        }
        hashNode->mapEntry = NULL;
    }
    hashMap->count = 0;
    hashMap->state = YZHHashMapStateClear;
}

void freeHashMap(struct YZHHashMap *hashMap)
{
    if (hashMap == NULL) {
        return;
    }
    clearHashMap(hashMap);
    
    if (hashMap->hashTable) {
        free(hashMap->hashTable);
        hashMap->hashTable = NULL;
    }
    hashMap->count = 0;
    hashMap->capacity = 0;
    hashMap->state = YZHHashMapStateFree;
    if (hashMap) {
        free(hashMap);
    }
}

void print(struct YZHHashMap *hashMap)
{
    int32_t tt = 0;
    for (int32_t i = 0; i < hashMap->capacity; ++i) {
        struct YZHHashNode *hashNode = &(hashMap->hashTable[i]);
        struct YZHMapEntry *mapEntry = hashNode->mapEntry;
        if (mapEntry) {
            tt += mapEntry->entry.tree->count;
            mapEntry->enumerator(mapEntry, NULL, &hashMap->state);
        }
    }
}

