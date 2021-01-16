//
// Created by cyan on 2020/10/22.
//

#ifndef DB_RM_FILEHANDLE_H
#define DB_RM_FILEHANDLE_H


#include "../pf/pf.h"
#include "struct.h"


class RM_FileHandle {
    TableHeader table_header;
    PF_FileHandle pf_file_handle;

    int insertPage();
    unsigned getOffset(unsigned slot_num) const {
        return 8 + table_header.slot_map_size + slot_num * table_header.record_size;  //单位都是byte
    }
public:
    bool is_open = false;
    friend class RecordManager;
    friend class RM_FileScan;
    ~RM_FileHandle();

    int getRecord(const RID &rid, Record & record) const;
    int insertRecord(const char *pData, RID& rid);
    int deleteRecord(const RID &rid);
    int updateRecord(const Record &rec);
    TableHeader getHeader() {return table_header;}
};

#endif //DB_RM_FILEHANDLE_H
