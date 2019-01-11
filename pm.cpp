#include <stdio.h>
#include "pm.h"
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include "semconfig.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include "mm.h"

using namespace std;

void readFirst(int semId, Record* rpf) {
    struct sembuf semDown = {0, -1, 0};
    struct sembuf semUp = {0, 1, 0};
    union semun arg; //todo remove
    printf("Read process #1 started\n");
    ifstream inFile("./input1.txt");
    string line;
    if (inFile.is_open()) {
        while (!inFile.eof()) {
            unsigned int memaddress;
            string accesstype; // R or W
            int accesstypeflag;
            //todo must be upped to limit by mm
            getline(inFile, line);
            if (!line.empty()) {
                semDown.sem_num = 0;
                semop(semId, &semDown, 1); //down parse allowance
                istringstream is(line);
                is >> std::hex >> memaddress;
                is >> accesstype;
                if (accesstype == "R") {
                    accesstypeflag = 0;
                } else {
                    accesstypeflag = 1;
                }
                //consume sharedmem
                semDown.sem_num = 4;
                semop(semId, &semDown, 1);
                rpf->process = getpid();
                rpf->address = memaddress;
                rpf->dirty = 0;
                semUp.sem_num = 2;
                semop(semId, &semUp, 1); //up parse completion flag
            }
        }
    } else {
        printf("Error opening file #1\n");
    }
}

void readSecond(int semId, Record* rps) {
    struct sembuf semDown = {0, -1, 0};
    struct sembuf semUp = {0, 1, 0};
    printf("Read process #2 started\n");
    ifstream inFile("./input2.txt");
    string line;
    if (inFile.is_open()) {
        while (!inFile.eof()) {
            unsigned int memaddress;
            string accesstype; // R or W
            int accesstypeflag;
            getline(inFile, line);
            if (!line.empty()) {
                semDown.sem_num = 1;
                semop(semId, &semDown, 1); //down parse allowance
                istringstream is(line);
                is >> std::hex >> memaddress;
                is >> accesstype;
                if (accesstype == "R") {
                    accesstypeflag = 0;
                } else {
                    accesstypeflag = 1;
                }
                //consume sharedmem
                semDown.sem_num = 5;
                semop(semId, &semDown, 1);
                rps->process = getpid();
                rps->address = memaddress;
                accesstypeflag == 0 ? rps->dirty = 0 : rps->dirty = 1;
                semUp.sem_num = 3;
                semop(semId, &semUp, 1); //up parse completion flag
            }
        }
    } else {
        printf("Error opening file #2\n");
    }
}