//IX_Manager.cpp
//
//create by lmm 2020/11/15
//
//

#include"IX_Manager.h"
#include"IX_IndexHandle.h"
#include"IX_Internal.h"
#include"../rm/RecordManager.h"
#include"../rm/RM_FileScan.h"
#include<iostream>
using namespace std;

IX_Manager& IX_Manager::GetInstance(){
    static IX_Manager instance;
    return instance;
}

//suppose the indexnum for the filename is unique.
RC IX_Manager::CreateIndex(const char* filename, int indexnum, AttrType attrType, int attrLength, int offset){
    //gen filename
    string str1(filename);
    string str = str1 + to_string(indexnum);

    //create indexfile and open it
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    int rc = pf_manager.CreateFile(str.c_str());
    cout<<"Create file success"<<endl;
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_manager.OpenFile(str.c_str(), file_handle);
    cout<<"Open file success"<<endl;
    TEST_RC_NOT_ZERO_ERROR
    rc = file_handle.AllocatePage(page_handle);
    cout<<"AllocatePage 1 success"<<endl;
    PageNum page1;
    page_handle.GetPageNum(page1);
    cout<<"page number is:"<<page1;
    TEST_RC_NOT_ZERO_ERROR
    char *data;
    page_handle.GetData(data);
    cout<<"GetData success"<<endl;

    //set the first page
    auto header = reinterpret_cast<IX_IndexHeader *>(data);
    header->filename = filename;
    header->attrType = attrType;
    header->attrLength = attrLength;
    header->offset = offset;
    header->is_new = true;

    //allocate page 2 as a internal node page
    rc = file_handle.AllocatePage(page_handle);
    cout<<"AllocatePage 2 success"<<endl;
    PageNum page2;
    page_handle.GetPageNum(page2);
    cout<<"page number is:"<<page2;
    TEST_RC_NOT_ZERO_ERROR
    header->rNpN = page2;
    header->pagenums = 2;
    //page_handle.GetData(data);
    //cout<<"GetData success"<<endl;
    //auto nodet = reinterpret_cast<nodetype *>(data);
    //nodet->nodet = -1;
    //auto rootnode = reinterpret_cast<internode *>(data + 4);
    //rootnode->sonpointer[0]=rootnode->sonpointer[1]=-1;*/

    //write back
    rc = file_handle.MarkDirty(0);
    TEST_RC_NOT_ZERO_ERROR
    cout<<"MarkDirty page 1 success"<<endl;
    rc = file_handle.UnpinPage(0);
    TEST_RC_NOT_ZERO_ERROR
    cout<<"UnpinPage page 1 success"<<endl;
    rc = file_handle.MarkDirty(1);
    TEST_RC_NOT_ZERO_ERROR
    cout<<"MarkDirty page 2 success"<<endl;
    rc = file_handle.UnpinPage(1);
    cout<<"UnpinPage page 2 success"<<endl;
    TEST_RC_NOT_ZERO_ERROR

    //insert all the indexs 调用insertIndex接口
    //open sourcefile to read index
    //scan get RID
    IX_indexHandle ix_indexhandle;
    ix_indexhandle.fisnew = true;
    ix_indexhandle.rootpage = page2;
    ix_indexhandle.pagenums = 2;
    ix_indexhandle.pf_filehandle = file_handle;
    RID rid;
    Record record;
    RM_FileHandle sourcefile;
    RecordManager::getInstance().openFile(filename, sourcefile);
    cout<<"openFile success"<<endl;
    RM_FileScan sourcescan;
//    sourcescan.startScan(&sourcefile, nullptr, str.c_str());
    cout<<"startScan success"<<endl;

    while(!(sourcescan.getNextRecord(record))){
        rc = ix_indexhandle.InsertRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
        rc = ix_indexhandle.InsertRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
        rc = ix_indexhandle.InsertRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
        rc = ix_indexhandle.InsertRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
        rc = ix_indexhandle.InsertRecord(record.rid);
        TEST_RC_NOT_ZERO_ERROR
        cout<<"record geted. record.data is"<<record.data<<endl;
        cout<<"InsertRecord success"<<endl;
        break;
    }
    cout<<"Insert record completely."<<endl;

    //close indexfile and close sourcefile
    //rc = file_handle.ForcePages();
    rc = pf_manager.CloseFile(file_handle);
    cout<<"CloseFile success"<<endl;
    TEST_RC_NOT_ZERO_ERROR
    rc = RecordManager::getInstance().closeFile(sourcefile);
    cout<<"closeFile success"<<endl;
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

RC IX_Manager::DestroyIndex(const char* filename, int indexnum){
    RC rc;
    string str1(filename);
    string str = str1 + to_string(indexnum);
    rc = pf_manager.DestroyFile(str.c_str());
    cout<<"DestroyFile success"<<endl;
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

RC IX_Manager::OpenIndex(const char* filename, int indexnum, IX_indexHandle& ix_indexhandle){
    RC rc;
    cout<<"in IX:OpenIndex"<<endl;
    string str1(filename);
    string str = str1 + to_string(indexnum);
    PF_FileHandle pf_fileh;
    rc = pf_manager.OpenFile(str.c_str(), pf_fileh);
    cout<<"Open Indexfile success"<<endl;
    PF_PageHandle pageh;
    pf_fileh.GetFirstPage(pageh);
    char* pdata;
    pageh.GetData(pdata);
    auto tableheader = reinterpret_cast<IX_IndexHeader *>(pdata);
    cout<<"tableheader->filename is "<<tableheader->filename;
    //cout<<"tableheader->attrType is "<<tableheader->attrType;
    cout<<"tableheader->attrLength is "<<tableheader->attrLength;
    cout<<"tableheader->rNpN is "<<tableheader->rNpN;
    ix_indexhandle.rootpage = tableheader->rNpN;
    ix_indexhandle.pagenums = tableheader->pagenums;
    if(tableheader->is_new == true){
        ix_indexhandle.fisnew = true;
    }
    cout<<"tableheader->offset is "<<tableheader->offset;
    pf_fileh.UnpinPage(0);
    ix_indexhandle.pf_filehandle = pf_fileh;
}

RC IX_Manager::CloseIndex(IX_indexHandle& ix_indexhandle){
    RC rc;
    cout<<"in IX:Close Index"<<endl;
    //!!!!!!Unpinpage 0
    //rc = ix_indexhandle.pf_filehandle.UnpinPage(0);
    //TEST_RC_NOT_ZERO_ERROR
    rc = pf_manager.CloseFile(ix_indexhandle.pf_filehandle);
    cout<<"close file success"<<endl;
    cout<<"end in CloseIndex"<<endl<<endl;
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}