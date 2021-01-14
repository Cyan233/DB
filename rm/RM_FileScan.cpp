//
// Created by cyan on 2021/1/9.
//

#include "RM_FileScan.h"
#include "RecordManager.h"
#include "vector"


bool satisfy(const vector<Condition> &conditions, char* data, tbinfos tb_info){
    for(auto &condition: conditions){ //对于每一个条件，都要满足
        int offset = 0;
        for(int i=0; i<tb_info.columns; i++) {  //查找该条件对应的列
            if(strcmp(condition.col.attr_name, tb_info.attrname[i])==0){
                bool ok = false;
                switch (tb_info.colattr[i]){
                    case AttrType::DATE :
                    case AttrType::INT :{
                        int vi = *reinterpret_cast<int *>(data+offset);
                        ok = compare(vi, condition.compare_value.i, condition.compOp);
                        break;
                    }
                    case AttrType::FLOAT : {
                        float vf = *reinterpret_cast<float *>(data + offset);
                        ok = compare(vf, condition.compare_value.f, condition.compOp);
                        break;
                    }
                }
                if(ok)
                    break;  //比对下一个条件
                else
                    return false;
            }
            offset += tb_info.attrsize[i];
        }
    }
    return true;
}

int RM_FileScan::startScan(RM_FileHandle *_file_handle, vector<Condition> *_conditions) {
    file_handle = _file_handle;
    curRID.pageID = 1;   //当前的页
    curRID.slotID = 0;   //当前的槽
    conditions = _conditions;
    delete[] bitdata;
    bitdata = new char[file_handle->table_header.slot_map_size];
    return 0;
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
            if (satisfy(*conditions, record.data, file_handle->table_header.tb_info)) {
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
