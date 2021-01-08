//IX_IndexHandle.c
//
//create by lmm 2020/11/16
//
//

#include "IX_Internal.h"
#include "IX_IndexHandle.h"
#include "IX_Manager.h"
#include <iostream>
using namespace std;

int IX_indexHandle::valuecompare(RID rid1, RID rid2)
{
    //todo
    return 1;
}

int IX_indexHandle::Indexcompare(RID rid1, RID rid2)
{
    //todo
    int status = valuecompare(rid1, rid2);
    if (status == 0)
    {
        if (rid1.pageID < rid2.pageID)
        {
            return -1;
        }
        else if (rid1.pageID > rid2.pageID)
        {
            return 1;
        }
        else
        {
            if (rid1.slotID < rid2.slotID)
            {
                return -1;
            }
            else
            { //全相等应在调用insertrecord处进行检查
                return 1;
            }
        }
    }
    return status;
}

RC IX_indexHandle::InsertRecord(const RID &rid)
{
    RC rc;
    char *data;
    cout << endl
         << "InsertRecord begin:" << endl;
    if (fisnew)
    {
        cout << "fisnew" << endl;
        PF_PageHandle pageHandle, firstph;
        rc = pf_filehandle.GetFirstPage(firstph);
        TEST_RC_NOT_ZERO_ERROR
        char *pdata;
        firstph.GetData(pdata);
        auto tableheader = reinterpret_cast<IX_IndexHeader *>(pdata);
        tableheader->is_new = false;
        fisnew = false;
        pf_filehandle.MarkDirty(0);
        pf_filehandle.UnpinPage(0);
        cout << "mark dirty success" << endl;

        //todo 插入第一条记录
        rc = pf_filehandle.GetThisPage(rootpage, pageHandle);
        TEST_RC_NOT_ZERO_ERROR
        char *fdata;
        pageHandle.GetData(fdata);
        auto nodet = reinterpret_cast<nodetype *>(fdata);
        nodet->nodet = 1;
        auto lnode = reinterpret_cast<leafnode *>(fdata + 4); //split时如果涉及到rootpage记得更换
        lnode->size = 1;
        lnode->leafdata[0] = rid;
        cout << "Insert first record success" << endl;
        pf_filehandle.MarkDirty(rootpage);
        pf_filehandle.UnpinPage(rootpage);
        return 0;
    }

    //else 每页格式：从第1页开始，先是nodetype，然后是internode/leafnode，然后是各记录
    int check = find_position_insert(rid, rootpage);
    for (int i = 1; i < pagenums; ++i)
    {
        pf_filehandle.UnpinPage(i);
    }
    if (check == 1)
    {
        cout << "Insert record success" << endl;
        return 0;
    }
    return 1;
}

int IX_indexHandle::find_position_insert(const RID &rid, PageNum fatherpage)
{
    PF_PageHandle pageHandle;
    pf_filehandle.GetThisPage(fatherpage, pageHandle);
    char *data;
    pageHandle.GetData(data);
    int status = 0; //not completed.
    cout << "In find_position:" << endl;

    auto header = reinterpret_cast<nodetype *>(data);
    if (header->nodet == 0)
    { //internode
        cout << "find internode ing..." << endl;
        auto itnode = reinterpret_cast<internode *>(data + 4);
        for (int i = 0; i < itnode->size; ++i)
        {
            if (Indexcompare(rid, itnode->interdata[i]) <= 0)
            { //rid在左边 在它的sonpointer页里
                status = find_position_insert(rid, itnode->sonpointer[i]);
                if (status == 1)
                {
                    return 1;
                }
                else
                { //需要处理上溢带来的标节点插入//todo
                    cout << "handle mark insert" << endl;
                    if (itnode->size < BN)
                    {
                        for (int j = itnode->size; j > i; --j)
                        {
                            itnode->interdata[j] = itnode->interdata[j - 1];
                            itnode->sonpointer[j] = itnode->sonpointer[j - 1];
                        }
                        itnode->interdata[i] = ridd;
                        itnode->sonpointer[i + 1] = topt;
                        itnode->size++;
                        return 1;
                    }
                    //如果再次发生了上溢

                    cout << "handle internal split" << endl;
                    RC rc;
                    PF_PageHandle newpage;
                    rc = pf_filehandle.AllocatePage(newpage);
                    pagenums++;
                    PageNum ndd;
                    newpage.GetPageNum(ndd);
                    pf_filehandle.MarkDirty(ndd);
                    cout << "new pagenum is" << ndd;

                    //if(pageHandle.GetPageNum == rootpage)
                    char *ndata;
                    newpage.GetData(ndata);
                    auto nodet = reinterpret_cast<nodetype *>(ndata);
                    nodet->nodet = 0;
                    auto newlt = reinterpret_cast<internode *>(ndata + 4);
                    for (int i = 0; i < BN / 2; ++i)
                    {
                        newlt->interdata[i] = itnode->interdata[i + BN / 2];
                        newlt->sonpointer[i] = itnode->sonpointer[i + BN / 2];
                        //itnode->interdata[i + BN / 2] = 0;
                        itnode->sonpointer[i + BN / 2] = 0;
                    }
                    newlt->size = BN / 2;
                    itnode->size = BN / 2;
                    if (Indexcompare(ridd, itnode->interdata[itnode->size - 1]) <= 0)
                    {
                        for (int i = 0; i < itnode->size; ++i)
                        {
                            if (Indexcompare(ridd, itnode->interdata[i]) <= 0)
                            { //rid在i左边
                                for (int j = itnode->size; j >= i + 1; --j)
                                {
                                    itnode->interdata[j] = itnode->interdata[j - 1];
                                    itnode->sonpointer[j] = itnode->sonpointer[j - 1];
                                }
                                itnode->interdata[i] = ridd;
                                itnode->sonpointer[i + 1] = topt;
                                itnode->size++;
                                break;
                            }
                        }
                    }
                    else
                    {
                        int i = 0;
                        for (i = 0; i < newlt->size; ++i)
                        {
                            if (Indexcompare(rid, newlt->interdata[i]) <= 0)
                            { //rid在i左边
                                for (int j = newlt->size; j >= i + 1; --j)
                                {
                                    newlt->interdata[j] = newlt->interdata[j - 1];
                                    newlt->sonpointer[j] = newlt->sonpointer[j - 1];
                                }
                                newlt->interdata[i] = ridd;
                                newlt->sonpointer[i + 1] = topt;
                                newlt->size++;
                                break;
                            }
                        }
                        if (i == newlt->size) //标节点上溢不会出现这种情况
                        {
                            cout << "Error" << endl;
                        }
                    }
                    PageNum pnn;
                    pageHandle.GetPageNum(pnn);
                    if (pnn == rootpage)
                    {
                        PF_PageHandle nrp;
                        rc = pf_filehandle.AllocatePage(nrp);
                        pagenums++;
                        //if(pageHandle.GetPageNum == rootpage)
                        char *rdata;
                        nrp.GetData(rdata);
                        auto rnodet = reinterpret_cast<nodetype *>(rdata);
                        rnodet->nodet = 0;
                        auto rint = reinterpret_cast<internode *>(rdata + 4);
                        rint->interdata[0] = itnode->interdata[itnode->size - 1];
                        rint->interdata[1] = newlt->interdata[newlt->size - 1];
                        rint->sonpointer[0] = pnn;
                        newpage.GetPageNum(pnn);
                        rint->sonpointer[1] = pnn;
                        rint->size = 2;
                        nrp.GetPageNum(pnn);
                        pf_filehandle.MarkDirty(pnn);
                        rootpage = pnn;
                        cout << "rootpage changed: " << rootpage << endl;
                        return 1;
                    }
                    ridd = itnode->interdata[itnode->size - 1]; //要传递给上层
                    newpage.GetPageNum(pnn);
                    topt = pnn;
                    return 2;
                }
            }
        }
        itnode->interdata[itnode->size - 1] = rid; //更新标值为新的rid：更大
        status = find_position_insert(rid, itnode->sonpointer[itnode->size - 1]);
        if (status == 1)
        {
            return 1;
        }
        else
        { //需要处理上溢带来的标节点插入//todo
            cout << "handle mark insert" << endl;
            if (itnode->size < BN)
            {
                itnode->interdata[itnode->size] = itnode->interdata[itnode->size - 1];
                itnode->sonpointer[itnode->size] = itnode->sonpointer[itnode->size - 1];
                itnode->interdata[itnode->size - 1] = ridd;
                itnode->sonpointer[itnode->size] = topt;
                itnode->size++;
                return 1;
            }
            //如果再次发生了上溢

            RC rc;
            PF_PageHandle newpage;
            rc = pf_filehandle.AllocatePage(newpage);
            pagenums++;
            PageNum dd;
            newpage.GetPageNum(dd);
            pf_filehandle.MarkDirty(dd);
            cout << "new pagenum is " << dd << endl;
            //if(pageHandle.GetPageNum == rootpage)
            char *ndata;
            newpage.GetData(ndata);
            auto nodet = reinterpret_cast<nodetype *>(ndata);
            nodet->nodet = 0;
            auto newlt = reinterpret_cast<internode *>(ndata + 4);
            for (int i = 0; i < BN / 2; ++i)
            {
                newlt->interdata[i] = itnode->interdata[i + BN / 2];
                newlt->sonpointer[i] = itnode->sonpointer[i + BN / 2];
                //itnode->interdata[i + BN / 2] = 0;
                itnode->sonpointer[i + BN / 2] = 0;
            }
            newlt->size = BN / 2;
            itnode->size = BN / 2;
            if (Indexcompare(ridd, itnode->interdata[itnode->size - 1]) <= 0)
            {
                for (int i = 0; i < itnode->size; ++i)
                {
                    if (Indexcompare(ridd, itnode->interdata[i]) <= 0)
                    { //rid在i左边
                        for (int j = itnode->size; j >= i + 1; --j)
                        {
                            itnode->interdata[j] = itnode->interdata[j - 1];
                            itnode->sonpointer[j] = itnode->sonpointer[j - 1];
                        }
                        itnode->interdata[i] = ridd;
                        itnode->sonpointer[i + 1] = topt;
                        itnode->size++;
                        break;
                    }
                }
            }
            else
            {
                int i = 0;
                for (i = 0; i < newlt->size; ++i)
                {
                    if (Indexcompare(rid, newlt->interdata[i]) <= 0)
                    { //rid在i左边
                        for (int j = newlt->size; j >= i + 1; --j)
                        {
                            newlt->interdata[j] = newlt->interdata[j - 1];
                            newlt->sonpointer[j] = newlt->sonpointer[j - 1];
                        }
                        newlt->interdata[i] = ridd;
                        newlt->sonpointer[i + 1] = topt;
                        newlt->size++;
                        break;
                    }
                }
                if (i == newlt->size) //标节点上溢不会出现这种情况
                {
                    cout << "Error" << endl;
                }
            }

            PageNum pn;
            pageHandle.GetPageNum(pn);
            if (pn == rootpage)
            {
                PF_PageHandle nrp;
                rc = pf_filehandle.AllocatePage(nrp);
                pagenums++;
                //if(pageHandle.GetPageNum == rootpage)
                char *rdata;
                nrp.GetData(rdata);
                auto rnodet = reinterpret_cast<nodetype *>(rdata);
                rnodet->nodet = 0;
                auto rint = reinterpret_cast<internode *>(rdata + 4);
                rint->interdata[0] = itnode->interdata[itnode->size - 1];
                rint->interdata[1] = newlt->interdata[newlt->size - 1];
                rint->sonpointer[0] = pn;
                newpage.GetPageNum(pn);
                rint->sonpointer[1] = pn;
                rint->size = 2;
                nrp.GetPageNum(pn);
                rootpage = pn;
                pf_filehandle.MarkDirty(pn);
                cout << "rootpage changed , rootpage is " << rootpage << endl;
                return 1;
            }
            ridd = itnode->interdata[itnode->size - 1]; //要传递给上层
            newpage.GetPageNum(pn);
            topt = pn;
            return 2;
        }
    }
    else
    { //leafnode
        cout << "find leaf node ing" << endl;
        auto lnode = reinterpret_cast<leafnode *>(data + 4);
        if (lnode->size < BN)
        {
            cout << "Insert directly" << endl;
            for (int i = 0; i < lnode->size; ++i)
            {
                if (Indexcompare(rid, lnode->leafdata[i]) <= 0)
                { //rid在i左边
                    for (int j = lnode->size; j >= i + 1; --j)
                    {
                        lnode->leafdata[j] = lnode->leafdata[j - 1];
                    }
                    lnode->leafdata[i] = rid;
                    lnode->size++;
                    return 1; //finish
                }
            }
            lnode->leafdata[lnode->size] = rid;
            lnode->size++;
            return 1;
        }
        else
        {
            //插入并处理上溢操作
            cout << "handle leafnode split" << endl;
            RC rc;
            PF_PageHandle newpage;
            rc = pf_filehandle.AllocatePage(newpage);
            pagenums++;
            //if(pageHandle.GetPageNum == rootpage)
            char *ndata;
            newpage.GetData(ndata);
            auto nodet = reinterpret_cast<nodetype *>(ndata);
            nodet->nodet = 1;
            auto newlt = reinterpret_cast<leafnode *>(ndata + 4);
            for (int i = 0; i < BN / 2; ++i)
            {
                newlt->leafdata[i] = lnode->leafdata[i + BN / 2];
                //lnode->leafdata[i + BN / 2] = 0;
            }
            newlt->size = BN / 2;
            lnode->size = BN / 2;
            PageNum pn;
            newpage.GetPageNum(pn);
            pf_filehandle.MarkDirty(pn);
            lnode->rightpage = pn;
            topt = pn;
            pageHandle.GetPageNum(pn);
            newlt->leftpage = pn;
            if (Indexcompare(rid, lnode->leafdata[lnode->size - 1]) <= 0)
            {
                for (int i = 0; i < lnode->size; ++i)
                {
                    if (Indexcompare(rid, lnode->leafdata[i]) <= 0)
                    { //rid在i左边
                        for (int j = lnode->size; j >= i + 1; --j)
                        {
                            lnode->leafdata[j] = lnode->leafdata[j - 1];
                        }
                        lnode->leafdata[i] = rid;
                        lnode->size++;
                        break;
                    }
                }
            }
            else
            {
                int i = 0;
                for (i = 0; i < newlt->size; ++i)
                {
                    if (Indexcompare(rid, newlt->leafdata[i]) <= 0)
                    { //rid在i左边
                        for (int j = newlt->size; j >= i + 1; --j)
                        {
                            newlt->leafdata[j] = newlt->leafdata[j - 1];
                        }
                        newlt->leafdata[i] = rid;
                        newlt->size++;
                        break;
                    }
                }
                if (i == newlt->size)
                {
                    newlt->leafdata[newlt->size] = rid;
                    newlt->size++;
                }
            }

            if (pn == rootpage)
            {
                PF_PageHandle nrp;
                rc = pf_filehandle.AllocatePage(nrp);
                pagenums++;
                //if(pageHandle.GetPageNum == rootpage)
                char *rdata;
                nrp.GetData(rdata);
                auto rnodet = reinterpret_cast<nodetype *>(rdata);
                rnodet->nodet = 0;
                auto rint = reinterpret_cast<internode *>(rdata + 4);
                rint->interdata[0] = lnode->leafdata[lnode->size - 1];
                rint->interdata[1] = newlt->leafdata[newlt->size - 1];
                rint->sonpointer[0] = pn;
                newpage.GetPageNum(pn);
                rint->sonpointer[1] = pn;
                rint->size = 2;
                nrp.GetPageNum(pn);
                pf_filehandle.MarkDirty(pn);
                rootpage = pn;
                cout << "rootpage changed, rootpage is " << rootpage << endl;
                return 1;
            }
            ridd = lnode->leafdata[lnode->size - 1]; //要传递给上层
            return 2;
        }
    }
}

RC IX_indexHandle::DeleteRecord(const RID &rid)
{
    return find_position_delete(rid, rootpage);
}

int IX_indexHandle::find_position_delete(const RID &rid, PageNum fatherPage)
{
    /*RC rc;
    //todo
    return 0;*/
    RC rc;
    PF_PageHandle pageHandle;
    rc = pf_filehandle.GetThisPage(fatherPage, pageHandle);
    TEST_RC_NOT_ZERO_ERROR

    char *fdata;
    pageHandle.GetData(fdata);
    auto nodet = reinterpret_cast<nodetype *>(fdata);
    if (nodet->nodet == 1)
    { //leafnode
        auto lnode = reinterpret_cast<leafnode *>(fdata + 4);
        for (int i = 0; i < lnode->size; ++i)
        {
            if (Indexcompare(rid, lnode->leafdata[i]) == 0)
            {
                for (int j = i; j < lnode->size - 1; ++j)
                {
                    lnode->leafdata[j] = lnode->leafdata[j + 1];
                }
                lnode->size--;
                cout << "delete success" << endl;
                pf_filehandle.MarkDirty(fatherPage);
                pf_filehandle.UnpinPage(fatherPage);
                return 0; //Delete success
            }
        }
        cout << "not find" << endl;
        return 1; //delete error
    }
    //internode
    auto itnode = reinterpret_cast<internode *>(fdata + 4); //split时如果涉及到rootpage记得更换
    int status = 1;                                         //not delete
    int comparee = 0;
    for (int i = 0; i < itnode->size; ++i)
    {
        comparee = Indexcompare(rid, itnode->interdata[i]);
        if (comparee <= 0)
        {
            status = find_position_delete(rid, itnode->sonpointer[i]);
            if (comparee == 0)
            {
                for (int j = i; j < itnode->size - 1; ++j)
                {
                    itnode->interdata[j] = itnode->interdata[j + 1];
                    itnode->sonpointer[j] = itnode->sonpointer[j + 1];
                }
                itnode->size--;
            }
            return status;
        }
    }
    return status; //error occur
}