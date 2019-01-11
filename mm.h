#ifndef ASKISI2_HATZ_MM_H
#define ASKISI2_HATZ_MM_H

typedef struct Record {
    int process;
    unsigned int address;
    int dirty; //0 clean, 1 dirty
} Record;

void memMan(int semId, Record* rpf, Record* rps);

#endif //ASKISI2_HATZ_MM_H
