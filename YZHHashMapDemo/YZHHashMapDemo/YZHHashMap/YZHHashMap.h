//
//  YZHHashMap.h
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/7.
//  Copyright © 2019年 yuan. All rights reserved.
//

#ifndef YZHHashMap_h
#define YZHHashMap_h

#include <stdio.h>
#include "YZHType.h"
#include "RBTree.h"

typedef enum YZHMapEntryType
{
    YZHMapEntryTypeRBTree   = 0,
}YZHMapEntryType_E;

/****************************************************
 *YZHHashMapState
 ****************************************************/
typedef enum YZHHashMapState{
    YZHHashMapStateNormal           = 0,
    //在调整
    YZHHashMapStateAdjust           = 1,
    //在释放MapEntry挂载的点YZHMapNode
    YZHHashMapStateClearMapEntry    = 2,
    YZHHashMapStateClear            = 3,
    YZHHashMapStateFree             = 4,
}YZHHashMapState_E;

/****************************************************
 *YZHMapNode
 ****************************************************/
typedef struct YZHMapNode {
    T key;
    T val;
    uint64_t hashValue;
}YZHMapNode_S,*PYZHMapNode_S;

/****************************************************
 *YZHMapEntry
 ****************************************************/
//前向声明
struct YZHMapEntry;
typedef struct YZHMapNode *(*MapEntryNodeAllocFunc)(struct YZHMapEntry *mapEntry);
typedef void (*MapEntryNodeFreeFunc)(struct YZHMapEntry *mapEntry, struct YZHMapNode *node);

//返回0表示失败，大于0表示成功
typedef int8_t (*MapEntryInsertFunc)(struct YZHMapEntry *mapEntry, struct YZHMapNode *node, void *reusedNode);
typedef struct YZHMapNode *(*MapEntryDeleteFunc)(struct YZHMapEntry *mapEntry, T *key);
typedef struct YZHMapNode *(*MapEntrySelectFunc)(struct YZHMapEntry *mapEntry, T *key);

typedef int8_t (*MapEntryEnumerateCallbackFunc)(struct YZHMapEntry *mapEntry, struct YZHMapNode *node, void *reusedNode);
typedef void (*MapEntryEnumerateFunc)(struct YZHMapEntry *mapEntry, MapEntryEnumerateCallbackFunc callback, void *enumerateInfo);
typedef int8_t (*MapEntryAdjustFunc)(struct YZHMapEntry *mapEntry, struct YZHMapNode *node, void *reusedNode);

//前向声明
struct YZHHashNode;
typedef struct YZHMapEntry
{
    YZHMapEntryType_E type;
    union {
        RBTree_S *tree;
        struct container {
            void *list;
            struct YZHMapEntry *userInfo;
        } *ctner;
    }entry;
    //node的创建和释放
    MapEntryNodeAllocFunc alloc;
    MapEntryNodeFreeFunc free;
    
    MapEntryInsertFunc insertFunc;
    MapEntryDeleteFunc deleteFunc;
    MapEntrySelectFunc selectFunc;
    MapEntryEnumerateFunc enumerator;
    //遍历mapEnter时回调回来函数
    MapEntryEnumerateCallbackFunc enumerateCallback;
    /*
     *遍历mapEnter时，给entry的参数,entry中container或者tree可以根据enumerateInfo来判断
     *一些不同形式的遍历，现在enumerateInfo指向的是YZHHashMapState转态
     */
    void *enumerateInfo;
    
    struct YZHHashNode *hashNode;
    void *userInfo;
}YZHMapEntry_S;

/****************************************************
 *YZHHashNode
 ****************************************************/
//前向声明
struct YZHHashMap;
typedef struct YZHHashNode
{
    struct YZHMapEntry *mapEntry;
    struct YZHHashMap *hashMap;
    void *userInfo;
}YZHHashNode_S;


/****************************************************
 *YZHHashMap
 ****************************************************/
typedef uint64_t (*HashMapHashFunc)(T *key);
//前向声明
struct YZHHashMap;
struct YZHHashMapAdjustCTX;

typedef int8_t (*HashMapShouldAdjustFunc)(struct YZHHashMap *hashMap);
typedef struct YZHMapEntry*(*MapEntryConstructorFunc)(struct YZHHashMap *hashMap);
typedef void (*MapEntryDestructorFunc)(struct YZHHashMap *hashMap, struct YZHMapEntry *mapEntry);

typedef struct YZHHashMap
{
    int32_t count;
    int32_t capacity;
    YZHHashMapState_E state;
    struct YZHHashNode *hashTable;
    struct YZHHashMapAdjustCTX *adjustCtx;
    
    HashMapHashFunc hashFunc;
    HashMapShouldAdjustFunc shouldAdjustFunc;
    MapEntryConstructorFunc mapEntryConstructor;
    MapEntryDestructorFunc mapEntryDestructor;
    
    void *userInfo;
}YZHHashMap_S;

/****************************************************
 *YZHHashMapAdjustContext
 ****************************************************/
typedef struct YZHHashMapAdjustCTX
{
    int32_t count;
    int32_t adjustCapacity;
    struct YZHHashNode *adjustTable;
     MapEntryAdjustFunc adjustFunc;
    void *userInfo;
}YZHHashMapAdjustCTX_S;

struct YZHHashMap *allocHashMapWithCapacity(int32_t capcity);
YZHMapNode_S *insertHashMap(struct YZHHashMap *hashMap, T *key, T *val);
T * deleteHashMap(struct YZHHashMap *hashMap, T *key);
T * selectHashMap(struct YZHHashMap *hashMap, T *key);
//只是清空，没有free
void clearHashMap(struct YZHHashMap *hashMap);
//有清空，再free
void freeHashMap(struct YZHHashMap *hashMap);

void print(struct YZHHashMap *hashMap);

#endif /* YZHHashMap_h */
