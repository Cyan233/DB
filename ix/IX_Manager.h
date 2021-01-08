//IX_Manager.h
//
//create by lmm 2020/11/15
//
//

#ifndef _IX_MANAGER_H
#define _IX_MANAGER_H

#include "../pf/pf.h"
#include "IX_IndexHandle.h"

struct IX_IndexHeader{
    const char* filename;
    AttrType attrType;
    int attrLength;
    PageNum rNpN;//root page number
    int offset;
    bool is_new;
    int pagenums;
};

class IX_Manager {
private:
    PF_Manager& pf_manager;
    IX_Manager() : pf_manager(PF_Manager::getInstance()) { }
public:
    RC CreateIndex(const char* filename, int indexnum, AttrType attrType, int attrLength, int offset);
    RC DestroyIndex(const char* filename, int indexnum);
    RC OpenIndex(const char* filename, int indexnum, IX_indexHandle& ix_indexhandle);
    RC CloseIndex(IX_indexHandle& ix_indexhandle);
    static IX_Manager& GetInstance();
};

#endif //_IX_MANAGER_H