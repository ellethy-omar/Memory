#include "scheduler_signal_handling.h"



void shortestJobFirstAlgorithm()
{
    PriorityQueue PCBpriQueue;
    initPriorityQueue(&PCBpriQueue);
    globalPCBpriQueue = &PCBpriQueue;
    
    currentTime = 1;

    while (!IsProcessGeneratoroutOfProcesses || !isEmptyPriorty(globalPCBpriQueue) || !isEmpty(globalBLockedProcessesQueue))
    {
        while (isEmptyPriorty(globalPCBpriQueue) && !processRunning)
        {
            if (getClk() > currentTime)
            {
                idleTime++;
                currentTime = getClk();
            }
        }

        while (processRunning);

        if (!isEmpty(globalBLockedProcessesQueue))
        {
            globalRunningPCBObject = dequeue(globalBLockedProcessesQueue);
        
            startProcess();
        }

        while (processRunning);

        if (!isEmptyPriorty(globalPCBpriQueue))
        {
            globalRunningPCBObject = dequeuePriority(globalPCBpriQueue);
        }

        if (globalRunningPCBObject.processID != -1)
            startProcess();
    }

    while (processRunning);
}

void prememtiveHigheestPriorityFirstAlgorithm()
{
    PriorityQueue PCBpriQueue;
    initPriorityQueue(&PCBpriQueue);
    globalPCBpriQueue = &PCBpriQueue;

    currentTime = getClk();

    while (!IsProcessGeneratoroutOfProcesses || !isEmptyPriorty(globalPCBpriQueue) || !isEmpty(globalBLockedProcessesQueue))
    {
        while (isEmptyPriorty(globalPCBpriQueue) && !processRunning)
        {
            if (getClk() > currentTime)
            {
                idleTime++;
                currentTime = getClk();
            }
        }

        if (!isEmpty(globalBLockedProcessesQueue))
        {
            globalRunningPCBObject = dequeue(globalBLockedProcessesQueue);
        
            if (globalRunningPCBObject.status == 0)
                startProcess();
        }

        while (processRunning);

        globalRunningPCBObject = dequeuePriority(globalPCBpriQueue);

        if (globalRunningPCBObject.status == 0)
            startProcess();
        else if (globalRunningPCBObject.status == 2)
            resumeCurrentProcess();

        while (processRunning);
    };

    while (processRunning);
}

void roundRobinAlgorithm(int* roundRobinQuantum) 
{
    Queue PCBQueue;
    initQueue(&PCBQueue);
    globalPCBQueue = &PCBQueue;
    
    currentTime = getClk();

    while (!IsProcessGeneratoroutOfProcesses || !isEmpty(globalPCBQueue))
    {
        while (isEmpty(globalPCBQueue) && !processRunning)
        {
            if (getClk() > currentTime)
            {
                idleTime++;
                currentTime = getClk();
            }
        }

        if (processRunning)
        {
            for (int i = 0; i < *roundRobinQuantum; i++)
            {
                currentTime = getClk();
                while (getClk() <= currentTime)
                    if (!processRunning)
                        break;
                    

                if (!processRunning)
                    break;
            }

            if (processRunning)
            {
                interruptCurrentProcess();
                enqueue(globalPCBQueue, globalRunningPCBObject);
            }
        }

        globalRunningPCBObject = dequeue(globalPCBQueue);

        if (globalRunningPCBObject.status == 0)
            startProcess();
        else if (globalRunningPCBObject.status == 2)
            resumeCurrentProcess();

        currentTime = getClk();
    };

    while (processRunning);
};

void multiLevelFeedbackLoopAlgorithm(int* roundRobinQuantum)    
{
    globalPCBQueue = (Queue*)malloc(11 * sizeof(Queue));

    if (!globalPCBQueue) {
        perror("Failed to allocate memory for queue array");
        exit(EXIT_FAILURE);
    };

    for (int i = 0; i < 11; i++)
        initQueue(&globalPCBQueue[i]);

    currentTime = getClk();

    while (!IsProcessGeneratoroutOfProcesses || !multiLevelIsEmpty(globalPCBQueue))
    {
        while (multiLevelIsEmpty(globalPCBQueue) && !processRunning)
        {
            if (getClk() > currentTime)
            {
                idleTime++;
                currentTime = getClk();
            }
        }

        if (processRunning)
        {
            for (int i = 0; i < *roundRobinQuantum; i++)
            {
                currentTime = getClk();
                while (getClk() <= currentTime)
                    if (!processRunning)
                        break;
                    

                if (!processRunning)
                    break;
            }

            if (processRunning)
            {
                interruptCurrentProcess();
                multiLevelEnqueue(globalPCBQueue, globalRunningPCBObject);
            }
        }

        globalRunningPCBObject = multiLevelDequeue(globalPCBQueue);

        if (globalRunningPCBObject.status == 0)
            startProcess();
        else if (globalRunningPCBObject.status == 2)
            resumeCurrentProcess();

        currentTime = getClk();
    };

    while (processRunning);

    free(globalPCBQueue);
};
