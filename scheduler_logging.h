#include "scheduler_data_structures.h"

ProceesControlBlock globalRunningPCBObject;
FILE *writerLogPref;
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
