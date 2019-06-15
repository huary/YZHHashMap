//
//  Queue.c
//  YZHHashMapDemo
//
//  Created by yuan on 2019/6/3.
//  Copyright © 2019年 yuan. All rights reserved.
//

#include "Queue.h"

void push(struct Queue *queue, struct QueueNode *node)
{
    if (queue == NULL || node == NULL) {
        return;
    }
    ++queue->count;
    if (queue->head == queue->tail && queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
        return;
    }
    QueueNode_S *tail = queue->tail;
    tail->next = node;
    node->prev = tail;
    node->next = NULL;
    
    queue->tail = node;
}

QueueNode_S* pop(struct Queue *queue)
{
    if (queue == NULL || queue->head == NULL) {
        return NULL;
    }
     --queue->count;
    QueueNode_S *node = queue->head;
    node->prev = NULL;
    
    queue->head = node->next;
    
    node->next = NULL;
    if (queue->tail == node) {
        queue->tail = NULL;
    }
    return node;
}

void clearQueue(struct Queue *queue)
{
    while (queue->head != NULL) {
        pop(queue);
    }
}
