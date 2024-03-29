//
//  ViewController.m
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/1.
//  Copyright © 2019年 yuan. All rights reserved.
//

#import "ViewController.h"
#import "RBTree.h"
#include <mach/mach_time.h>
#include "YZHType.h"
#include "YZHHashMap.h"
#include "macro.h"


//#define INT_SWAP(X,Y)       ((X)==(Y) ?:((X)=(X)^(Y),(Y)=(X)^(Y),(X)=(X)^(Y)))
//
//#define _PTR_VAL(PTR)                (*(&(PTR)))
//#define _PTR_VAL_L(PTR)              _PTR_VAL(PTR)
//#define _PTR_VAL_R(PTR)             ((intptr_t)_PTR_VAL(PTR))
//#define PTR_XOR(PTR_X,PTR_Y)        (_PTR_VAL_R(PTR_X) ^ _PTR_VAL_R(PTR_Y))
//#define PTR_SWAP(X,Y)               ((X)==(Y) ?:(_PTR_VAL_L(X)= PTR_XOR(X,Y),_PTR_VAL_L(Y)= PTR_XOR(X,Y),_PTR_VAL_L(X)=PTR_XOR(X,Y)))


@interface ViewController ()

/* <#注释#> */
@property (nonatomic, strong) NSMutableArray<NSNumber*> *list;

/* <#注释#> */
@property (nonatomic, strong) NSMutableDictionary<NSNumber*,NSNumber*> *info;

@end

@implementation ViewController

static RBTreeNodeComparisonResult_E compareValue(struct RBTree *tree ,RBTreeNode_S *first, RBTreeNode_S *second)
{
    return compare(&first->key, &second->key);
//    if (first->key < second->key) {
//        return YZHOrderedASC;//ASC;
//    }
//    else if (first->key == second->key) {
//        return YZHOrderedEQ;//SAME;
//    }
//    else {
//        return YZHOrderedDES;//DESC;
//    }
}

static void copyValue(struct RBTree *tree, RBTreeNode_S *src, RBTreeNode_S *dst)
{
    dst->key = src->key;
}

static void swapValue(struct RBTree *tree, RBTreeNode_S *first, RBTreeNode_S *second)
{
    swap(first, second);
//    NSLog(@"1、frist=%@,second=%@",@(first->key),@(second->key));
//    INTEGER_SWAP(first->key, second->key);
//    NSLog(@"2、frist=%@,second=%@",@(first->key),@(second->key));

}

static void release(struct RBTree *tree, RBTreeNode_S *node)
{
//    if (node == NULL) {
//        free(node);
//        node = NULL;
//    }
}

static void enumerator(struct RBTree *tree, RBTreeNode_S *node, int32_t level)
{
    static int32_t lv = -1;
    if (lv != level) {
        printf("\n%d====:%lld(%s) ",level,node->key,node->color ? "B" : "R");
    }
    else {
        printf("%lld(%s) ",node->key,node->color ? "B" : "R");
    }
    lv = level;
}



- (void)viewDidLoad {
    [super viewDidLoad];
    
//    [self _test];
    
//    [self _test2];
    
//    [self _test3];
    
}

-(void)_test3
{
    int a = 3;
    int *pa = &a;
    int b = 5;
    int *pb = &b;
    NSLog(@"1.pa=%p,pb=%p",pa,pb);
    PTR_SWAP(pa, pb);
    NSLog(@"2.pa=%p,pb=%p",pa,pb);
    NSLog(@"pa=%d,pb=%d",*pa,*pb);
}

-(NSMutableArray<NSNumber*>*)list
{
    if (_list == nil) {
        _list = [NSMutableArray array];
    }
    return _list;
}

-(NSMutableDictionary<NSNumber*,NSNumber*>*)info
{
    if (_info == nil) {
        _info = [NSMutableDictionary dictionary];
    }
    return _info;
}

-(void)_test
{
    RBTree_S *tree = calloc(1, sizeof(RBTree_S));
    tree->compare = compareValue;
    tree->copy = copyValue;
    tree->swap = swapValue;
//    tree->release = release;
    tree->enumerator = enumerator;
    
    int a[] = {12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};//{4,6,2,9,7,0,22,10,20,16,23,11,26,14,30,19,28,31,15,3};//{12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};
    int cnt = sizeof(a)/sizeof(int);
    for (int i = 0; i < cnt; ++i) {
        int b = a[i];
        RBTreeNode_S *node = calloc(1, sizeof(RBTreeNode_S));
        T t = {.V.val = b};
        node->key = t;
        insertRBTree(tree, node);
        
        NSLog(@"\ni=%d,insertV=%d,插入后:\n",i,b);
        enumerateRBTree(tree, BTreeEnumerateTypeZ, NULL);
    }
    
    NSLog(@"delete:====================分割线");
//    int d[] ={12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};
    cnt = sizeof(a)/sizeof(int);
    for (int i = 0; i < cnt; ++i) {
        int r = a[i];
        NSLog(@"\n\ni=%d,deleteV=%d",i,r);
//        if (i == 13) {
//            NSLog(@"====================");
//        }
        
        deleteRBTreeWithKey(tree, r);
        
        enumerateRBTree(tree, BTreeEnumerateTypeZ, NULL);
//        NSLog(@"\n\n");
//
//        if (i == 13) {
//            break;
//        }
    }
    NSLog(@"FINISH================");
}


int64_t getUptimeInMilliseconds()
{
    const int64_t kOneMillion = 1000 * 1000;
    static mach_timebase_info_data_t s_timebase_info;
    
    if (s_timebase_info.denom == 0) {
        (void) mach_timebase_info(&s_timebase_info);
    }
    
    return ((mach_absolute_time() * s_timebase_info.numer) / (kOneMillion * s_timebase_info.denom));
}



-(void)_test2
{
    int32_t cnt = 1000000;//5000000;
    int32_t mask = 1048575;//8388607;//1048575;
    
    
    RBTreeNode_S *ptrNodeT = malloc(cnt * sizeof(RBTreeNode_S));//calloc(cnt, sizeof(RBTreeNode_S));
    if (ptrNodeT == NULL) {
        return;
    }
    memset(ptrNodeT, 0, cnt * sizeof(RBTreeNode_S));
    
    RBTree_S *tree = calloc(1, sizeof(RBTree_S));
    tree->compare = compareValue;
    tree->copy = copyValue;
    tree->swap = swapValue;
//    tree->free = release;
    tree->enumerator = enumerator;
    
    int32_t i = 0;
    while (i < cnt) {
        uint32_t val = arc4random()&mask;
        NSNumber *num = @(val);
        if (![self.info objectForKey:num]) {
            [self.list addObject:num];
            [self.info setObject:num forKey:num];
            T t = {.V.val = val};
            ptrNodeT[i].key = t;
            insertRBTree(tree, &ptrNodeT[i]);
            ++i;
        }
    }
    
//    enumerateRBTree(tree, BTreeEnumerateTypeZ, NULL);
//    return;
//    [self.list enumerateObjectsUsingBlock:^(NSNumber * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
//        NSLog(@"idx=%ld,val=%@",idx,obj);
//    }];
    

    NSLog(@"start===========================");
    int64_t start = getUptimeInMilliseconds();
    [self.list enumerateObjectsUsingBlock:^(NSNumber * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [self.info objectForKey:obj];
    }];
    int64_t end = getUptimeInMilliseconds();
    NSLog(@"info.differ=%@",@(end - start));
    
    start = getUptimeInMilliseconds();
    [self.list enumerateObjectsUsingBlock:^(NSNumber * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        selectRBTreeWithKey(tree, [obj unsignedIntValue]);
    }];
    end = getUptimeInMilliseconds();
    NSLog(@"tree.differ=%@,count=%d",@(end - start),tree->count);
    
//    enumerateRBTree(tree, BTreeEnumerateTypeZ, NULL);
//    enumerateRBTree(tree, BTreeEnumerateTypeLNR, NULL);
//    enumerateRBTree(tree, BTreeEnumerateTypeRNL, NULL);
    
    if (tree) {
        free(tree);
    }
    if (ptrNodeT) {
        free(ptrNodeT);
    }
//    self.info = nil;
//    self.list = nil;
}

int8_t hashMapShouldAdjustFunc(struct YZHHashMap *hashMap)
{
    return 0;
}

-(void)_test4
{
    int32_t cnt = self.list.count;//1000000;//100;
//    int32_t mask = 1048575;//127;//1048575;
    
    int32_t capcity = cnt * 0.2;//cnt * 0.2;//cnt;//1048576;//524288;//cnt;//1048576;//524288;//262144;//10;//10000;
    YZHHashMap_S *hashMap = allocHashMapWithCapacity(capcity);
    hashMap->shouldAdjustFunc = hashMapShouldAdjustFunc;

    [self.list enumerateObjectsUsingBlock:^(NSNumber * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        uint32_t val = [obj unsignedIntValue];
        T key = {.V.val = val};
        T value = {.V.val = val};
        insertHashMap(hashMap, &key, &value);
    }];
    
    NSLog(@"========hashMap.cout=%d",hashMap->count);
    
//    print(hashMap);
//    return ;
    
    
    NSLog(@"start===========================");
    int64_t start = getUptimeInMilliseconds();
    [self.list enumerateObjectsUsingBlock:^(NSNumber * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [self.info objectForKey:obj];
    }];
    int64_t end = getUptimeInMilliseconds();
    NSLog(@"info.differ=%@",@(end - start));
    
    start = getUptimeInMilliseconds();
    [self.list enumerateObjectsUsingBlock:^(NSNumber * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        int32_t val = [obj unsignedIntValue];
        T key = {.V.val = val};
        T *value = selectHashMap(hashMap, &key);
        if (value->V.val != val) {
            NSLog(@"===================2");
            *stop = YES;
        }
//        selectHashMap(hashMap, &key);
    }];
    end = getUptimeInMilliseconds();
    NSLog(@"hashMap.differ=%@,count=%d",@(end - start),hashMap->count);
    
    freeHashMap(hashMap);
    self.info = nil;
    self.list = nil;
}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self _test2];
    [self _test4];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}




@end
