//
// Created by cyan on 2020/10/21.
//

#ifndef DB_RECORDMANAGER_H
#define DB_RECORDMANAGER_H

#include "../pf/pf.h"
#include <string>
#include "RM_FileHandle.h"
using namespace std;


class RecordManager {
    PF_Manager & pf_manager;
    RecordManager() : pf_manager(PF_Manager::getInstance()) { }
public:
    int createFile (const string& filename, unsigned record_size, int nullable_num);   //record_size单位为Byte
    int destroyFile(const string& filename);
    int openFile(const string& filename, RM_FileHandle &file_handle);
    int closeFile(RM_FileHandle& file_handle);
    static RecordManager& getInstance() ;
};

#endif //DB_RECORDMANAGER_H
