//
//  YZHType.h
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/8.
//  Copyright © 2019年 yuan. All rights reserved.
//

#ifndef YZHType_h
#define YZHType_h

#include <stdio.h>

#define INTEGER_SWAP(X,Y)           (X==Y ?:(X=(X)^(Y), Y=(X)^(Y), X=(X)^(Y)))

#define _PTR_VAL(PTR)               (*(&(PTR)))
//左值
#define _PTR_VAL_L(PTR)             _PTR_VAL(PTR)
//右值
#define _PTR_VAL_R(PTR)             ((intptr_t)_PTR_VAL(PTR))
//异或
#define PTR_XOR(PTR_X,PTR_Y)        (_PTR_VAL_R(PTR_X) ^ _PTR_VAL_R(PTR_Y))
//指针交换
#define PTR_SWAP(PTR_X,PTR_Y)       ((PTR_X)==(PTR_Y) ?:(_PTR_VAL_L(PTR_X)= PTR_XOR(PTR_X,PTR_Y),_PTR_VAL_L(PTR_Y)= PTR_XOR(PTR_X,PTR_Y),_PTR_VAL_L(PTR_X)=PTR_XOR(PTR_X,PTR_Y)))


typedef enum YZHComparisonResult  {
    YZHOrderedASC       = -1,
    YZHOrderedEQ        = 0,
    YZHOrderedDES       = 1,
}YZHComparisonResult_E;

struct T;
typedef void (*initTFunc)(struct T *t);
typedef void (*deallocTFunc)(struct T *t);

typedef struct T{
    //size为0时，取.val，size>0否则取ptr所指向的为size长度的内容,size<0时取ptr指向的对象(C++,OBJC之类的对象，自己实现比较函数)
    int32_t size;
    union {
        uint8_t *ptr;
        uint64_t val;
    } V;
    
    initTFunc init;
    deallocTFunc dealloc;
}T;

void memswap(uint8_t *first, uint8_t *second, size_t size);

YZHComparisonResult_E compare(T *first, T *second);
//void copy(T *src, T *dst);
void swap(T *first, T *second);


#endif /* YZHType_h */
