//
//  Queue.h
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/3.
//  Copyright © 2019年 yuan. All rights reserved.
//

#ifndef Queue_h
#define Queue_h

#include <stdio.h>

typedef struct QueueNode{
    struct QueueNode *prev;
    struct QueueNode *next;
    int64_t iValue;
    void *value;
    void *userInfo;
}QueueNode_S,*PQueueNode_S;


typedef struct Queue{
    struct QueueNode *head;
    struct QueueNode *tail;
    int32_t count;
}Queue_S;

void push(struct Queue *queue, struct QueueNode *node);
QueueNode_S* pop(struct Queue *queue);
void clearQueue(struct Queue *queue);

#endif /* Queue_h */
