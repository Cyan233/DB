//
// Created by cyan on 2020/10/24.
//

#ifndef DB_RM_FILESCAN_H
#define DB_RM_FILESCAN_H

#include <string>
#include "RM_FileHandle.h"
#include "../utils/MyBitMap.h"
#include "../parser/Expr.h"

class RM_FileHandle;

class RM_FileScan {
    RM_FileHandle* file_handle;
    RID curRID;
    MyBitMap bitmap = MyBitMap(0, 1);
    char *bitdata = nullptr;
    Expr *condition;
    string tableName;
public:
    int startScan(RM_FileHandle *_fileHandle, Expr *_condition, const std::string &tableName);
    int openScan(RM_FileHandle *fileHandle, AttrType attrType, int attrLength, int attrOffset, CompOp compOp, void *value);
    int getNextRecord(Record& record);
    ~RM_FileScan();
};


#endif //DB_RM_FILESCAN_H
