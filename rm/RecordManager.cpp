//
// Created by cyan on 2020/10/21.
//

#include "RecordManager.h"
#include "RM_FileScan.h"
//debug two
using namespace std;


int RecordManager::createFile(const string& filename, unsigned record_size, int nullable_num) {
    //debug
    cout<<"create rm file, filename is"<<filename<<endl;
    // Create the file
    int rc = pf_manager.CreateFile(filename.c_str());
    TEST_RC_NOT_ZERO_ERROR

    // Create table header
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    rc = pf_manager.OpenFile(filename.c_str(), file_handle);
    TEST_RC_NOT_ZERO_ERROR
    rc = file_handle.AllocatePage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    char *data;
    page_handle.GetData(data);  //set data pointer

    //Init table header
    auto header = reinterpret_cast<TableHeader *>(data);
    header->first_spare_page = -1;  //表示没有空页
    header->page_num = 1;
    header->record_num = 0;
    header->record_size = record_size;
    header->slot_per_page = (PF_PAGE_SIZE - sizeof(TableHeader)) / (record_size + nullable_num + 1) - 1; //todo nullable
    header->slot_map_size = (header->slot_per_page>>3u)+1;

    //Write back
    rc = file_handle.MarkDirty(0);
    TEST_RC_NOT_ZERO_ERROR
    rc = file_handle.UnpinPage(0);
    TEST_RC_NOT_ZERO_ERROR

    //Close the file
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}


int RecordManager::openFile(const string& filename, RM_FileHandle &file_handle) {
    //debug
    cout<<"open rm file, filename is"<<filename<<endl;
    // set table header
    int rc = pf_manager.OpenFile(filename.c_str(), file_handle.pf_file_handle);
    TEST_RC_NOT_ZERO_ERROR

    PF_PageHandle page_handle;
    rc = file_handle.pf_file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    char *data;
    rc = page_handle.GetData(data);
    TEST_RC_NOT_ZERO_ERROR
    auto header = reinterpret_cast<TableHeader *>(data);
    file_handle.table_header = *header;
    //** file_handle.table_header.is_modified = false;
    file_handle.is_open = true;

    // Release table header
    rc = file_handle.pf_file_handle.UnpinPage(0);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

int RecordManager::closeFile(RM_FileHandle& file_handle) {
    //debug
    //cout<<"close file begin"<<endl;
    int rc;
    if (file_handle.table_header.is_modified) {
        PF_PageHandle page_handle;
        char *data;
        file_handle.pf_file_handle.GetFirstPage(page_handle);
        page_handle.GetData(data);
        //mm change to see
        //auto header = reinterpret_cast<TableHeader *>(data);
        //*header = file_handle.table_header;
        rc = file_handle.pf_file_handle.MarkDirty(0);
        TEST_RC_NOT_ZERO_ERROR
        rc = file_handle.pf_file_handle.UnpinPage(0);
        TEST_RC_NOT_ZERO_ERROR
    }

    //Close the file
    rc = pf_manager.CloseFile(file_handle.pf_file_handle);
    TEST_RC_NOT_ZERO_ERROR
    file_handle.is_open = false;
    //debug
    cout<<"close file success"<<endl;
    return 0;
}
