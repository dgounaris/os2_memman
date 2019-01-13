#ifndef ASKISI2_HATZ_MM_H
#define ASKISI2_HATZ_MM_H

typedef struct Record {
    int process;
    unsigned int address;
    int dirty; //0 clean, 1 dirty
} Record;

typedef struct Stats {
    unsigned int readNum;
    unsigned int writeNum;
} Stats;

class PageHashNode {
public:
    unsigned int page;
    unsigned int pId; //which pid loaded this page
    int dirty; //is this page dirty?
    PageHashNode* next;
    PageHashNode(unsigned int page, unsigned int pId, int dirty);
};

class PageHashTable {
public:
    int size; //counter for the elements in the table
    int tolerated; //tolerated page faults so far, resets on every flush
    PageHashNode** table;
    unsigned int getHash(unsigned int page);
    PageHashTable();
    ~PageHashTable();
    void put(unsigned int page, unsigned int pId, int dirty, Stats* ssm);
    bool contains(unsigned int page);
    void remove(unsigned int page, Stats* ssm);
    void flagDirty(unsigned int page);
    void flush(unsigned int pId, Stats* ssm);
    void flush(Stats* ssm);
};

void memMan(int semId, Record* rpf, Record* rps, Stats* ssm, int k, int q, int memsegnum);

#endif //ASKISI2_HATZ_MM_H
