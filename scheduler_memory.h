#include "scheduler_logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TOTAL_MEMORY 1024


typedef struct MemoryBlock {
    int start;                  
    int size;                 
    bool is_free;              
    struct MemoryBlock* left;  
    struct MemoryBlock* right; 
} MemoryBlock;

// global variable
MemoryBlock* memoryRoot;

// here i use binary tree to split the blocks
void initializeMemory() {
    memoryRoot = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    memoryRoot->start = 0;
    memoryRoot->size = TOTAL_MEMORY;
    memoryRoot->is_free = true;  
    memoryRoot->left = NULL;
    memoryRoot->right = NULL;
}

MemoryBlock* allocateBlock(MemoryBlock* block, int size) {
    printf("block size = %d\n", block->size);
    // check if block isn't free and if it is too small
    if (!block->is_free || block->size < size) {
        return NULL; 
    }

    // check if block is exactly the same size and take it
    if (block->size == size) {
        block->is_free = false;
        return block;
    }

    // check if block can't be split
    if (block->size > size && block->size < size * 2) {
        block->is_free = false;
        return block;
    }

    if (!block->left && !block->right) {
        // Split the block 
        int halfSize = block->size / 2;
        block->left = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        block->right = (MemoryBlock*)malloc(sizeof(MemoryBlock));

        block->left->start = block->start;
        block->left->size = halfSize;
        block->left->is_free = true;
        block->left->left = NULL;
        block->left->right = NULL;

        block->right->start = block->start + halfSize;
        block->right->size = halfSize;
        block->right->is_free = true;
        block->right->left = NULL;
        block->right->right = NULL;
    }

    // using recursion allocate memory
    MemoryBlock* allocatedBlock = allocateBlock(block->left, size);
    if (!allocatedBlock) {
        allocatedBlock = allocateBlock(block->right, size);
    }
    return allocatedBlock;
}

// use recursion to deallocate blocks
void deallocateBlock(MemoryBlock* block, int start) {
    if (!block || block->start > start || (block->start + block->size) <= start) {
        return;
    }

    if (block->start == start && block->size <= 256) {
        block->is_free = true;
        return;
    }

    deallocateBlock(block->left, start);
    deallocateBlock(block->right, start);

    // Check if children can be merged
    if (block->left && block->right && block->left->is_free && block->right->is_free) {
        free(block->left);
        free(block->right);
        block->left = NULL;
        block->right = NULL;
        block->is_free = true;
    }
}

// Function to allocate memory
bool allocateMemory() {
    MemoryBlock* allocatedBlock = allocateBlock(memoryRoot, globalRunningPCBObject.memorySize);
    if (allocatedBlock) {
        globalRunningPCBObject.memoryStart = allocatedBlock->start;
        logMemoryAllocated();
        return true;
    } else {
        printf("Error: Not enough memory for process %d\n", globalRunningPCBObject.processID);
        return false;
    }
}

// Function to deallocate memory
void deallocateMemory() {
    deallocateBlock(memoryRoot, globalRunningPCBObject.memoryStart);
    logMemoryDeallocated();
}
