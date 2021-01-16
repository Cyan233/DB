//
// Created by cyan on 2020/10/22.
//

#include "RecordManager.h"
#include "RM_FileHandle.h"
#include "../utils/MyBitMap.h"


RM_FileHandle::~RM_FileHandle(){
    if (is_open) {
        RecordManager::getInstance().closeFile(*this);
    }
}

int RM_FileHandle::getRecord(const RID& rid, Record &record) const {
    if (rid.pageID<0 || rid.slotID >= table_header.slot_per_page || rid.slotID < 0)
        return 1;
    PF_PageHandle page_handle;
    char *page_data;
    int rc = pf_file_handle.GetThisPage(rid.pageID, page_handle);
    TEST_RC_NOT_ZERO_ERROR
    rc = page_handle.GetData(page_data);
    TEST_RC_NOT_ZERO_ERROR
    record.setData(page_data + getOffset(rid.slotID), table_header.record_size, rid);
    rc = pf_file_handle.UnpinPage(rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

int RM_FileHandle::insertRecord(const char *pData, RID& rid) {
    //debug
    //cout<<endl<<"insert record in rm file, data is"<<pData<<endl;
    int rc;
    // find spare page
    if (table_header.first_spare_page <= 0) {
        rc = insertPage();
        TEST_RC_NOT_ZERO_ERROR
        //debug
        //cout<<"insert page in rm file success"<<endl;
    }
    //find spare slot
    unsigned pageID = table_header.first_spare_page;
    PF_PageHandle page_handle;
    char *page_data;
    rc = pf_file_handle.GetThisPage(pageID, page_handle);
    TEST_RC_NOT_ZERO_ERROR   //**
    //debug
    //cout<<"get pageID, pageID is "<<pageID<<endl;
    page_handle.GetData(page_data);
    MyBitMap bitmap(table_header.slot_map_size << 3, reinterpret_cast<unsigned *>(page_data + 8));
    unsigned slotID = bitmap.findLeftOne();  //一定会找到空闲的slot
    //debug
    //cout<<"find slotID, slotID is "<<slotID<<endl;
    memcpy(page_data + getOffset(slotID), pData, table_header.record_size);
    // update
    bitmap.setBit(slotID, 0);
    if (bitmap.findLeftOne() >= table_header.slot_per_page) {  //如果用完本页最后一个slot
        table_header.first_spare_page = reinterpret_cast<int *>(page_data)[0];
        reinterpret_cast<int *>(page_data)[0] = -1;
    }
    table_header.record_num ++;
    table_header.is_modified = true;
    rid.slotID = slotID;
    rid.pageID = pageID;
    //debug
    //cout<<endl<<"insert record success"<<endl;

    rc = pf_file_handle.MarkDirty(rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_file_handle.UnpinPage(rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

int RM_FileHandle::deleteRecord(const RID &rid) {
    PF_PageHandle page_handle;
    char *page_data;
    int rc  = pf_file_handle.GetThisPage(rid.pageID, page_handle);
    TEST_RC_NOT_ZERO_ERROR
    rc = page_handle.GetData(page_data);
    TEST_RC_NOT_ZERO_ERROR
    MyBitMap bitmap(table_header.slot_map_size << 3, reinterpret_cast<unsigned *>(page_data + 8));
    if (bitmap.findLeftOne() >= table_header.slot_per_page) {
        reinterpret_cast<int *>(page_data)[0] = table_header.first_spare_page;
        table_header.first_spare_page = rid.pageID;
    }
    bitmap.setBit(rid.slotID, 1);
    table_header.record_num--;
    table_header.is_modified = true;

    rc = pf_file_handle.MarkDirty(rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_file_handle.UnpinPage(rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

int RM_FileHandle::updateRecord(const Record &rec) {
    PF_PageHandle page_handle;
    char *page_data;
    int rc = pf_file_handle.GetThisPage(rec.rid.pageID, page_handle);
    TEST_RC_NOT_ZERO_ERROR
    rc = page_handle.GetData(page_data);
    TEST_RC_NOT_ZERO_ERROR
    memcpy(page_data + getOffset(rec.rid.slotID), rec.data, table_header.record_size);
    rc = pf_file_handle.MarkDirty(rec.rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_file_handle.UnpinPage(rec.rid.pageID);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

int RM_FileHandle::insertPage() {
    PF_PageHandle page_handle;
    char *page_data;
    int rc = pf_file_handle.AllocatePage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    rc = page_handle.GetData(page_data);
    TEST_RC_NOT_ZERO_ERROR
    reinterpret_cast<int *>(page_data)[0] = table_header.first_spare_page;
    table_header.first_spare_page = table_header.page_num;
    table_header.page_num += 1;
    table_header.is_modified = true;
    memset(page_data + 8, 0xff, table_header.slot_map_size);

    rc = pf_file_handle.MarkDirty(table_header.page_num-1);
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_file_handle.UnpinPage(table_header.page_num-1);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}
