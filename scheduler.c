#include "scheduler_algorithms.h"

int main(int argc, char *argv[])
{
    initClk();

    IsProcessGeneratoroutOfProcesses = false;
    processRunning = false;
    idleTime = 0;

    key_t key_id;
    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    signal(SIGINT, clearResourcesScheduler);
    signal(SIGUSR1, receiveProcessFromProcessGenerator);
    signal(SIGUSR2, processGeneratorHasRunOutOfProcesses);
    signal(SIGCHLD, handleProcessExiting);
    
    writerLogPref = fopen("scheduler.log", "w");
    fprintf(writerLogPref, "#id arrival runtime priority\n");
    
    wirterMemoryLog = fopen("memory.log", "w");
    fprintf(wirterMemoryLog, "#At time x allocated y bytes for process z from i to j\n");

    LinkedListFinishedState finishedList;
    initList(&finishedList);
    finishedProcessesList = &finishedList;

    initializeMemory();

    Queue BlockedProcessesQueue;
    globalBLockedProcessesQueue = &BlockedProcessesQueue;
    initQueue(globalBLockedProcessesQueue);

    schedulingAlgorithm = atoi(argv[1]);
    int roundRobinQuantum = -1;

    switch (schedulingAlgorithm)
    {
        case 1: {
                printf("Scheduler detected: Shortest Job First \n");

                bool usePriority = false;
                usePriorityPointer = &usePriority;

                shortestJobFirstAlgorithm();
            }
            break;
        case 2: {
                printf("Scheduler detected: Preemptive Highest Priority First \n");

                bool usePriority = true;
                usePriorityPointer = &usePriority;

                prememtiveHigheestPriorityFirstAlgorithm();
            }
            break;    
        case 3: {
                if (argc == 3 && schedulingAlgorithm)
                {
                    roundRobinQuantum = atoi(argv[2]);
                    printf("Scheduler detected: Round robin with quantum =  %d \n", roundRobinQuantum);
                    roundRobinAlgorithm(&roundRobinQuantum);
                }
                else {
                    perror("Scheduler detected: No round robin quantum given error!");
                    return EXIT_FAILURE;
                }
            }
            break;
        case 4: {
                if (argc == 3 && schedulingAlgorithm)
                {
                    roundRobinQuantum = atoi(argv[2]);
                    printf("Scheduler detected: Multiple level Feedback Loop Round robin with quantum =  %d \n", roundRobinQuantum);
                    multiLevelFeedbackLoopAlgorithm(&roundRobinQuantum);
                }
                else {
                    perror("Scheduler detected: No round robin quantum given error!");
                    return EXIT_FAILURE;
                }
            }
            break;

        default: {
                perror("Wrong schedling algortihm sincerly the scheduler \n");
                return EXIT_FAILURE;
            }
    }

    printf("Scheduler is out of processes\n");
    raise(SIGINT);
};
