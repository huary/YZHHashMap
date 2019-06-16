//
//  YZHType.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/11.
//  Copyright © 2019年 yuan. All rights reserved.
//

#include "YZHType.h"
#include "type.h"
#include <stdlib.h>
#include <memory.h>

void memswap(uint8_t *first, uint8_t *second, size_t size)
{
    size_t i = 0;
    while (i < size) {
        if (size > i + 4) {
            INTEGER_SWAP(*((int32_t*)first+i), *((int32_t*)second+i));
            i = i + 4;
        }
        else {
            INTEGER_SWAP(*(first+i), *(second+i));
            ++i;
        }
    }
}


YZHComparisonResult_E compare(T *first, T *second)
{
    if (first == NULL || second == NULL) {
        return YZHOrderedEQ;
    }
    if (first->size == 0 && second->size == 0) {
        if (first->V.val < second->V.val) {
            return YZHOrderedASC;
        }
        else if (first->V.val == second->V.val) {
            return YZHOrderedEQ;
        }
        else {
            return YZHOrderedDES;
        }
    }
    else if (first->size > 0 && second->size > 0) {
        int32_t size = first->size > second->size ? second->size : first->size;
        int r = memcmp(first->V.ptr, second->V.ptr, size);
        if (r < 0) {
            return YZHOrderedASC;
        }
        else if (r == 0) {
            if (first->size == second->size) {
                return YZHOrderedEQ;
            }
            else if (first->size > second->size) {
                return YZHOrderedDES;
            }
            else {
                return YZHOrderedASC;
            }
        }
        else {
            return YZHOrderedDES;
        }
    }
    return YZHOrderedEQ;    
}


//void copy(T *src, T *dst)
//{
//    
//}
void swap(T *first, T *second)
{
    if (first == NULL || second == NULL) {
        return;
    }
    INTEGER_SWAP(first->size, second->size);
    INTEGER_SWAP(first->V.val, second->V.val);
    PTR_SWAP(first->V.ptr, second->V.ptr);
}

