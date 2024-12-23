#include "headers.h"
#include "proceess_generator_data_structures.h"
#include <signal.h>
#include <string.h>

void clearResourcesProcessGenerator(int SIGNUM);
void handleChildrenExiting (int signum);
bool skipLine(FILE *filePtr);
bool intializeProcessFromInput(FILE *filePtr, struct ProcessData* process);
bool sendProcessToScheduler(int msgq_id, struct  ProcessData* process);

// Global variables are only used in main and in clearResources
int msgq_id;
Queue* globalProcessesQueue;
bool hasSchedulerExited;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResourcesProcessGenerator);
    signal(SIGCHLD, handleChildrenExiting);

    // TODO Initialization
    // 1. Read the input files.

    char* filepath = argv[1];
    FILE *filePtr;
    filePtr = fopen(filepath, "r");

    if (skipLine(filePtr) == true)
    {
        return EXIT_FAILURE;
    }
    
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.

    int schedulingAlgorithm_schedulerID = atoi(argv[3]);
    int roundRobinQuantum = -1;

    switch (schedulingAlgorithm_schedulerID)
    {
        case 1: printf("Process Generator detected: Shortest Job First \n");
            break;
        case 2: printf("Process Generator detected: Preemptive Highest Priority First \n");
            break;    
        case 3: {
                if (argc == 6 && schedulingAlgorithm_schedulerID)
                {
                    roundRobinQuantum = atoi(argv[5]);
                    printf("Process Generator detected: Round robin with quantum =  %d\n", roundRobinQuantum);
                }
                else {
                    perror("Process Generator detected: No quantum given error!\n");
                    return EXIT_FAILURE;
                 }
            }
            break;
        case 4: {
                if (argc == 6 && schedulingAlgorithm_schedulerID)
                {
                    roundRobinQuantum = atoi(argv[5]);
                    printf("Process Generator detected: Multiple level Feedback Loop with quantum =  %d\n", roundRobinQuantum);
                }
                else {
                    perror("Process Generator detected: Multiple level Feedback Loop No quantum given error!\n");
                    return EXIT_FAILURE;
                 }
            }
            break;
        default: perror("Wrong scheduling algortihm sincerly process generator \n");
            return EXIT_FAILURE;
    }
    
    // 3. Initiate and create the scheduler and clock processes.

    int clockProcessID = fork();

    if (clockProcessID == 0)
    {
        execl("./clk.out", "./clk.out", NULL);

        perror("execl failed");
        exit(1);
    } else {
        int schedulerProcessID =  fork();
        if (schedulerProcessID == 0)
        {
            char schedulingAlgorithmStr[10];
            char roundRobinQuantumStr[10];
            sprintf(schedulingAlgorithmStr, "%d", schedulingAlgorithm_schedulerID);
            sprintf(roundRobinQuantumStr, "%d", roundRobinQuantum);
            
            execl("./scheduler.out", "./scheduler.out", schedulingAlgorithmStr, roundRobinQuantumStr, NULL);

            perror("execl failed");
            exit(1);
        };
        schedulingAlgorithm_schedulerID = schedulerProcessID;
    };
    
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int currentTime = getClk();
    printf("Current Time is %d\n", currentTime);

    key_t key_id;

    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    ProcessData dummyProcess;
    int countOfProcesses = 0;
    
    Queue processQueue;
    initializeQueue(&processQueue);
    globalProcessesQueue = &processQueue;

    while (intializeProcessFromInput(filePtr, &dummyProcess))
    {
        enqueueQ(globalProcessesQueue, dummyProcess);
        countOfProcesses++;
    }

    fclose(filePtr);
    // 6. Send the information to the scheduler at the appropriate time.
    bool doneSending = false;
    currentTime = getClk();

    dummyProcess = dequeueQ(globalProcessesQueue);

    while (!isEmptyQ(globalProcessesQueue) || dummyProcess.arrivaltime != -1)
    {
        while (getClk() != currentTime + 1);
        
        doneSending = false;

        currentTime = getClk();
        printf("Current Time is %d\n", currentTime);

        while (dummyProcess.arrivaltime == currentTime)
        {
            sendProcessToScheduler(msgq_id, &dummyProcess);
            dummyProcess = dequeueQ(globalProcessesQueue);
            doneSending = true;
        }

        if (doneSending)
        {
            doneSending = false;

            kill(schedulingAlgorithm_schedulerID, SIGUSR1);
        }
    }

    printf("Out of processes\n");

    kill(schedulingAlgorithm_schedulerID, SIGUSR2); //signal that we are out of processes

    hasSchedulerExited = false;

    currentTime = getClk();
    while(!hasSchedulerExited)
    {
        while (getClk() <= currentTime);
        
        currentTime = getClk();
        printf("Current Time is %d\n", currentTime);
    }

    printf("Thank you very much, Operating Systems project done Exiting\n");
    raise(SIGINT);
    // 7. Clear clock resources
    destroyClk(true);
};

void clearResourcesProcessGenerator(int signum)
{
    //TODO Clears all resources in case of interruption
    destroyClk(true);

    struct msqid_ds ctrl_status_ds;
    msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
    ctrl_status_ds.msg_perm.mode = 0666;
    msgctl(msgq_id, IPC_SET, &ctrl_status_ds);

    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);

    while(isEmptyQ(globalProcessesQueue) == false)
        dequeueQ(globalProcessesQueue);

    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
    exit(0);
};

void handleChildrenExiting (int signum)
{
    hasSchedulerExited = true;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

bool skipLine(FILE *filePtr)
{
    if (!filePtr) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), filePtr) == NULL) {
        perror("Error reading the file");
        fclose(filePtr);
        return EXIT_FAILURE;
    }
    return false;
};

bool intializeProcessFromInput(FILE *filePtr, ProcessData* process)
{
    if ( fscanf(
        filePtr, 
        "%d %d %d %d %d",

        &process->id,
        &process->arrivaltime, 
        &process->runningtime,
        &process->priority,
        &process->memorySize
    ) != 5)
    {
        return false;
    }
    return true;
}

bool sendProcessToScheduler(int msgq_id,  ProcessData* process)
{   
    struct msgbuff message;
    message.mtype = 7;
    message.data = *process;

    int send_val = msgsnd(msgq_id, &message, sizeof(message.data), !IPC_NOWAIT);

    if (send_val == -1){
        perror("Errror in send");
        return false;
    }
    return true;
}