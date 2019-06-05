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


//#define INT_SWAP(X,Y)       (X==Y ?:(X=X^Y,Y=X^Y,X=X^Y))
@interface ViewController ()

/* <#注释#> */
@property (nonatomic, strong) NSMutableArray<NSNumber*> *list;

/* <#注释#> */
@property (nonatomic, strong) NSMutableDictionary<NSNumber*,NSNumber*> *info;

@end

@implementation ViewController

static RBTreeNodeComparisonResult_E compare(RBTreeNode_S *first, RBTreeNode_S *second)
{
    if (first->key < second->key) {
        return ASC;
    }
    else if (first->key == second->key) {
        return SAME;
    }
    else {
        return DESC;
    }
}

static void copy(RBTreeNode_S *src, RBTreeNode_S *dst)
{
    dst->key = src->key;
}

static void swap(RBTreeNode_S *first, RBTreeNode_S *second)
{
//    NSLog(@"1、frist=%@,second=%@",@(first->key),@(second->key));
    INTEGER_SWAP(first->key, second->key);
//    NSLog(@"2、frist=%@,second=%@",@(first->key),@(second->key));

}

static void release(RBTreeNode_S *node)
{
//    if (node == NULL) {
//        free(node);
//        node = NULL;
//    }
}

static void enumerator(RBTreeNode_S *node, int32_t level)
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
    
    [self _test2];
}

//@property (nonatomic, strong) NSMutableArray<NSNumber*> *list;

/* <#注释#> */
//@property (nonatomic, strong) NSMutableDictionary<NSNumber*,NSNumber*> *info;

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
    tree->compare = compare;
    tree->copy = copy;
    tree->swap = swap;
    tree->release = release;
    tree->enumerator = enumerator;
    
    int a[] = {12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};//{4,6,2,9,7,0,22,10,20,16,23,11,26,14,30,19,28,31,15,3};//{12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};
    int cnt = sizeof(a)/sizeof(int);
    for (int i = 0; i < cnt; ++i) {
        int b = a[i];
        RBTreeNode_S *node = calloc(1, sizeof(RBTreeNode_S));
        node->key = b;
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
    int32_t cnt = 1000000;
    int32_t mask = 1048575;
    
    
    RBTreeNode_S *ptrNodeT = malloc(cnt * sizeof(RBTreeNode_S));//calloc(cnt, sizeof(RBTreeNode_S));
    if (ptrNodeT == NULL) {
        return;
    }
    memset(ptrNodeT, 0, cnt * sizeof(RBTreeNode_S));
    
    RBTree_S *tree = calloc(1, sizeof(RBTree_S));
    tree->compare = compare;
    tree->copy = copy;
    tree->swap = swap;
    tree->release = release;
    tree->enumerator = enumerator;
    
    int32_t i = 0;
    while (i < cnt) {
        uint32_t val = arc4random()&mask;
        NSNumber *num = @(val);
        if (![self.info objectForKey:num]) {
            [self.list addObject:num];
            [self.info setObject:num forKey:num];
            
            ptrNodeT[i].key = val;
            insertRBTree(tree, &ptrNodeT[i]);
            ++i;
        }
    }
    
    
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
    self.info = nil;
    self.list = nil;
}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self _test2];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
    
//    [self.view.subviews en]
}




@end
