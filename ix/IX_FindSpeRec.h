//IX_FindSpeRec.h
//
//create by lmm 2020/11/16
//
//
#ifndef _IX_FINDSPEREC_H
#define _IX_FINDSPEREC_H

#include "IX_IndexHandle.h"
#include "../pf/pf.h"

class IX_FindSpeRec{
public:
    IX_FindSpeRec();
    ~IX_FindSpeRec();
    RC OpenScan(IX_indexHandle &indexhandle, CompOp compOp, const void *value);
    RC FindNextRec(IX_indexHandle &indexhandle, RID& rid);
private:
    bool Compare(RID& rid);
    int find_position(IX_indexHandle &indexhandle, PageNum fatherPage);
    PageNum p_current;
    int current_index;
    CompOp comp;
    const void *findvalue;
};

#endif //_IX_FINDSPEREC_H