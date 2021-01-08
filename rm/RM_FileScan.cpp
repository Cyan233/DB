//
// Created by cyan on 2020/10/24.
//

#include "RM_FileScan.h"
#include "RecordManager.h"


RM_FileScan::~RM_FileScan(){
    delete[] bitdata;
}

int RM_FileScan::startScan(RM_FileHandle *_file_handle, Expr *_condition, const string &_tableName) {
    file_handle = _file_handle;
    curRID.pageID = 1;   //当前的页
    curRID.slotID = 0;   //当前的槽
    condition = _condition;
    delete[] bitdata;
    bitdata = new char[file_handle->table_header.slot_map_size];
    tableName = _tableName;
    return 0;
}

int RM_FileScan::openScan(RM_FileHandle *fileHandle, AttrType attrType, int attrSize, int attrOffset,
        CompOp compOp, void *value) {  //判断条件
    Expr *_condition = nullptr;
    if (compOp != CompOp::NO_OP) {
        Expr *left = new Expr();
        left->attrInfo.attrSize = attrSize;
        left->attrInfo.attrOffset = attrOffset;
        left->attrInfo.attrType = attrType;
        left->attrInfo.notNull = false;
        left->nodeType = NodeType::ATTR_NODE;

        Expr *right;
        if (value == nullptr || attrType==AttrType::VARCHAR || attrType==AttrType::NO_ATTR)
            right = new Expr();
        else {
            switch (attrType) {
                case AttrType::INT:
                case AttrType::DATE: {
                    int i = *reinterpret_cast<int *>(value);
                    right = new Expr(i);
                    break;
                }
                case AttrType::FLOAT: {
                    float f = *reinterpret_cast<float *>(value);
                    right = new Expr(f);
                    break;
                }
                case AttrType::BOOL: {
                    bool b = *reinterpret_cast<bool *>(value);
                    right = new Expr(b);
                    break;
                }
                case AttrType::STRING: {
                    char *s = reinterpret_cast<char *>(value);
                    right = new Expr(s);
                    break;
                }
                default:
                    right = new Expr();
            }
        }
        _condition = new Expr(left, compOp, right);
    }
    return startScan(fileHandle, _condition, "");
}

int RM_FileScan::getNextRecord(Record& record) {
    char *data;
    PF_PageHandle pageHandle;
    bool found = false;
    unsigned max_slot_id = file_handle->table_header.slot_per_page;
    unsigned map_size = file_handle->table_header.slot_map_size;
    int rc = file_handle->pf_file_handle.GetThisPage(curRID.pageID, pageHandle);
    TEST_RC_NOT_ZERO_ERROR
    while (true) {   //per page
        rc = pageHandle.GetData(data);
        TEST_RC_NOT_ZERO_ERROR
        memcpy(bitdata, data + 8, map_size);
        bitmap = MyBitMap(map_size * 8, reinterpret_cast<unsigned *>(bitdata));
        curRID.slotID = bitmap.findLeftOne();
        while (curRID.slotID < max_slot_id) {   //per slot
            bitmap.setBit(curRID.slotID,0);
            // 看是否符合要求
            rc = file_handle->getRecord(curRID, record);
            TEST_RC_NOT_ZERO_ERROR
            if(condition== nullptr){
                found = true;
                break;
            }
            condition->init_calculate(tableName);
            condition->calculate(record.data, tableName);
            if (not condition->calculated or condition->is_true()) {
                condition->init_calculate(tableName);
                found = true;
                break;
            }
            curRID.slotID = bitmap.findLeftOne();
        }
        rc = file_handle->pf_file_handle.UnpinPage(curRID.pageID);
        TEST_RC_NOT_ZERO_ERROR
        if (found) break;
        rc = file_handle->pf_file_handle.GetNextPage(curRID.pageID, pageHandle);  //不只是pageID+1
        TEST_RC_NOT_ZERO_ERROR
        rc = pageHandle.GetPageNum(curRID.pageID);  //更新页数
        TEST_RC_NOT_ZERO_ERROR
        curRID.slotID = 0;
    }
    return 0;
}
