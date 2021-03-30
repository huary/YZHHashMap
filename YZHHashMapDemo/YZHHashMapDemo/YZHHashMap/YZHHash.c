//
//  YZHHash.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/8/25.
//  Copyright © 2019 yuan. All rights reserved.
//

#include "YZHHash.h"
#include "macro.h"
//2^5 - 1 = 31
static int32_t defaultHashSeedLS_Bits_s = 5;


uint64_t BKDRHash(T *key)
{
    if (key == NULL) {
        return 0;
    }
    uint64_t hash = 0;
    if (key->size == 0) {
        uint64_t val = key->V.val;
        return val;
    }
    else if (key->size > 0) {
        int32_t i = 0;
        int32_t size = key->size;
        uint8_t *ptr = key->V.ptr;
        for (i = 0 ; i < size; ++i) {
            hash = TYPE_LS(hash, defaultHashSeedLS_Bits_s) - hash + ptr[i];
        }
    }
    else {
        
    }
    return hash;
}
