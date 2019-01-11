#ifndef ASKISI2_HATZ_SEMCONFIG_H
#define ASKISI2_HATZ_SEMCONFIG_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

union semun {
    int val;
    struct semid_ds *bif;
    unsigned short *array;
};

#endif //ASKISI2_HATZ_SEMCONFIG_H
