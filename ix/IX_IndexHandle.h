//IX_indexHandle.h
//
//create by lmm 2020/11/15
//
//
#ifndef _IX_INDEXHANDLE_H
#define _IX_INDEXHANDLE_H

#include "../rm/struct.h"
#include "../pf/pf.h"

class IX_indexHandle {
public:
    RC InsertRecord(const RID& rid);
    RC DeleteRecord(const RID& rid);
    PF_FileHandle pf_filehandle;
    PageNum rootpage;
    bool fisnew;
    RID ridd;//上溢产生的新标值
    PageNum topt;//上溢产生的新页面
    //新标值与新页面不对应 新页面对应于新标值的后一位
    int pagenums;

private:
    int find_position_insert(const RID& rid, PageNum fatherPage);
    int find_position_delete(const RID& rid, PageNum fatherPage);
    int Indexcompare(RID rid1, RID rid2);//<0:rid1 is in the left or  = rid2  otherwise >0
    int valuecompare(RID rid1, RID rid2);
};

#endif //!_IX_INDEXHANDLE_H