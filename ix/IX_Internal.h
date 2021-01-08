//IX_Internal.h
//
//create by lmm 2020/11/16
//
//
#ifndef _IX_INTERNAL_H
#define _IX_INTERNAL_H

#include"../pf/pf.h"
#include"../rm/struct.h"
#include"../Constants.h"

static const int BN = 10;//请设置为偶说

struct internode{
    int size = 0;//已经存储了size个数据
    RID interdata[BN];//每个interdata是对应son中的最大元素
    PageNum sonpointer[BN];
};

struct leafnode{
    int size = 0;
    RID leafdata[BN];
    PageNum leftpage, rightpage;//to previous and next leafnode
};

struct nodetype{
    int nodet;//0 for internode 1 for leafnode
};

#endif //!_IX_INTERNAL_H