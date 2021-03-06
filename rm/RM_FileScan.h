//
// Created by cyan on 2021/1/9.
//

#ifndef DB_RM_FILESCAN_H
#define DB_RM_FILESCAN_H


#include "RecordManager.h"
#include "../utils/MyBitMap.h"

class RM_FileHandle;

class RM_FileScan {
    RM_FileHandle* file_handle;
    RID curRID;
    MyBitMap bitmap = MyBitMap(0, 1);
    char *bitdata = nullptr;
    vector<Condition> *conditions;
public:
    int startScan(RM_FileHandle *_file_handle, vector<Condition> *_conditions);
    int getNextRecord(Record& record);
    ~RM_FileScan() { delete[] bitdata ;}
};

bool satisfy(const vector<Condition> &conditions, char* data, tbinfos tb_info);

#endif //DB_RM_FILESCAN_H
