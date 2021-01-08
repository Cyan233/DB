//IX_FindSpeRec.cpp
//
//create by lmm 2020/11/28
//
//

#include"IX_Internal.h"
#include"IX_IndexHandle.h"
#include"IX_FindSpeRec.h"
#include<iostream>
using namespace std;

RC IX_FindSpeRec::OpenScan(IX_indexHandle &indexhandle, CompOp compOp, const void *value){
    //todo
    comp = compOp;
    findvalue = value;
    int status = find_position(indexhandle, indexhandle.rootpage);
    return status;
}

int IX_FindSpeRec::find_position(IX_indexHandle &indexhandle, PageNum fatherPage){
    RC rc;
    PF_PageHandle pageHandle;
    rc = indexhandle.pf_filehandle.GetThisPage(fatherPage, pageHandle);
    TEST_RC_NOT_ZERO_ERROR

    char *fdata;
    pageHandle.GetData(fdata);
    auto nodet = reinterpret_cast<nodetype *>(fdata);
    if (nodet->nodet == 1)
    { //leafnode
        auto lnode = reinterpret_cast<leafnode *>(fdata + 4);
        for (int i = 0; i < lnode->size; ++i)
        {
            if (Compare(lnode->leafdata[i]) == 0)//第一个相等的
            {
                cout<<"find success"<<endl;
                p_current = fatherPage;
                current_index = i;
                return 0;
            }
        }
        cout << "not find, error occur" << endl;
        return 1; //delete error
    }
    //internode
    auto itnode = reinterpret_cast<internode *>(fdata + 4); 
    int status = 1;                                         //not find
    int comparee = 0;
    for (int i = 0; i < itnode->size; ++i)
    {
        comparee = Compare(itnode->interdata[i]);
        if (comparee <= 0)
        {
            status = find_position(indexhandle, itnode->sonpointer[i]);
            return status;
        }
    }
    return status; //error occur
}

RC IX_FindSpeRec::FindNextRec(IX_indexHandle &indexhandle, RID& rid){
    //todo
    RC rc;
    PF_PageHandle pageHandle;
    rc = indexhandle.pf_filehandle.GetThisPage(p_current, pageHandle);
    TEST_RC_NOT_ZERO_ERROR

    char *fdata;
    pageHandle.GetData(fdata);
    auto nodet = reinterpret_cast<nodetype *>(fdata);
    if(nodet->nodet == 0){
        cout<<"Error occur"<<endl;
    }
    auto lnode = reinterpret_cast<leafnode *>(fdata + 4);
    if(Compare(lnode->leafdata[current_index])!=0){
        cout<<"Not any more"<<endl;
        return 1;
    }
    cout<<"find success"<<endl;
    rid = lnode->leafdata[current_index];
    if(current_index < lnode->size - 1){
        current_index++;
    }else{
        p_current = lnode->rightpage;
        current_index = 0;
    }
    return 0;//find success
}

bool IX_FindSpeRec::Compare(RID& rid){
    //todo
    return 1;
    //当前值在调用处给定的标识符findvalue的左边是时<0 相等时=0 右边时>0
}