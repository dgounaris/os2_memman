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
    PageHashTable pageHashTable;
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
            unsigned int currentPageToLoad = rpf->address/4096;
            if (pageHashTable.contains(currentPageToLoad) && rps->dirty == 1) {
                //only update dirtyness
                pageHashTable.flagDirty(currentPageToLoad);
            } else {
                pageHashTable.put(currentPageToLoad, rpf->process, rpf->dirty);
                if (pageHashTable.contains(currentPageToLoad)) {
                    printf("Memory address stored successfully!\n");
                } else {
                    printf("Error while storing!\n");
                }
            }
            pageHashTable.remove(currentPageToLoad);
            if (!pageHashTable.contains(currentPageToLoad)) {
                printf("Memory address deleted successfully!\n");
            }
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
            semop(semId, &semDown, 1); //down parse completion to consume
            printf("%d %x %d\n", rps->process, rps->address, rps->dirty);
            unsigned int currentPageToLoad = rpf->address/4096;
            if (pageHashTable.contains(currentPageToLoad) && rps->dirty == 1) {
                //only update dirtyness
                pageHashTable.flagDirty(currentPageToLoad);
            } else {
                pageHashTable.put(currentPageToLoad, rpf->process, rpf->dirty);
                if (pageHashTable.contains(currentPageToLoad)) {
                    printf("Memory address stored successfully!\n");
                } else {
                    printf("Error while storing!\n");
                }
            }
            pageHashTable.remove(currentPageToLoad);
            if (!pageHashTable.contains(currentPageToLoad)) {
                printf("Memory address deleted successfully!\n");
            }
            //todo implement fwf algorithm
            semUp.sem_num = 5;
            semUp.sem_op = 1;
            semop(semId, &semUp, 1); //up sharedmem flag to unblock pm
        }
    }
}

PageHashNode::PageHashNode(unsigned int page, unsigned int pId, int dirty): page(page), pId(pId), dirty(dirty), next(NULL) {}

PageHashTable::PageHashTable() {
    table = new PageHashNode*[100]();
}

PageHashTable::~PageHashTable() {
    for (int i = 0; i < 100; i++) {
        PageHashNode* bucket = table[i];
        while (bucket != NULL) {
            PageHashNode* prev = bucket;
            bucket = prev->next;
            delete prev;
        }
    }
    delete [] table;
}

unsigned int PageHashTable::getHash(unsigned int page) {
    return page % 100;
}

//todo add a parameter of shared memory segment to counters for read from disk!!!
void PageHashTable::put(unsigned int page, unsigned int pId, int dirty) {
    unsigned int bucketIndex = getHash(page);
    PageHashNode* bucket = table[bucketIndex];
    if (bucket == NULL) {
        table[bucketIndex] = new PageHashNode(page, pId, dirty);
        return;
    }
    while (bucket->next != NULL) {
        bucket = bucket->next;
    }
    bucket->next = new PageHashNode(page, pId, dirty);
    printf("Reading from disk!");
}

bool PageHashTable::contains(unsigned int page) {
    unsigned int bucketIndex = getHash(page);
    PageHashNode* bucket = table[bucketIndex];
    while (bucket != NULL) {
        if (bucket->page == page) {
            return true;
        }
        bucket = bucket->next;
    }
    return false;
}

//todo add a parameter of shared memory segment to counters for write to disk!!!
void PageHashTable::remove(unsigned int page) {
    unsigned int bucketIndex = getHash(page);
    PageHashNode* bucket = table[bucketIndex];
    PageHashNode* prev = NULL;
    while (bucket != NULL) {
        if (bucket->page == page) {
            if (prev == NULL) {
                table[bucketIndex] = bucket->next;
            } else {
                prev->next = bucket->next;
            }
            if (bucket->dirty == 1) {
                printf("Writing to disk!\n");
            }
            delete bucket;
            return;
        }
    }
}

void PageHashTable::flagDirty(unsigned int page) {
    unsigned int bucketIndex = getHash(page);
    PageHashNode* bucket = table[bucketIndex];
    while (bucket != NULL) {
        if (bucket->page == page) {
            bucket->dirty = 1;
            return;
        }
        bucket = bucket->next;
    }
}