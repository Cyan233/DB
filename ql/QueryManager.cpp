//
// Created by cyan on 2021/1/8.
//

#include "iostream"
#include "QueryManager.h"
#include "../rm/RecordManager.h"
#include "../rm/RM_FileScan.h"
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
    rc = get_infos(dbname, info);
    TEST_RC_NOT_ZERO_ERROR
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

int QueryManager::Delete(const string& dbname, vector<Condition> &where_conditions){
    RM_FileHandle file_handle;
    int rc = RecordManager::getInstance().openFile(dbname, file_handle);
    TEST_RC_NOT_ZERO_ERROR
    tbinfos info;
    get_infos(dbname, info);
    Record record;
    RM_FileScan scan;
    scan.startScan(&file_handle, &where_conditions);
    while(scan.getNextRecord(record)==0){  //找得到记录
        rc = file_handle.deleteRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
    }
    return 0;
}

int QueryManager::Update(const string& dbname, vector<Condition> &where_conditions, vector<SetClause>& set_clauses){
    RM_FileHandle file_handle;
    int rc = RecordManager::getInstance().openFile(dbname, file_handle);
    TEST_RC_NOT_ZERO_ERROR
    tbinfos tb_info;
    get_infos(dbname, tb_info);
    Record record;
    RM_FileScan scan;
    scan.startScan(&file_handle, &where_conditions);
    while(scan.getNextRecord(record)==0){  //找得到记录，更改其值
        for(auto& set_clause:set_clauses){  //每一列赋值
            int offset = 0;
            for(int i=0; i<tb_info.columns; i++) {  //查找该set等式对应的列
                if(strcmp(set_clause.attr_name, tb_info.attrname[i])==0){
                    switch (tb_info.colattr[i]){
                        case AttrType::DATE :
                        case AttrType::INT :
                            memcpy(record.data+offset, &set_clause.set_value.i, sizeof(int));
                            break;
                        case AttrType::FLOAT :
                            memcpy(record.data+offset, &set_clause.set_value.f, sizeof(float));
                            break;
                        case AttrType::VARCHAR :
                            memcpy(record.data+offset, &set_clause.set_value.s, tb_info.attrsize[i]);
                            break;
                    }
                    break;
                }
                offset += tb_info.attrsize[i];
            }
        }
        rc = file_handle.updateRecord(record);
        TEST_RC_NOT_ZERO_ERROR
    }
    return 0;
}

int QueryManager::Select(vector<string>& dbnames, vector<Condition_joint> &where_conditions, vector<Col>& selector){
    // SELECT selector FROM dbnames WHERE where_conditions

}
