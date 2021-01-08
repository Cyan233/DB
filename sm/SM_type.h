//SM_type.h
//
//create by lmm 2020/11/30
//
//
#ifndef SM_TYPE_H
#define SM_TYPE_H
#include"../Constants.h"

struct dbinfos{
    char dbname[100];
    int strsize;
    //todo add 现有的所有表及其模式信息
};//dbinfo页的结构：前4字节记录记录条数 后面是各条dbinfo

struct whole_tbinfos{
    char tbname[100];
    int strsize;
    //todo add more things
};//tbinfo页的结构：前4字节记录记录条数 后面是各条tbinfo

struct tbinfos{
    char tbname[100];
    int strsize;
    int recordsize;

    //need to get follow parameters:
    int columns = 0;//<=10
    AttrType colattr[10];
    char attrname[10][100];
    int attrsize[10];
    bool has_default[10];//false no true yes
    //todo:more good writing
    //for m default
    int coldi[10];
    float coldf[10];
    string coldm[10];
    int length_limit[10];//-1for null >0for has_limit

    //can be null or not
    bool cannull[10];//yes can be no cannot
    //从低位到高位 分别对应columns0-9 如果是_key的组成部分则为1 否则为0
    bool has_pkey = false;//false no true yes
    int primary_key;//可以先调用find_spec 如果找不到就insert 否则return error
    int fkey_num;//0-10
    int foreign_key[10];
    char reference[10][100];
    int rnsize[10];
};//table的结构，单独成文件

#endif //!SM_TYPE_H