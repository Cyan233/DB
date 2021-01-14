//
// Created by cyan on 2021/1/8.
//

#include "iostream"
#include "QueryManager.h"
#include "../rm/RecordManager.h"
#include "../rm/RM_FileScan.h"
#include "vector"
#include "set"
using namespace std;


int get_infos(const string& tbname, tbinfos& info){
    PF_FileHandle info_file_handle;
    PF_Manager pf_manager;
    PF_PageHandle page_handle;
    string str = tbname + "_tableinfo";
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


int QueryManager::Insert(const string& tbname, const vector<vector<value>>& records){
    RM_FileHandle file_handle;
    int rc = RecordManager::getInstance().openFile(tbname, file_handle);
    TEST_RC_NOT_ZERO_ERROR
    tbinfos info;
    rc = get_infos(tbname, info);
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

int QueryManager::Delete(const string& tbname, vector<Condition> &where_conditions){
    RM_FileHandle file_handle;
    int rc = RecordManager::getInstance().openFile(tbname, file_handle);
    TEST_RC_NOT_ZERO_ERROR
    tbinfos info;
    get_infos(tbname, info);
    Record record;
    RM_FileScan scan;
    scan.startScan(&file_handle, &where_conditions);
    while(scan.getNextRecord(record)==0){  //找得到记录
        rc = file_handle.deleteRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
    }
    return 0;
}

int QueryManager::Update(const string& tbname, vector<Condition> &where_conditions, vector<SetClause>& set_clauses){
    RM_FileHandle file_handle;
    int rc = RecordManager::getInstance().openFile(tbname, file_handle);
    TEST_RC_NOT_ZERO_ERROR
    tbinfos tb_info;
    get_infos(tbname, tb_info);
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

void printSelect(Record& record, const vector<Col>& selector, tbinfos tb_info){
    for (auto &select:selector) {
        int offset = 0;
        for (int i = 0; i < tb_info.columns; i++) {  //输出需要对的列的值
            if (strcmp(select.attr_name, tb_info.attrname[i]) == 0) {
                switch (tb_info.colattr[i]) {
                    case AttrType::DATE :
                    case AttrType::INT :
                        cout << *reinterpret_cast<int *>(record.data + offset) << ", ";
                        break;
                    case AttrType::FLOAT :
                        cout << *reinterpret_cast<float *>(record.data + offset) << ", ";
                        break;
                    case AttrType::VARCHAR :
                        printf("%.*s\n", tb_info.attrsize[i], record.data + offset);
                        break;
                }
            }
            offset += tb_info.attrsize[i];
        }
    }
}

int jointSelect(int level, vector<Record> path, const vector<Condition_joint> &conditions_joint, set<Record>* records,
        const vector<vector<Col>>& selector, const tbinfos* tbinfos, const vector<char*>& tbnames){
    if(level==0){
        for(int i=0;i<path.size();i++){
            printSelect(path[i], selector[i], tbinfos[i]);
        }
    }
    Condition new_codition;
    int changed_col = 0;
    for(auto& record: records[level]){  // 挑选一条，与其他表进行联合
        path.push_back(record);
        level--;
        set<Record> new_records;
        set<Record> old_records;
        for(auto& condition: conditions_joint){ //更新比对条件
            if(strcmp(condition.cols[1].tb_name, tbnames[level]) == 0 || strcmp(condition.cols[0].tb_name,tbnames[level])==0){
                int match = 0;
                if(strcmp(condition.cols[1].tb_name, tbnames[level]) == 0 ){
                    new_codition.compOp = condition.compOp;
                    match = 1;
                } else{
                    new_codition.compOp = switchLeftRight(condition.compOp);
                    match = 0;
                }
                new_codition.col = condition.cols[!match];
                int offset = 0;
                for (int i = 0; i < tbinfos[level].columns; i++) {  // 找到需要替换的值
                    if (strcmp(condition.cols[match].attr_name, tbinfos[level].attrname[i]) == 0) {
                        switch (tbinfos[level].colattr[i]) {
                            case AttrType::DATE :
                            case AttrType::INT :
                                new_codition.compare_value.i = *reinterpret_cast<int *>(record.data + offset);
                                break;
                            case AttrType::FLOAT :
                                new_codition.compare_value.f = *reinterpret_cast<float *>(record.data + offset);
                                break;
                            case AttrType::VARCHAR :
                                new_codition.compare_value.s = record.data + offset;
                                break;
                        }
                        break;
                    }
                    offset += tbinfos[level].attrsize[i];
                }
                // 筛选这一条件的相连的表的结果
                for (int i = 0; i < tbnames.size(); i++) {
                    if(strcmp(tbnames[i], new_codition.col.tb_name)==0){
                        changed_col = i;
                        old_records = records[i];
                        vector<Condition> new_vec;
                        new_vec.push_back(new_codition);
                        for(auto& rec:records[i]){
                            if(satisfy(new_vec, rec.data, tbinfos[i])){
                                new_records.insert(rec);
                            }
                        }
                        break;
                    }
                }
            }
        }
        records[changed_col] = new_records;
        jointSelect(level-1, path, conditions_joint, records, selector, tbinfos, tbnames);
        records[changed_col] = old_records;
        level++;
        path.pop_back();
    }
    return 0;
}

int QueryManager::Select(vector<char*>& tbnames, vector<vector<Col>>& selector,   // selecotr[i]是tbnames[i]里的列
        vector<Condition_joint> &conditions_joint, vector<vector<Condition>> &conditions) {  //conditions也是按表分开的，没有条件的就是空vector
    // SELECT selector FROM tbnames WHERE where_conditions
    int rc;
    RM_FileHandle file_handle[tbnames.size()];
    tbinfos tb_info[tbnames.size()];
    for (int i = 0; i < tbnames.size(); i++) {
        rc = RecordManager::getInstance().openFile(tbnames[i], file_handle[i]);
        TEST_RC_NOT_ZERO_ERROR
        tb_info[i] = file_handle[i].getHeader().tb_info;
    }
    Record record;
    RM_FileScan scan;
    // 1.如果只有单表条件conditions，说明只有一个数据表
    scan.startScan(&file_handle[0], &conditions[0]);
    while (scan.getNextRecord(record) == 0) {  //找得到记录，更改其值
        printSelect(record, selector[0], tb_info[0]);
        cout<<endl;
    }
    // 2.存在多表联合的条件
    auto* records = new set<Record>[tbnames.size()];
    // 先比较单表的条件，找到每个表里符合条件的RID
    for  (int i = 0; i < tbnames.size(); i++) {
        scan.startScan(&file_handle[i], &conditions[i]);
        while (scan.getNextRecord(record) == 0) {  //找得到记录，更改其值
            records[i].insert(record);
        }
    }
    // 对于每个多表条件，递归查找
    vector<Record> path;
    jointSelect(tbnames.size(), path, conditions_joint, records, selector, tb_info, tbnames);
    return 0;
}
