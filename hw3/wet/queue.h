//
// Created by student on 6/17/23.
//
#include "segel.h"

#ifndef WET_QUEUE_H
#define WET_QUEUE_H
// Node structure
typedef struct Node {
    void* data;
    double arrival_time;
    struct Node* next;
} Node;

// Queue structure
typedef struct Queue {
    Node* front;
    Node* rear;
    int queue_size;
} Queue;

// Function prototypes
Queue* createQueue();
int isEmpty(Queue* queue);
void enqueue(Queue* queue, void* data);
void* dequeue(Queue* queue, double* arrival_time, double* dispatch_time);

#endif //WET_QUEUE_H
