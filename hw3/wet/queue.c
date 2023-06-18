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



// Check if the queue is empty
int isEmpty(Queue* queue) {
    return queue->queue_size == 0;
}

// Insert an element into the queue
void enqueue(Queue* queue, int data, double arrival_time) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;
    new_node->arrival_time = arrival_time;

    if (isEmpty(queue)) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
    queue->queue_size++;
}

// Remove an element from the queue
int dequeue(Queue* queue, double* arrival_time, double* dispatch_time) {
    if (isEmpty(queue)) {
        return -1;
    }

    Node* temp = queue->front;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    if(arrival_time != NULL){
        *arrival_time = temp->arrival_time;
    }
    if(dispatch_time != NULL) {
        *dispatch_time = Time_GetSeconds() - temp->arrival_time;
    }

    int result = temp->data;
    free(temp);
    queue->queue_size--;
    return result;
}

int dequeue_i(Queue* queue, int i){
    if(i > queue->queue_size){
        return -1;
    }

    if(i == 1){
        int result = queue->front->data;
        queue->front = queue->front->next;
        if(queue->queue_size == 1){
            queue->rear = NULL;
        }
        queue->queue_size--;
        return result;
    }
    int curr = 1;
    Node* temp = queue->front;
    while(curr != i-1){
        temp = temp->next;
        curr++;
    }
    Node* node_to_delete = temp->next;
    temp->next = node_to_delete->next;

    if(i == queue->queue_size){
        queue->rear = temp;
    }
    int result = node_to_delete->data;
    free(node_to_delete);
    queue->queue_size--;
    return result;
}