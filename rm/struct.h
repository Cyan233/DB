//
// Created by cyan on 2020/10/22.
//

#ifndef DB_STRUCT_H
#define DB_STRUCT_H

#include <cstring>
#include <cstdio>
#include "../sm/SM_type.h"
#include <vector>
using namespace std;

struct RID{  // record_id
    long pageID = 0;
    unsigned slotID = 0;
    explicit RID(unsigned pi=0, unsigned si=0) : pageID(pi), slotID(si) {}
};


struct Record{
    char * data;
    RID rid;

    Record() : data(nullptr) { }
    void setData(const char * _data, unsigned _size, const RID & _rid) {
        rid = _rid;
        delete[] data;
        data = new char[_size];
        memcpy(data, _data, _size);
    }
    ~Record() {
        delete[] data;
    }
};


struct TableHeader {  //整张表的特征信息，存储在第一页
    int first_spare_page = -1;  //>0时表示空闲的页的id，否则表示没有空闲页
    unsigned page_num = 1;
    unsigned record_num = 0;
    unsigned record_size;    //单位是Byte
    unsigned slot_per_page;
    unsigned slot_map_size;   //单位是Byte
    bool is_modified = false;

    tbinfos tb_info;
};

struct value{
    int i;
    float f;
    char s[100];
};

struct Col{  //比如 teacher.name
    char tb_name[100];
    char attr_name[100];
};

struct Condition{  //比如 teacheer.name < 100
    CompOp compOp;
    char *col;
    value compare_value;
};

struct Condition_joint{  //比如 teacher.id = student.teacher_id
    CompOp compOp;
    Col cols[2];
};

struct SetClause{  //比如 a = 100
    char attr_name[100];
    value set_value;
};


#endif //DB_STRUCT_H
