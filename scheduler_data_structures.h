#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headers.h"
//------------------------------------------
//  STRUCT DEFINITIONS
//------------------------------------------

typedef struct ProceesControlBlock {
    int processID;
    int arrivalTime;
    int priority;
    int runningTime;
    int waitingTime;
    int forkID;
    int interupredAt;
    int status; // 0 = waiting, 1 = running, 2 = stopped, 3 = resumed, 4 = finished
    int memorySize;
    int memoryStart;
} ProceesControlBlock;

typedef struct FinishedState
{
    float WTA;
    int waitingTime;
} FinishedState;

typedef struct NodePCB {
    ProceesControlBlock data;
    struct NodePCB* next;
} NodePCB;

typedef struct Queue {
    NodePCB* front;
    NodePCB* rear;
} Queue;

typedef struct PriorityQueue {
    NodePCB* front;
} PriorityQueue;

typedef struct NodeFinishedState
{
    FinishedState data;
    struct NodeFinishedState* next;
} NodeFinishedState;

typedef struct LinkedListFinishedState
{
    NodeFinishedState* head;
    NodeFinishedState* tail;
} LinkedListFinishedState;

//------------------------------------------

void printPCB(ProceesControlBlock* pcb)
{
    printf("Current process has id: %d, status: %d, priority: %d, runnningTime: %d\n",
        pcb->processID, pcb->status, pcb->priority, pcb->runningTime
    );
}

//------------------------------------------
//  PCBQUEUES
//------------------------------------------

void initQueue(Queue* q) {
    q->front = q->rear = NULL;
}

bool isEmpty(Queue* q) {
    return q->front == NULL;
}

void enqueue(Queue* q, ProceesControlBlock pcb) {
    NodePCB* newNode = (NodePCB*)malloc(sizeof(NodePCB));
    if (!newNode) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    newNode->data = pcb;
    newNode->next = NULL;
    if (isEmpty(q)) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

ProceesControlBlock dequeue(Queue* q) {
    if (isEmpty(q)) {
        fprintf(stderr, "Queue is empty!\n");
        ProceesControlBlock pcb;
        pcb.status = -1;
        pcb.processID = -1;
    }
    NodePCB* temp = q->front;
    ProceesControlBlock pcb = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return pcb;
}

void printQueue(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty!\n");
        return;
    }

    NodePCB* current = q->front;
    printf("Queue contents:\n");
    while (current != NULL) {
        printPCB(&current->data);
        current = current->next;
    }
}

//------------------------------------------
//  MULTI-LVEL-PCBQUEUES
//------------------------------------------

bool multiLevelIsEmpty(Queue* queuesPointer)
{
    for (int i = 0; i < 11; i++)
    {
        if (!isEmpty(&queuesPointer[i]))
            return false;     
    }

    return true;
}

void multiLevelEnqueue(Queue* queuesPointer, ProceesControlBlock pcb)
{
    enqueue(&queuesPointer[pcb.priority], pcb);
}

ProceesControlBlock multiLevelDequeue(Queue* queuesPointer)
{
    for (int i = 0; i < 11; i++)
    {
        if (!isEmpty(&queuesPointer[i]))
        {
            ProceesControlBlock pcb = dequeue(&queuesPointer[i]);
            if (pcb.priority != 10)
                pcb.priority++;
            
            return pcb;
        }
    }

    ProceesControlBlock pcb;
    pcb.status = -1;
    pcb.processID = -1;

    return pcb;
}

void multiLevelPrint(Queue* queuesPointer)
{
    for (int i = 0; i < 11; i++)
        printQueue(&queuesPointer[i]);
}

//------------------------------------------
//  PRIORITY-PCBQUEUES
//------------------------------------------

void initPriorityQueue(PriorityQueue* pq) {
    pq->front = NULL;
}

bool isEmptyPriorty(PriorityQueue* pq)
{
    return pq->front == NULL;
}

void enqueuePriority(PriorityQueue* pq, ProceesControlBlock pcb, bool* usePriority) {
    NodePCB* newNode = (NodePCB*)malloc(sizeof(NodePCB));
    if (!newNode) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    newNode->data = pcb;
    newNode->next = NULL;

    bool compareByPriority = (usePriority == NULL) || *usePriority;
    int comparisonValue = compareByPriority ? pcb.priority : pcb.runningTime;

    if (pq->front == NULL || 
       (compareByPriority ? pq->front->data.priority > comparisonValue : pq->front->data.runningTime > comparisonValue)) {
        newNode->next = pq->front;
        pq->front = newNode;
    } else {
        NodePCB* current = pq->front;

        while (current->next != NULL && 
              (compareByPriority ? current->next->data.priority <= comparisonValue : current->next->data.runningTime <= comparisonValue)) {
            current = current->next;
        }

        newNode->next = current->next;
        current->next = newNode;
    }
}

ProceesControlBlock dequeuePriority(PriorityQueue* pq) {
    if (pq->front == NULL) {
        ProceesControlBlock pcb;
        pcb.status = -1;
        pcb.processID = -1;
    }

    NodePCB* temp = pq->front;
    ProceesControlBlock pcb = temp->data;
    pq->front = pq->front->next;
    free(temp);
    return pcb;
}

void printPriorityQueue(PriorityQueue* pq) {
    if (pq->front == NULL) {
        printf("Priority Queue is empty!\n");
        return;
    }

    NodePCB* current = pq->front;
    printf("Priority Queue contents:\n");
    while (current != NULL) {
        printf("ProcessID: %d, Priority: %d, ArrivalTime: %d\n", 
                current->data.processID, 
                current->data.priority, 
                current->data.arrivalTime);
        current = current->next;
    }
}

//------------------------------------------
//  FINISHEDLIST
//------------------------------------------

void initList(LinkedListFinishedState* list)
{
    list->head = NULL;
    list->tail = NULL;
}

bool isEmptyList(LinkedListFinishedState* list)
{
    return list->head == NULL;
}

NodeFinishedState* createNode(FinishedState finishedState)
{
    NodeFinishedState* newNode = (NodeFinishedState*)malloc(sizeof(NodeFinishedState));
    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newNode->data = finishedState;
    newNode->next = NULL;
    return newNode;
}

void appendToList(LinkedListFinishedState* list, FinishedState finishedState)
{
    NodeFinishedState* newNode = createNode(finishedState);
    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;
    }
    else
    {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

void printList(LinkedListFinishedState* list)
{
    NodeFinishedState* temp = list->head;
    while (temp != NULL)
    {
        printf("WTA: %.2f, Waiting Time: %d\n", temp->data.WTA, temp->data.waitingTime);
        temp = temp->next;
    }
}

void freeList(LinkedListFinishedState* list)
{
    NodeFinishedState* temp = list->head;
    while (temp != NULL)
    {
        NodeFinishedState* nextNode = temp->next;
        free(temp);
        temp = nextNode;
    }
    list->head = NULL;
    list->tail = NULL;
}

FinishedState dequeueFromList(LinkedListFinishedState* list)
{
    if (isEmptyList(list))
    {
        FinishedState temp;
        temp.waitingTime = -1;
        return temp;
    }

    NodeFinishedState* temp = list->head;
    FinishedState dequeuedData = temp->data;
    list->head = list->head->next;
    if (list->head == NULL)
    {
        list->tail = NULL;
    }
    free(temp);
    return dequeuedData;
}