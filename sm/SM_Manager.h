//SM_Manager.h
//
//create by lmm 2020/11/29
//
//
#ifndef SM_Manager_H
#define SM_Manager_H
#include "../Constants.h"
#include "../rm/RecordManager.h"
#include "../ix/IX_Manager.h"
#include "SM_type.h"

class SM_Manager{
private:
    void copy_tbinfos(tbinfos* dest, tbinfos* sorc);
public:
    RC CreateDb(const char *dbName);//创建数据库
    RC DropDb(const char *dbName);//删除数据库
    RC UseDb(const char *dbName);//切换到该数据库
    RC CheckoutDb();//切出该数据库
    RC initDbInfo();//init数据库信息 记录所有的数据库
    RC ShowDb();//列出现有的所有数据库以及其包含的所有表名

    RC Descv(const char *tbName);//列出指定表的模式信息
    
    //to modified
    RC CreateTable(const char *tbName, tbinfos* tbinfo);
    RC DropTable(const char *tbName);
    RC InitTbInfo();//init表信息 记录所有的表
    RC ShowTb();//列出现有的所有表

    //主键的添加与取消
    //to modified
    RC CreatePK(const char *tbName, const char *pkey);
    RC DropPK(const char *tbName);

    //外键的添加与取消
    //to modified
    RC CreateFK(const char *tbName, int fkey, const char* refertbname);
    RC DropFK(const char *tbName, int fkey, const char* refertbname);

    //列的添加、删除以及修改
    RC AddColumn(const char *tbName, AttrType attrt, const char* attrname, int code, bool hd, int ll);
    //todo change AttrType attrt to a list
    RC DropColumn(const char *tbName, AttrType attrt, const char* attrname);
    //todo
    RC ModifiedColumn(const char *tbName, AttrType src_attrt, const char* src_attrname, AttrType dest_attrt, const char* dest_attrname, int code, bool hd, int ll);
    static SM_Manager& GetInstance();
};

#endif //!SM_Manager_H