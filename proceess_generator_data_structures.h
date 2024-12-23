#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Defining the ProcessData structure

typedef struct Node {
    struct ProcessData data;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} Queue;

void initializeQueue(Queue* q)
{
    q->front = NULL;
    q->rear = NULL;
}

bool isEmptyQ(Queue* q) { return (q->front == NULL); }

void enqueueQ(Queue* q, struct ProcessData value)
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new node");
        return;
    }

    newNode->data = value;
    newNode->next = NULL;

    if (isEmptyQ(q)) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

struct ProcessData dequeueQ(Queue* q)
{
    if (isEmptyQ(q)) {
        struct ProcessData emptyProcess;
        emptyProcess.id = 0;
        emptyProcess.arrivaltime = -1;
        return emptyProcess;
    }

    Node* temp = q->front;
    struct ProcessData dequeuedData = temp->data;

    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    return dequeuedData;
}

struct ProcessData peekQ(Queue* q)
{
    if (isEmptyQ(q)) {
        struct ProcessData emptyProcess = {0, 0, 0, 0};
        return emptyProcess;
    }

    return q->front->data;
}

void printQueue(Queue* q)
{
    if (isEmptyQ(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue:\n");
    Node* temp = q->front;
    while (temp != NULL) {
        struct ProcessData p = temp->data;
        printf("ID: %d, Arrival: %d, Priority: %d, Runtime: %d\n",
               p.id, p.arrivaltime, p.priority, p.runningtime);
        temp = temp->next;
    }
    printf("\n");
}