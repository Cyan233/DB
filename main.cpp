#include <iostream>

#include "rm/RecordManager.h"
#include "rm/RM_FileHandle.h"
#include "utils/MyBitMap.h"
#include "ix/IX_Manager.h"
#include "sm/SM_Manager.h"
#include "ql/QueryManager.h"

void test_query(){
    tbinfos tbinfo;
    tbinfo.columns = 2;
    tbinfo.colattr[0] = AttrType::INT;
    tbinfo.colattr[1] = AttrType::FLOAT;
    tbinfo.attrsize[0] = tbinfo.attrsize[0] = 4;
    strcpy(tbinfo.attrname[0],"id");
    strcpy(tbinfo.attrname[1],"weight");
    SM_Manager::GetInstance().CreateTable("query", &tbinfo);
    SM_Manager::GetInstance().ShowTb();
    QueryManager qm;
    vector<vector<value>> records;
    vector<value> rec;
    value v;
    v.i = 5;
    rec.push_back(v);
    v.f = 1.2;
    rec.push_back(v);
    records.push_back(rec);
    rec[0].i = 10;
    records.push_back(rec);
    qm.Insert("query", records);
    vector<char*> tbnames;
    tbnames.push_back("query");
    vector<vector<Col>> selector;
    vector<Col> sel;
    selector.push_back(sel);
    vector<Condition_joint> conditions_joint;
    vector<vector<Condition>> conditions;
    vector<Condition> a;
    conditions.push_back(a);
    qm.Select(tbnames, selector, conditions_joint, conditions);
    cout<<"*****"<<endl;
    // delete
    vector<Condition> where;
    Condition w; strcpy(w.col.attr_name, "id"); w.compOp = CompOp::EQ_OP; w.compare_value.i = 5;
    where.push_back(w);
    qm.Delete("query", where);
    qm.Select(tbnames, selector, conditions_joint, conditions);

    SM_Manager::GetInstance().DropTable("query");
}

int main() {
    MyBitMap::initConst();
    test_query();
    // test record manager
    RM_FileHandle rm;
    RecordManager::getInstance().createFile("ff", 5, 1);
    RecordManager::getInstance().openFile("ff", rm);
    char a[5]="df";
    RID rid_one, rid_two;
    Record record;
    rm.insertRecord(a, rid_one);
    rm.insertRecord("hello", rid_two);
    rm.getRecord(rid_one, record);
    cout<<"record_one geted. record.data is"<<record.data<<endl;
    rm.getRecord(rid_two, record);
    cout<<"record_two geted. record.data is"<<record.data<<endl;
    //rm.deleteRecord(rid_two);
    RecordManager::getInstance().closeFile(rm);
    //RecordManager::getInstance().destroyFile("ff");

    //test ix manager
    IX_Manager::GetInstance().CreateIndex("ff", 1, AttrType::INT, 10, 10);
    IX_indexHandle indexhandle;
    IX_Manager::GetInstance().OpenIndex("ff", 1, indexhandle);
    IX_Manager::GetInstance().CloseIndex(indexhandle);
    IX_Manager::GetInstance().DestroyIndex("ff", 1);

    //test sm manager
    SM_Manager::GetInstance().initDbInfo();
    SM_Manager::GetInstance().ShowDb();
    SM_Manager::GetInstance().CreateDb("db111");
    SM_Manager::GetInstance().ShowDb();
    SM_Manager::GetInstance().CreateDb("db222");
    SM_Manager::GetInstance().ShowDb();
    SM_Manager::GetInstance().DropDb("db111");
    SM_Manager::GetInstance().ShowDb();
    SM_Manager::GetInstance().UseDb("db222");
    SM_Manager::GetInstance().InitTbInfo();
    SM_Manager::GetInstance().ShowTb();
    tbinfos tbinfoff;
    SM_Manager::GetInstance().CreateTable("ddddddd", &tbinfoff);
    SM_Manager::GetInstance().ShowTb();
    SM_Manager::GetInstance().CreateTable("aaaaaaa", &tbinfoff);
    SM_Manager::GetInstance().ShowTb();
    SM_Manager::GetInstance().DropTable("ddddddd");
    SM_Manager::GetInstance().ShowTb();
    SM_Manager::GetInstance().CreatePK("aaaaaaa", 10);
    //SM_Manager::GetInstance().DropPK("aaaaaaa");
    SM_Manager::GetInstance().CreateFK("aaaaaaa", 10, "ddddddd");
    SM_Manager::GetInstance().DropFK("aaaaaaa", 10, "ddddddd");
    SM_Manager::GetInstance().AddColumn("aaaaaaa", AttrType::INT, "id", 10, true, 10);
    SM_Manager::GetInstance().Descv("aaaaaaa"); 
    //SM_Manager::GetInstance().DropColumn("aaaaaaa", AttrType::INT, "id");
    SM_Manager::GetInstance().ModifiedColumn("aaaaaaa", AttrType::INT, "id", AttrType::INT, "ie", 10, true, 10);
    SM_Manager::GetInstance().Descv("aaaaaaa"); 
    SM_Manager::GetInstance().CheckoutDb();

    //debug
    cout<<"success"<<endl<<endl;
    return 0;
}