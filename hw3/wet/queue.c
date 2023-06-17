//
// Created by student on 6/17/23.
//
#include "queue.h"

// Create an empty queue
Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = queue->rear = NULL;
    queue->queue_size = 0;
    return queue;
}

double Time_GetSeconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double) ((double)t.tv_sec + (double)t.tv_usec / 1e6);
}

// Check if the queue is empty
int isEmpty(Queue* queue) {
    return queue->queue_size == 0;
}

// Insert an element into the queue
void enqueue(Queue* queue, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;
    new_node->arrival_time = Time_GetSeconds();

    if (isEmpty(queue)) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
    queue->queue_size++;
}

// Remove an element from the queue
void* dequeue(Queue* queue, double* arrival_time, double* dispatch_time) {
    if (isEmpty(queue)) {
        return NULL;
    }

    Node* temp = queue->front;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    *arrival_time = temp->arrival_time;
    *dispatch_time = temp->arrival_time - Time_GetSeconds();

    void* result = temp->data;
    free(temp);
    queue->queue_size--;
    return result;
}