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

void memMan(int semId, Record* rpf, Record* rps, Stats* ssm, int k, int q, int memsegnum) {
    struct sembuf semDown = {0, -1, 0};
    struct sembuf semUp = {0, 1, 0};
    printf("Memory management process started\n");
    //todo need 1 or 2 hashtables?!?!?!?!
    PageHashTable firstPageHashTable;
    PageHashTable secondPageHashTable;
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
            if (firstPageHashTable.contains(currentPageToLoad)) {
                if (rpf->dirty == 1) {
                    //only update dirtyness
                    firstPageHashTable.flagDirty(currentPageToLoad);
                }
            } else {
                printf("PAGE FAULT!\n");
                //todo add a pf counter instead of size for k
                //todo check memsegmun/2 with size to remove by fifo
                firstPageHashTable.tolerated++;
                if (firstPageHashTable.size >= memsegnum/2) {
                    //todo fifo
                }
                if (firstPageHashTable.tolerated == k) {
                    firstPageHashTable.tolerated = 0;
                    firstPageHashTable.flush(ssm);
                }
                firstPageHashTable.put(currentPageToLoad, rpf->process, rpf->dirty, ssm);
                if (firstPageHashTable.contains(currentPageToLoad)) {
                    printf("Memory address stored successfully!\n");
                } else {
                    printf("Error while storing!\n");
                }
            }
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
            unsigned int currentPageToLoad = rps->address/4096;
            if (secondPageHashTable.contains(currentPageToLoad)) {
                if (rps->dirty == 1) {
                    //only update dirtyness
                    secondPageHashTable.flagDirty(currentPageToLoad);
                }
            } else {
                printf("PAGE FAULT!\n");
                secondPageHashTable.tolerated++;
                //todo add a pf counter instead of size for k
                //todo check memsegmun/2 with size to remove by fifo
                if (secondPageHashTable.size >= memsegnum/2) {
                    //todo fifo
                }
                if (secondPageHashTable.tolerated == k) {
                    secondPageHashTable.tolerated = 0;
                    secondPageHashTable.flush(ssm);
                }
                secondPageHashTable.put(currentPageToLoad, rps->process, rps->dirty, ssm);
                if (secondPageHashTable.contains(currentPageToLoad)) {
                    printf("Memory address stored successfully!\n");
                } else {
                    printf("Error while storing!\n");
                }
            }
            semUp.sem_num = 5;
            semUp.sem_op = 1;
            semop(semId, &semUp, 1); //up sharedmem flag to unblock pm
        }
    }
    //flush all the elements
    firstPageHashTable.flush(ssm);
    secondPageHashTable.flush(ssm);
}

PageHashNode::PageHashNode(unsigned int page, unsigned int pId, int dirty): page(page), pId(pId), dirty(dirty), next(NULL) {}

PageHashTable::PageHashTable(): size(0), tolerated(0) {
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

void PageHashTable::put(unsigned int page, unsigned int pId, int dirty, Stats* ssm) {
    unsigned int bucketIndex = getHash(page);
    PageHashNode* bucket = table[bucketIndex];
    if (bucket == NULL) {
        table[bucketIndex] = new PageHashNode(page, pId, dirty);
        printf("Reading from disk!\n");
        size++;
        ssm->readNum++;
        return;
    }
    while (bucket->next != NULL) {
        bucket = bucket->next;
    }
    bucket->next = new PageHashNode(page, pId, dirty);
    ssm->readNum++;
    printf("Reading from disk!\n");
    size++;
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

void PageHashTable::remove(unsigned int page, Stats* ssm) {
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
                ssm->writeNum++;
            }
            size--;
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

void PageHashTable::flush(unsigned int pId, Stats* ssm) {}

void PageHashTable::flush(Stats* ssm) {
    for (int i = 0; i < 100; i++) {
        PageHashNode *bucket = table[i];
        while (bucket != NULL) {
            PageHashNode *prev = bucket;
            bucket = prev->next;
            if (prev->dirty == 1) {
                printf("Writing to disk!\n");
                ssm->writeNum++;
            }
            delete prev;
        }
        table[i] = NULL;
    }
    printf("Flushed!\n");
    size = 0;
}