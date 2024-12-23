#include "scheduler_memory.h"
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

// Pointer mostly means it is not used in all algorithms with exception of finishedProcessesList,
// to make it easier to use in functions, since I forget when to use * and &
// object means it is used in all algorithms

Queue* globalPCBQueue;
PriorityQueue* globalPCBpriQueue;
Queue* globalBLockedProcessesQueue;

int schedulingAlgorithm;
int msgq_id;

bool* usePriorityPointer;
int idleTime;
int currentTime;
bool processRunning;
bool IsProcessGeneratoroutOfProcesses;

void startProcess()
{
    if (!allocateMemory(globalRunningPCBObject.processID, globalRunningPCBObject.memorySize, getClk()))
    {
        enqueue(globalBLockedProcessesQueue, globalRunningPCBObject);    
        return;
    }

    int forkID = fork();
    if (forkID == 0)
    {
        char processRunningTimeStr[10];
        char processIDStr[10];

        sprintf(processRunningTimeStr, "%d", globalRunningPCBObject.runningTime);
        sprintf(processIDStr, "%d", globalRunningPCBObject.processID);

        execl("./process.out", "./process.out", processIDStr, processRunningTimeStr, NULL);
        perror("execl failed");
        exit(1);
    };

    processRunning = true;

    globalRunningPCBObject.forkID = forkID;
    globalRunningPCBObject.waitingTime += getClk() - globalRunningPCBObject.interupredAt;
    globalRunningPCBObject.status = 1;

    logProcessStarted();
}

void interruptCurrentProcess()
{
    if (processRunning)
    {
        kill(globalRunningPCBObject.forkID, SIGSTOP);
        processRunning = false;
    
        printf("Process Interrupted with id: %d\n", globalRunningPCBObject.processID);

        globalRunningPCBObject.status = 2;
        globalRunningPCBObject.interupredAt = getClk();
        logProcessInterrupted();
    }
}

void resumeCurrentProcess()
{
    
    processRunning = true;

    kill(globalRunningPCBObject.forkID, SIGCONT);

    printf("Process Resumed with id: %d\n", globalRunningPCBObject.processID);
    globalRunningPCBObject.waitingTime += getClk() - globalRunningPCBObject.interupredAt;
    globalRunningPCBObject.status = 3;
    logProcessResumed();

}

void receiveProcessFromProcessGenerator(int signum)
{
    struct msgbuff message;
    bool doneReceiving = false;

    while (!doneReceiving)
    {
        int recv_val = msgrcv(msgq_id, &message, sizeof(message.data), 7, IPC_NOWAIT); // Non-blocking
        if (recv_val == -1)
        {
            if (errno == ENOMSG)
            {
                doneReceiving = true;
            }
            else
            {
                perror("Error in receive");
                return;
            }
        }
        else
        {
            ProceesControlBlock dummyPCBObject;

            dummyPCBObject.processID = message.data.id;
            dummyPCBObject.arrivalTime = message.data.arrivaltime;
            dummyPCBObject.priority = message.data.priority;
            dummyPCBObject.runningTime =message.data.runningtime;
            dummyPCBObject.memorySize = message.data.memorySize;
            dummyPCBObject.status = 0;
            dummyPCBObject.waitingTime = 0;
            dummyPCBObject.interupredAt = dummyPCBObject.arrivalTime;
            dummyPCBObject.originalPriority = dummyPCBObject.priority;

            switch (schedulingAlgorithm)
            {
                case 1:  {
                    if (!isEmptyPriorty(globalPCBpriQueue) || processRunning)
                        enqueuePriority(globalPCBpriQueue, dummyPCBObject, usePriorityPointer);
                    else
                    {
                        globalRunningPCBObject = dummyPCBObject;
                        startProcess();
                    }
                }
                    break;
                case 2: {
                    if (processRunning && dummyPCBObject.priority < globalRunningPCBObject.priority)
                    {
                        interruptCurrentProcess();

                        enqueuePriority(globalPCBpriQueue, globalRunningPCBObject, usePriorityPointer);
                        enqueuePriority(globalPCBpriQueue, dummyPCBObject, usePriorityPointer);
                    }
                    else if (isEmptyPriorty(globalPCBpriQueue) && !processRunning)
                        {
                            globalRunningPCBObject = dummyPCBObject;
                            startProcess();
                        }
                    else
                        enqueuePriority(globalPCBpriQueue, dummyPCBObject, usePriorityPointer);
                }
                    break;    
                case 3: {
                    enqueue(globalPCBQueue, dummyPCBObject);
                }
                    break;
                case 4: {
                    multiLevelEnqueue(globalPCBQueue, dummyPCBObject);
                }
                    break;
            }
        }
    }
}

void handleProcessExiting(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            processRunning = false;
            globalRunningPCBObject.status = 4;
            
            int exitCode = WEXITSTATUS(status);
            printf("Child of scheduler exited with code (id): %d\n", exitCode);

            deallocateMemory(globalRunningPCBObject.processID, globalRunningPCBObject.memoryStart, getClk());

            FinishedState finishedProcess;
            finishedProcess.waitingTime = globalRunningPCBObject.waitingTime;
            finishedProcess.WTA = (float)(getClk() - globalRunningPCBObject.arrivalTime) / (float)(globalRunningPCBObject.runningTime);

            appendToList(finishedProcessesList, finishedProcess);
            logProcessFinished(&finishedProcess);
        }
    }
}

void processGeneratorHasRunOutOfProcesses(int signum)
{
    IsProcessGeneratoroutOfProcesses = true;
    printf("Heard that the processGenerator is out of processes\n");
}

void clearResourcesScheduler(int signum)
{
    fclose(writerLogPref);

    int currentTime = getClk();
    int cpuUtilization = (currentTime - idleTime) * 100 / currentTime;

    writerLogPref = fopen("scheduler.perf", "w");
    fprintf(writerLogPref, "CPU utilization = %d%%AVG\n", cpuUtilization);

    int sumWTA = 0, sumWaitingTime = 0;
    int countOfProcesses = 0;
    while (!isEmptyList(finishedProcessesList))
    {
        FinishedState dummyFInishedState = dequeueFromList(finishedProcessesList);
        sumWTA += dummyFInishedState.WTA;
        sumWaitingTime += dummyFInishedState.waitingTime;
        countOfProcesses++;
    };
    
    fprintf(writerLogPref, "WTA = %.2f\n", (float)(sumWTA) / (float)(countOfProcesses));
    fprintf(writerLogPref, "Avg Waiting = %.2f", (float)(sumWaitingTime) / (float)(countOfProcesses));
    fclose(writerLogPref);

    printf("Scheduler terminating\n");
    destroyClk(false);
    exit(0);
}
