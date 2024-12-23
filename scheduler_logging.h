#include "scheduler_data_structures.h"
#include <math.h>

ProceesControlBlock globalRunningPCBObject;
FILE *writerLogPref;
FILE *wirterMemoryLog;
LinkedListFinishedState* finishedProcessesList;

void logProcessStarted()
{
    fprintf(
        writerLogPref,
        "At time %d process %d started arr %d total %d remain %d wait %d\n",
        getClk(), 
        globalRunningPCBObject.processID, 
        globalRunningPCBObject.arrivalTime,
        globalRunningPCBObject.runningTime,
        globalRunningPCBObject.runningTime - (getClk() - globalRunningPCBObject.arrivalTime - globalRunningPCBObject.waitingTime),
        globalRunningPCBObject.waitingTime
    );
}

void logProcessInterrupted()
{
    fprintf(
        writerLogPref,
        "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
        getClk(),
        globalRunningPCBObject.processID,
        globalRunningPCBObject.arrivalTime,
        globalRunningPCBObject.runningTime, 
        globalRunningPCBObject.runningTime - (getClk() - globalRunningPCBObject.arrivalTime - globalRunningPCBObject.waitingTime),
        globalRunningPCBObject.waitingTime
    );
}

void logProcessResumed()
{
    fprintf(
        writerLogPref,
        "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
        getClk(),
        globalRunningPCBObject.processID,
        globalRunningPCBObject.arrivalTime,
        globalRunningPCBObject.runningTime, 
        globalRunningPCBObject.runningTime - (getClk() - globalRunningPCBObject.arrivalTime - globalRunningPCBObject.waitingTime),
        globalRunningPCBObject.waitingTime
    );
}

void logProcessFinished(FinishedState* finishedProcess)
{
    fprintf(
        writerLogPref,
        "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %f\n",
        getClk(),
        globalRunningPCBObject.processID,
        globalRunningPCBObject.arrivalTime,
        globalRunningPCBObject.runningTime,
        0,
        globalRunningPCBObject.waitingTime,
        getClk() - globalRunningPCBObject.arrivalTime,
        finishedProcess->WTA
    );
}

int nextPowerOfTwo(unsigned int n) {
    if (n == 0) return 1;
    if ((n & (n - 1)) == 0) return n; // n is already a power of two
    return pow(2, ceil(log2(n)));    // Calculate the next power of two
}

void logMemoryAllocated()
{
    int alignedSize = nextPowerOfTwo(globalRunningPCBObject.memorySize);
    int endAddress = globalRunningPCBObject.memoryStart + alignedSize - 1;
    fprintf(
        wirterMemoryLog,
        "At time %d allocated %d bytes for process %d from %d to %d \n",
        getClk(),
        globalRunningPCBObject.memorySize,
        globalRunningPCBObject.processID,
        globalRunningPCBObject.memoryStart, 
        endAddress
    );
}

void logMemoryDeallocated()
{
    int alignedSize = nextPowerOfTwo(globalRunningPCBObject.memorySize);
    int endAddress = globalRunningPCBObject.memoryStart + alignedSize - 1;
    fprintf(
        wirterMemoryLog,
        "At time %d freed %d bytes for process %d from %d to %d \n",
        getClk(),
        globalRunningPCBObject.memorySize,
        globalRunningPCBObject.processID,
        globalRunningPCBObject.memoryStart, 
        endAddress
    );
}
