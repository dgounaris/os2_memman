#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include "semconfig.h"
#include "pm.h"
#include "mm.h"

void initSems(int semId);

int main(int argc, char *argv[]) {
    int k = 2;
    int q = 2;
    int memsegnum = 10;
    int memsegsize = 4096;
    if (argc == 4) {
        k = atoi(argv[1]);
        memsegnum = atoi(argv[2]);
        q = atoi(argv[3]);
    }
    int numOfSemaphores = 6;
    int semId = semget((key_t) 222, numOfSemaphores, IPC_CREAT | 0660);
    struct sembuf semDown = {0, -1, 0};
    struct sembuf semUp = {0, 1, 0};
    initSems(semId);
    //create shared memory segments
    int readProcessFirst = shmget((key_t) 801, sizeof(Record), IPC_CREAT | 0666);
    Record* rpf = (Record*)shmat(readProcessFirst, NULL, 0);
    int readProcessSecond = shmget((key_t) 802, sizeof(Record), IPC_CREAT | 0666);
    Record* rps = (Record*)shmat(readProcessSecond, NULL, 0);
    int statsMemory = shmget((key_t) 803, sizeof(Stats), IPC_CREAT | 0666);
    Stats* ssm = (Stats*)shmat(statsMemory, NULL, 0);
    if (fork() == 0) {
        readFirst(semId, rpf);
        return 0;
    }
    if (fork() == 0) {
        readSecond(semId, rps);
        return 0;
    }
    if (fork() == 0) {
        memMan(semId, rpf, rps, ssm);
        return 0;
    }

    int status;
    while (wait(&status) > 0); //wait for all children to finish
    wait(&status);

    int i;
    for (i = 0; i < numOfSemaphores; i++) {
        semctl(semId, i, IPC_RMID, 0);
    }

    shmdt(rpf); shmdt(rps); shmdt(ssm);
    shmctl(readProcessFirst, 0, IPC_RMID);
    shmctl(readProcessSecond, 0, IPC_RMID);
    shmctl(statsMemory, 0, IPC_RMID);
}

void initSems(int semId) {
    union semun arg;
    int i = 0;
    //first state parse allowance
    arg.val = 0;
    for (i = 0; i < 2; i++) {
        semctl(semId, i, SETVAL, arg);
    }
    //first state parsed flag
    arg.val = 0;
    for (i = 2; i < 4; i++) {
        semctl(semId, i, SETVAL, arg);
    }
    //shared mem flag
    arg.val = 1;
    for (i = 4; i < 6; i++) {
        semctl(semId, i, SETVAL, arg);
    }
}