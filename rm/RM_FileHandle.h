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
    unsigned getOffset(unsigned slot_num) const;
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
