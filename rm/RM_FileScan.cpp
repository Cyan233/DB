//
// Created by cyan on 2021/1/9.
//

#include "RM_FileScan.h"
#include "vector"
#include "Compare.h"
   
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
    //debug
    //cout<<"in getNextRecord, curRID.pageID is "<<curRID.pageID<<endl;
    int rc = file_handle->pf_file_handle.GetThisPage(curRID.pageID, pageHandle);
    TEST_RC_NOT_ZERO_ERROR
    while (true) {   //per page
        rc = pageHandle.GetData(data);
        TEST_RC_NOT_ZERO_ERROR
        memcpy(bitdata, data + 8, map_size);
        bitmap = MyBitMap(map_size * 8, reinterpret_cast<unsigned *>(bitdata));
        curRID.slotID = bitmap.findLeftOne();
        //debug
        //cout<<"curRID.slotID is "<<curRID.slotID<<endl;
        while (curRID.slotID < max_slot_id) {   //per slot
            bitmap.setBit(curRID.slotID,0);
            // 看是否符合要求
            rc = file_handle->getRecord(curRID, record);
            TEST_RC_NOT_ZERO_ERROR
            //debug
            cout<<"begin to satisfy "<<endl;
            if(conditions == nullptr){
                found = true;
                break;
            }
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
