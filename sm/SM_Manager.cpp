//SM_Manager.cpp
//
//create by lmm 2020/11/29
//
//
#include <unistd.h>
#include <iostream>
#include "SM_Manager.h"
#include "../pf/pf.h"
#include "../rm/RecordManager.h"
using namespace std;

#if defined(_WIN32)
#include <io.h>
#include <direct.h>
#elif defined(__linux__) or defined(__APPLE__)
#include <sys/stat.h>
#include <sys/types.h>
#endif

void SM_Manager::copy_tbinfos(tbinfos *dest, tbinfos *sorc)
{
    memcpy(dest, sorc, sizeof(tbinfos));
}

SM_Manager &SM_Manager::GetInstance()
{
    static SM_Manager instance;
    return instance;
}

RC SM_Manager::CreateDb(const char *dbName)
{
    RC rc;
#if defined(_WIN32)
    rc = mkdir(dbName);
#else
    rc = mkdir(dbName, S_IRWXU); //用户可以读、写、执行
#endif
    if (rc != 0)
    {
        //cout << "mkdir false" << endl;
        return rc;
    }
    //cout << "create database correctly" << endl;

    //Create Database info file
    chdir(dbName);
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    /*//cout<<"create database info file"<<endl;  
    rc = pf_manager.CreateFile("dbinfo");
    //cout<<"Create dbinfo success"<<endl;
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_manager.OpenFile("dbinfo", file_handle);
    //cout<<"Open dbinfo success"<<endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.AllocatePage(page_handle);
    //cout<<"AllocatePage 1 for dbinfo success"<<endl;
    PageNum page1;
    page_handle.GetPageNum(page1);
    //cout<<"page number is:"<<page1;
    TEST_RC_NOT_ZERO_ERROR
    
    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout<<"MarkDirty page 1 success"<<endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout<<"UnpinPage page 1 success"<<endl;
    rc = pf_manager.CloseFile(file_handle);*/

    //add info in dbinfo
    chdir("..");
    PF_FileHandle pf_filehandle;
    rc = pf_manager.OpenFile("dbinfo", pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "open whole dbinfo success" << endl;
    char *wpdata;
    PF_PageHandle wpage1;
    pf_filehandle.GetFirstPage(wpage1);
    PageNum paa;
    wpage1.GetPageNum(paa);
    wpage1.GetData(wpdata);
    auto offsetptr = reinterpret_cast<int *>(wpdata);
    //cout << "write file dbinfo:" << endl;
    auto wdbinfo = reinterpret_cast<dbinfos *>(wpdata + 4);
    wdbinfo = wdbinfo + (*offsetptr);
    wdbinfo->strsize = strlen(dbName);
    strncpy(wdbinfo->dbname, dbName, wdbinfo->strsize);
    *offsetptr = *offsetptr + 1; //db记录的条数
    pf_filehandle.MarkDirty(paa);
    pf_filehandle.UnpinPage(paa);
    rc = pf_manager.CloseFile(pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "write dbinfo success" << endl;
    //todo maybe the space is not enough,allocate more pages

    return 0;
}

RC SM_Manager::DropDb(const char *dbName)
{
    char str[1010];
#if defined(_WIN32)
    sprintf(str, "rd /s/q %s", dbName);
#elif defined(__linux__) or defined(__APPLE__)
    sprintf(str, "rm -r %s", dbName);
#endif

    RC rc;
    PF_FileHandle pf_filehandle;
    PF_Manager pf_manager;
    rc = pf_manager.OpenFile("dbinfo", pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "open dbinfo success" << endl;
    char *wpdata;
    PF_PageHandle wpage1;
    pf_filehandle.GetFirstPage(wpage1);
    wpage1.GetData(wpdata);
    auto offsetptr = reinterpret_cast<int *>(wpdata);
    int offset = *offsetptr;
    auto wdbinfo = reinterpret_cast<dbinfos *>(wpdata + 4);
    //cout << "modify file dbinfo:" << endl;
    int counti = 0;
    while (counti < offset - 1)
    {
        if (strncmp(wdbinfo->dbname, dbName, wdbinfo->strsize) == 0)
        {
            for (int i = counti; i < offset - 1; ++i)
            {
                strncpy(wdbinfo->dbname, (wdbinfo + 1)->dbname, (wdbinfo + 1)->strsize);
                wdbinfo->strsize = (wdbinfo + 1)->strsize;
                wdbinfo++;
            }
            break;
        }
        counti++;
        wdbinfo++;
    }
    *offsetptr = *offsetptr - 1;
    PageNum paa;
    wpage1.GetPageNum(paa);
    rc = pf_filehandle.MarkDirty(paa);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page success" << endl;
    rc = pf_filehandle.UnpinPage(paa);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page success" << endl;
    rc = pf_manager.CloseFile(pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "dropDb " << dbName << " success" << endl;
    return system(str);
}

RC SM_Manager::UseDb(const char *dbName)
{
    RC rc;
    rc = chdir(dbName);
    //cout << "chdir in " << dbName << endl;
    return rc;
}

RC SM_Manager::CheckoutDb()
{
    RC rc;
    rc = chdir("..");
    return rc;
}

RC SM_Manager::initDbInfo()
{
    //Create Database info file
    //cout << "create database info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    rc = pf_manager.CreateFile("dbinfo");
    //cout << "Create file success" << endl;
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_manager.OpenFile("dbinfo", file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.AllocatePage(page_handle);
    //cout << "AllocatePage 1 success" << endl;
    PageNum page1;
    page_handle.GetPageNum(page1);
    //cout << "page number is:" << page1;
    TEST_RC_NOT_ZERO_ERROR

    char *wpdata;
    page_handle.GetData(wpdata);
    auto offsetptr = reinterpret_cast<int *>(wpdata);
    *offsetptr = 0;
    //cout << "init offset success" << endl;

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
}

RC SM_Manager::ShowDb()
{
    RC rc;
    PF_Manager pf_manager;
    PF_FileHandle pf_filehandle;
    rc = pf_manager.OpenFile("dbinfo", pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "open dbinfo success" << endl;
    char *wpdata;
    PF_PageHandle wpage1;
    rc = pf_filehandle.GetFirstPage(wpage1);
    TEST_RC_NOT_ZERO_ERROR
    PageNum paa;
    wpage1.GetPageNum(paa);
    //cout << "page num is " << paa << endl;
    wpage1.GetData(wpdata);
    //cout << "wpdata:" << wpdata << endl;
    //cout << "file dbinfo:" << endl;
    cout<<"show dbinfo"<<endl;

    auto offsetptr = reinterpret_cast<int *>(wpdata);
    int offset = *offsetptr;
    if (offset == 0)
    {
        //cout << "dbinfo print end" << endl;
        pf_filehandle.UnpinPage(paa);
        rc = pf_manager.CloseFile(pf_filehandle);
        TEST_RC_NOT_ZERO_ERROR
        return 0;
    }
    auto wdbinfo = reinterpret_cast<dbinfos *>(wpdata + 4);
    int counti = 0;
    while (counti < offset)
    {
        cout << wdbinfo->dbname << endl;
        counti++;
        wdbinfo++;
    }
    //cout << "dbinfo print end" << endl;
    pf_filehandle.UnpinPage(paa);
    rc = pf_manager.CloseFile(pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

RC SM_Manager::CreateTable(const char *tbName, tbinfos *tbinfo)
{
    //compute offset
    tbinfo->recordsize = 0;
    for (int i = 0; i < tbinfo->columns; ++i)
    {
        switch (tbinfo->colattr[i])
        {
        case AttrType::INT:
            tbinfo->recordsize += 4;
            continue;
        case AttrType::FLOAT:
            tbinfo->recordsize += 4;
            continue;
        case AttrType::DATE:
            tbinfo->recordsize += 10;
            continue;
        case AttrType::STRING:
            if (tbinfo->length_limit[i] > 0)
            {
                tbinfo->recordsize += tbinfo->length_limit[i];
            }
            else
            {
                tbinfo->recordsize += 100;
            }
            continue;
        default:
            break;
        }
    }

    //init tbinfos
    tbinfo->fkey_num = 0;

    //create table
    //mm: to do maybe modify the parameter nullable
    RecordManager::getInstance().createFile(tbName, tbinfo->recordsize, 0);

    //create tbinfos
    //cout << "create table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    /*
    string str1(tbName);
    string str = str1 + "_tableinfo";
    rc = pf_manager.CreateFile(str.c_str());
    //cout << "Create file success" << endl;
    TEST_RC_NOT_ZERO_ERROR*/
    string str(tbName);
    rc = pf_manager.OpenFile(str.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR

    //cout << "write data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    ////cout << "no zero error after write data in tbinfo" << endl;
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    ////cout << "wpdata is " << wpdata << endl;
    ////cout << "after" << endl;
    copy_tbinfos(wtbinfo, tbinfo);
    ////cout<<"wtbinfo->columns:"<<wtbinfo->columns<<endl;
    wtbinfo->strsize = strlen(tbName);
    strncpy(wtbinfo->tbname, tbName, wtbinfo->strsize);
    //write back
    rc = file_handle.MarkDirty(0);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 0 success" << endl;
    rc = file_handle.UnpinPage(0);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 0 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "write table info success" << endl;

    //add info in tbinfo
    PF_FileHandle pf_filehandle;
    rc = pf_manager.OpenFile("whole_table_info", pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "open whole_table_info success" << endl;
    PF_PageHandle wpage1;
    pf_filehandle.GetFirstPage(wpage1);
    PageNum paa;
    wpage1.GetPageNum(paa);
    wpage1.GetData(wpdata);
    auto offsetptr = reinterpret_cast<int *>(wpdata);
    //cout << "write whole_table_info:" << endl;
    auto whole_wtbinfo = reinterpret_cast<whole_tbinfos *>(wpdata + 4);
    whole_wtbinfo = whole_wtbinfo + (*offsetptr);
    whole_wtbinfo->strsize = strlen(tbName);
    strncpy(whole_wtbinfo->tbname, tbName, whole_wtbinfo->strsize);
    *offsetptr = *offsetptr + 1; //tb记录的条数
    pf_filehandle.MarkDirty(paa);
    pf_filehandle.UnpinPage(paa);
    rc = pf_manager.CloseFile(pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "write whole_table_info success" << endl;

    return 0;
}

RC SM_Manager::DropTable(const char *tbName)
{
    RC rc;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    rc = pf_manager.DestroyFile(tbName);
    TEST_RC_NOT_ZERO_ERROR
    //rc = pf_manager.DestroyFile(str.c_str());
    //TEST_RC_NOT_ZERO_ERROR

    PF_FileHandle pf_filehandle;
    rc = pf_manager.OpenFile("whole_table_info", pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "open whole_table_info success" << endl;
    char *wpdata;
    PF_PageHandle wpage1;
    rc = pf_filehandle.GetFirstPage(wpage1);
    TEST_RC_NOT_ZERO_ERROR
    PageNum paa;
    wpage1.GetPageNum(paa);
    //cout << "page num is " << paa << endl;
    wpage1.GetData(wpdata);
    //cout << "wpdata:" << wpdata << endl;
    //cout << "file whole_table_info:" << endl;

    auto offsetptr = reinterpret_cast<int *>(wpdata);
    int offset = *offsetptr;
    if (offset == 0)
    {
        //cout << "clean tableinfo failed" << endl;
        return 0;
    }
    auto wtbinfo = reinterpret_cast<whole_tbinfos *>(wpdata + 4);
    int counti = 0;
    while (counti < offset)
    {
        if(strncmp(wtbinfo->tbname, tbName, wtbinfo->strsize) == 0){
            wtbinfo->strsize = -1;
            //cout<<"clean tableinfo success, wtbname is "<<wtbinfo->tbname<<endl;
            break;
        }
        counti++;
        wtbinfo++;
    }
    rc = pf_filehandle.MarkDirty(paa);
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_filehandle.UnpinPage(paa);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "Unpinpage pa success" << endl;
    rc = pf_manager.CloseFile(pf_filehandle);
    //cout << "close file success" << endl;
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

RC SM_Manager::InitTbInfo()
{
    //Create table info file
    //cout << "create whole_table_info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    rc = pf_manager.CreateFile("whole_table_info");
    //cout << "Create file success" << endl;
    TEST_RC_NOT_ZERO_ERROR
    rc = pf_manager.OpenFile("whole_table_info", file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.AllocatePage(page_handle);
    //cout << "AllocatePage 1 success" << endl;
    PageNum page1;
    page_handle.GetPageNum(page1);
    //cout << "page number is:" << page1;
    TEST_RC_NOT_ZERO_ERROR

    char *wpdata;
    page_handle.GetData(wpdata);
    auto offsetptr = reinterpret_cast<int *>(wpdata);
    *offsetptr = 0;
    //cout << "init offset success" << endl;

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR

    //cout << "init whole_table_info file success" << endl;
    return 0;
}

RC SM_Manager::Descv(const char *tbName){
    RC rc;
    PF_Manager pf_manager;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    rc = pf_manager.OpenFile(tbName, file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR

    cout <<endl<< "show data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    ////cout << "no zero error after write data in tbinfo" << endl;
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);

    cout<<"table name is "<<wtbinfo->tbname<<endl;
    for (int i = 0; i < wtbinfo->columns; ++i)
    {
        string str_p(wtbinfo->attrname[i],wtbinfo->attrsize[i]);
        cout<<"attrname is "<<str_p<<endl;
        cout<<"can be null? "<<wtbinfo->cannull[i]<<endl<<endl;
    }
    cout<<"has pkey? "<<wtbinfo->has_pkey<<endl<<endl;
    for (int i = 0; i < wtbinfo->fkey_num; ++i)
    {
        string str_p(wtbinfo->reference[i],wtbinfo->rnsize[i]);
        cout<<"fkey "<<i<<" is "<<wtbinfo->foreign_key[i]<<endl;
        cout<<"reference file is "<<str_p<<endl<<endl;
    }
    
    rc = file_handle.UnpinPage(0);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 0 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "show table info success" << endl;
    return 0;
}

RC SM_Manager::ShowTb()
{
    RC rc;
    PF_Manager pf_manager;
    PF_FileHandle pf_filehandle;
    rc = pf_manager.OpenFile("whole_table_info", pf_filehandle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "open whole_table_info success" << endl;
    char *wpdata;
    PF_PageHandle wpage1;
    rc = pf_filehandle.GetFirstPage(wpage1);
    TEST_RC_NOT_ZERO_ERROR
    PageNum paa;
    wpage1.GetPageNum(paa);
    //cout << "page num is " << paa << endl;
    wpage1.GetData(wpdata);
    //cout << "wpdata:" << wpdata << endl;
    //cout << "file whole_table_info:" << endl;
    cout<<"show tables in database:"<<endl;

    auto offsetptr = reinterpret_cast<int *>(wpdata);
    int offset = *offsetptr;
    if (offset == 0)
    {
        //cout << "whole_table_info print end" << endl;
        rc = pf_filehandle.UnpinPage(paa);
        TEST_RC_NOT_ZERO_ERROR
        //cout << "Unpinpage pa success" << endl;
        rc = pf_manager.CloseFile(pf_filehandle);
        TEST_RC_NOT_ZERO_ERROR
        //cout << "closefile after pa success" << endl;
        return 0;
    }
    auto wtbinfo = reinterpret_cast<whole_tbinfos *>(wpdata + 4);
    int counti = 0;
    while (counti < offset)
    {
        if (wtbinfo->strsize < 0){
            //cout<<"cleaned: tbname is "<<wtbinfo->tbname<<endl;
        }else{
            cout << wtbinfo->tbname << endl;
        } 
        counti++;
        wtbinfo++;
    }
    //cout << "whole_table_info print end" << endl;
    rc = pf_filehandle.UnpinPage(paa);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "Unpinpage pa success" << endl;
    rc = pf_manager.CloseFile(pf_filehandle);
    //cout << "close file success" << endl;
    TEST_RC_NOT_ZERO_ERROR
    return 0;
}

RC SM_Manager::CreatePK(const char *tbName, int pkey)
{
    //modify tbinfos
    //cout << "Create primary key" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    //rc = pf_manager.OpenFile(str.c_str(), file_handle);
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    PageNum page1;
    page_handle.GetPageNum(page1);

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    wtbinfo->primary_key = pkey;
    wtbinfo->has_pkey = true;
    //cout << "add primary key completely" << endl;
    //cout << "primary key is " << wtbinfo->primary_key << endl;

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl
}

RC SM_Manager::DropPK(const char *tbName)
{
    //modify tbinfos
    //cout << "Drop primary key" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    PageNum page1;
    page_handle.GetPageNum(page1);

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    //auto wtbinfo = reinterpret_cast<tbinfos *>(wpdata);
    wtbinfo->has_pkey = false;
    //cout << "drop primary key completely" << endl;

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl
}

RC SM_Manager::CreateFK(const char *tbName, int fkey, const char *refertbname)
{
    //modify tbinfos
    //cout << "Create foreign key" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    PageNum page1;
    page_handle.GetPageNum(page1);

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    //auto wtbinfo = reinterpret_cast<tbinfos *>(wpdata);
    //cout<<"temp"<<endl<<endl;
    //cout<<"wtbinfo->fkey_num is "<<wtbinfo->fkey_num<<endl;
    wtbinfo->foreign_key[wtbinfo->fkey_num] = fkey;
    wtbinfo->rnsize[wtbinfo->fkey_num] = strlen(tbName);
    strncpy(wtbinfo->reference[wtbinfo->fkey_num], tbName, wtbinfo->fkey_num);
    wtbinfo->fkey_num++;
    //cout << "add foreign key completely" << endl;
    //cout << "foreign key is " << wtbinfo->foreign_key[wtbinfo->fkey_num - 1] << endl;

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl<< endl;
}

RC SM_Manager::DropFK(const char *tbName, int fkey, const char *refertbname)
{
    //modify tbinfos
    //cout << "Drop foreign key" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    PageNum page1;
    page_handle.GetPageNum(page1);

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    //auto wtbinfo = reinterpret_cast<tbinfos *>(wpdata);
    for (int i = 0; i < wtbinfo->fkey_num; ++i)
    {
        if (wtbinfo->foreign_key[i] == fkey && wtbinfo->rnsize[i] == strlen(refertbname) && strncmp(wtbinfo->reference[i], refertbname, wtbinfo->rnsize[i]) == 0)
        {
            for (int j = i; j < wtbinfo->fkey_num - 1; ++j)
            {
                wtbinfo->foreign_key[j] = wtbinfo->foreign_key[j + 1];
                strncpy(wtbinfo->reference[j], wtbinfo->reference[j + 1], wtbinfo->rnsize[j + 1]);
                wtbinfo->rnsize[j] = wtbinfo->rnsize[j + 1];
            }
            break;
        }
    }
    wtbinfo->fkey_num--;
    //cout << "drop foreign key completely" << endl;

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl<< endl;
    return 0;
}

RC SM_Manager::AddColumn(const char *tbName, AttrType attrt, const char *attrname, int code, bool hd, int ll)
{
    //modify tbinfos
    //cout << "Add Column" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    PageNum page1;
    page_handle.GetPageNum(page1);
    //cout << "page number is:" << page1;
    TEST_RC_NOT_ZERO_ERROR

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    //auto wtbinfo = reinterpret_cast<tbinfos *>(wpdata);

    wtbinfo->colattr[wtbinfo->columns] = attrt;
    wtbinfo->attrsize[wtbinfo->columns] = strlen(attrname);
    strncpy(wtbinfo->attrname[wtbinfo->columns], attrname, strlen(attrname));
    wtbinfo->has_default[wtbinfo->columns] = hd;
    //todo change coldi to more types
    wtbinfo->coldi[wtbinfo->columns] = code;
    wtbinfo->length_limit[wtbinfo->columns] = ll;
    wtbinfo->columns++;
    switch (wtbinfo->colattr[wtbinfo->columns - 1])
    {
    case AttrType::INT:
        wtbinfo->recordsize += 4;
        break;
    case AttrType::FLOAT:
        wtbinfo->recordsize += 4;
        break;
    case AttrType::DATE:
        wtbinfo->recordsize += 10;
        break;
    case AttrType::STRING:
        if (ll > 0)
        {
            wtbinfo->recordsize += ll;
        }
        else
        {
            wtbinfo->recordsize += 100;
        }
        break;
    default:
        break;
    }

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl<< endl;

    /*rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR

    //cout << "copy data in first page:" << endl;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR

    RecordManager::getInstance().createFile("ff", 5, 1);
    memcpy(dest, wpdata, 4092));

    //cout << "" <<endl;
    rc = file_handle.AllocatePage(page_handle);

    rc = pf_manager.DestroyFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR*/

    return 0;
}

RC SM_Manager::DropColumn(const char *tbName, AttrType attrt, const char *attrname)
{
    //modify tbinfos
    //cout << "Drop Column" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    PageNum page1;
    page_handle.GetPageNum(page1);
    //cout << "page number is:" << page1;
    TEST_RC_NOT_ZERO_ERROR

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    //auto wtbinfo = reinterpret_cast<tbinfos *>(wpdata);

    //todo 此处直接置空 maybe 改进
    for (int i = 0; i < wtbinfo->columns; ++i)
    {
        if (wtbinfo->colattr[i] == attrt && wtbinfo->attrsize[i] == strlen(attrname) && strncmp(wtbinfo->attrname[i], attrname, wtbinfo->attrsize[i]) == 0)
        {
            wtbinfo->colattr[i] = AttrType::NO_ATTR;
            break;
        }
    }

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl<< endl;

    return 0;
}

RC SM_Manager::ModifiedColumn(const char *tbName, AttrType src_attrt, const char *src_attrname, AttrType dest_attrt, const char *dest_attrname, int code, bool hd, int ll)
{
    //modify tbinfos
    //cout << "Modify Column" << endl;
    //cout << "modify table info file" << endl;
    RC rc;
    PF_FileHandle file_handle;
    PF_PageHandle page_handle;
    PF_Manager pf_manager;
    string str1(tbName);
    //string str = str1 + "_tableinfo";
    rc = pf_manager.OpenFile(str1.c_str(), file_handle);
    //cout << "Open file success" << endl;
    TEST_RC_NOT_ZERO_ERROR

    rc = file_handle.GetFirstPage(page_handle);
    TEST_RC_NOT_ZERO_ERROR
    PageNum page1;
    page_handle.GetPageNum(page1);
    //cout << "page number is:" << page1;
    TEST_RC_NOT_ZERO_ERROR

    //cout << "modify data in tbinfo" << endl;
    char *wpdata;
    rc = page_handle.GetData(wpdata);
    TEST_RC_NOT_ZERO_ERROR
    auto wheader = reinterpret_cast<TableHeader *>(wpdata);
    auto wtbinfo = &(wheader->tb_info);
    //auto wtbinfo = reinterpret_cast<tbinfos *>(wpdata);

    //todo 此处直接置空 maybe 改进
    for (int i = 0; i < wtbinfo->columns; ++i)
    {
        if (wtbinfo->colattr[i] == src_attrt && wtbinfo->attrsize[i] == strlen(src_attrname) && strncmp(wtbinfo->attrname[i], src_attrname, wtbinfo->attrsize[i]) == 0)
        {
            wtbinfo->colattr[i] = dest_attrt;
            wtbinfo->attrsize[i] = strlen(dest_attrname);
            strncpy(wtbinfo->attrname[i], dest_attrname, strlen(dest_attrname));
            wtbinfo->has_default[i] = hd;
            //todo change coldi to more types
            wtbinfo->coldi[i] = code;
            wtbinfo->length_limit[i] = ll;
            //todo change whole recordsize.
        }
    }

    //write back
    rc = file_handle.MarkDirty(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "MarkDirty page 1 success" << endl;
    rc = file_handle.UnpinPage(page1);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "UnpinPage page 1 success" << endl;
    rc = pf_manager.CloseFile(file_handle);
    TEST_RC_NOT_ZERO_ERROR
    //cout << "modify table info success" << endl<< endl;

    return 0;
}