#ifndef ASKISI2_HATZ_MM_H
#define ASKISI2_HATZ_MM_H

typedef struct Record {
    int process;
    unsigned int address;
    int dirty; //0 clean, 1 dirty
} Record;

class PageHashNode {
public:
    unsigned int page;
    PageHashNode* next;
    PageHashNode(unsigned int page);
};

class PageHashTable {
    PageHashNode** table;
    unsigned int getHash(unsigned int page);
    PageHashTable();
    ~PageHashTable();
    void put(unsigned int page);
    bool contains(unsigned int page);
    void remove(unsigned int page);
};

void memMan(int semId, Record* rpf, Record* rps);

#endif //ASKISI2_HATZ_MM_H
