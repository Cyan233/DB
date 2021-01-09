//
// Created by cyan on 2021/1/8.
//

#include "iostream"
#include "QueryManager.h"
#include "../rm/RecordManager.h"
#include "vector"
using namespace std;


int get_infos(const string& dbname, tbinfos& info){
    PF_FileHandle info_file_handle;
    PF_Manager pf_manager;
    PF_PageHandle page_handle;
    string str = dbname + "_tableinfo";
    int rc = pf_manager.OpenFile(str.c_str(), info_file_handle);
    TEST_RC_NOT_ZERO_ERROR
    rc = info_file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    PageNum page1;
    page_handle.GetPageNum(page1);
    TEST_RC_NOT_ZERO_ERROR
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    info = *reinterpret_cast<tbinfos *>(wpdata);
}


int QueryManager::Insert(const string& dbname, const vector<vector<value>>& records){
    RM_FileHandle file_handle;
    int rc = RecordManager::getInstance().openFile(dbname, file_handle);
    TEST_RC_NOT_ZERO_ERROR
    tbinfos info;
    get_infos(dbname, info);
    RID rid;
    for (auto &rec: records) {
        char data[info.recordsize];
        int offset=0;
        for (int i=0;i<rec.size();i++){
            switch (info.colattr[i]){
                case AttrType::DATE:
                case AttrType::INT:
                    memcpy(data+offset, &rec[i].i, sizeof(int));
                    offset+=4;
                    break;
                case AttrType::FLOAT:
                    memcpy(data+offset, &rec[i].f, sizeof(float));
                    offset+=4;
                    break;
                case AttrType::VARCHAR:
                    memcpy(data+offset, rec[i].s, info.attrsize[i]);
                    offset+=info.attrsize[i];
                    break;
            }
        }
        file_handle.insertRecord(data, rid);
    }
    return 0;
}