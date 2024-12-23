#include "headers.h"

void clearResourcesProcess(int signum);
void correctCurrentTime(int signum);

int processID;
int currentTime;

int main(int agrc, char *argv[])
{
    //TODO The process needs to get the remaining time from somewhere
    initClk();

    signal(SIGINT, clearResourcesProcess);
    signal(SIGCONT, correctCurrentTime);

    processID = atoi(argv[1]);
    int remainingtime = atoi(argv[2]);
    int tikTok = remainingtime;
    currentTime = getClk();

    printf("Process with id:%d started: remainging time = %d\n", processID, remainingtime);
    for (int i = 0; i < tikTok; i++)
    {
        while (getClk() <= currentTime);

        currentTime = getClk();
        remainingtime--;
        printf("id:%d, remaining time = %d\n", processID, remainingtime);
    }
    
    raise(SIGINT);
    return 0;
}

void correctCurrentTime(int signum)
{
    currentTime = getClk();
}

void clearResourcesProcess(int signum)
{
    printf("Child process of scheduler with id: %d exiting\n", processID);
    destroyClk(false);
    exit(processID);
}