#include <stdio.h>
#include "mm.h"
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include "semconfig.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>

void memMan(int semId, Record* rpf, Record* rps) {
    struct sembuf semDown = {0, -1, 0};
    struct sembuf semUp = {0, 1, 0};
    int q = 2;
    printf("Memory management process started\n");
    int i, j;
    for (i = 0; i < 3; i++) {
        semUp.sem_num = 0;
        semUp.sem_op = 2;
        semop(semId, &semUp, 1); //up parse allowance to limit
        for (j = 0; j < q; j++) {
            semDown.sem_num = 2;
            semDown.sem_op = -1;
            semop(semId, &semDown, 1); //down parse completion to consume
            printf("%d %x %d\n", rpf->process, rpf->address, rpf->dirty);
            //todo implement hashing
            //todo implement fwf algorithm
            semUp.sem_num = 4;
            semUp.sem_op = 1;
            semop(semId, &semUp, 1); //up sharedmem flag to unblock pm
        }
        semUp.sem_num = 1;
        semUp.sem_op = 2;
        semop(semId, &semUp, 1); //up parse allowance to limit
        for (j = 0; j < q; j++) {
            semDown.sem_num = 3;
            semDown.sem_op = -1;
            semop(semId, &semDown, 1); //up parse allowance to limit
            printf("%d %x %d\n", rps->process, rps->address, rps->dirty);
            //todo implement hashing
            //todo implement fwf algorithm
            semUp.sem_num = 5;
            semUp.sem_op = 1;
            semop(semId, &semUp, 1); //up sharedmem flag to unblock pm
        }
    }
}