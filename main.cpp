#include <iostream>
#include <sstream>

#include "rm/RecordManager.h"
#include "rm/RM_FileHandle.h"
#include "utils/MyBitMap.h"
#include "ix/IX_Manager.h"
#include "sm/SM_Manager.h"

int main(int args, char **argv)
{
    if (args <= 1)
    {
        cout << "init" << endl;
        MyBitMap::initConst();
        // test record manager
        RM_FileHandle rm;
        RecordManager::getInstance().createFile("ff", 5, 1);
        RecordManager::getInstance().openFile("ff", rm);
        char a[5] = "df";
        RID rid_one, rid_two;
        Record record;
        rm.insertRecord(a, rid_one);
        rm.insertRecord("hello", rid_two);
        rm.getRecord(rid_one, record);
        cout << "record_one geted. record.data is" << record.data << endl;
        rm.getRecord(rid_two, record);
        cout << "record_two geted. record.data is" << record.data << endl;
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
        cout << "success" << endl
             << endl;
    }
    else
    {
        char read[1000];
        cout << "please input orders:" << endl;
        while (cin.getline(read, 1000))
        {
            if (strlen(read) > 0)
            {
                cout << "not null " << strlen(read) << endl;
                istringstream str(read);
                string out;
                if (str >> out)
                {
                    if (strncmp(out.c_str(), "USE", out.size()) == 0)
                    {
                        str >> out;
                        if (out.size() != 0)
                        {
                            SM_Manager::GetInstance().UseDb(out.c_str());
                        }
                    }
                    else if (strncmp(out.c_str(), "CREATE", out.size()) == 0)
                    {
                        str >> out;
                        if (strncmp(out.c_str(), "DATABASE", out.size()) == 0)
                        {
                            str >> out;
                            SM_Manager::GetInstance().CreateDb(out.c_str());
                            SM_Manager::GetInstance().UseDb(out.c_str());
                            SM_Manager::GetInstance().InitTbInfo();
                            SM_Manager::GetInstance().CheckoutDb();
                        }
                        else if (strncmp(out.c_str(), "TABLE", out.size()) == 0)
                        {
                            str >> out;
                            if (out.size() != 0)
                            {
                                tbinfos tbinfoff;
                                string col;
                                string ll;
                                string buffer;
                                str >> ll; //(
                                while (str >> ll)
                                {
                                    strncpy(tbinfoff.attrname[tbinfoff.columns], ll.c_str(), ll.size());
                                    tbinfoff.attrsize[tbinfoff.columns] = ll.size();
                                    str >> ll;
                                    istringstream ill(ll);
                                    if (getline(ill, buffer, '('))
                                    {
                                        cout << buffer << endl;
                                        if (strncmp(buffer.c_str(), "INT", buffer.size()) == 0)
                                        {
                                            tbinfoff.colattr[tbinfoff.columns] = AttrType::INT;
                                        }
                                        else if (strncmp(buffer.c_str(), "FLOAT", buffer.size()) == 0)
                                        {
                                            tbinfoff.colattr[tbinfoff.columns] = AttrType::FLOAT;
                                        }
                                        else
                                        {
                                            tbinfoff.colattr[tbinfoff.columns] = AttrType::STRING;
                                        }
                                    }
                                    if (getline(ill, buffer, ')'))
                                    {
                                        istringstream iss(buffer);
                                        int llimit;
                                        iss>>llimit;
                                        tbinfoff.length_limit[tbinfoff.columns] = llimit;
                                        cout<<llimit<<endl;
                                    }else{
                                        tbinfoff.length_limit[tbinfoff.columns] = -1;
                                    }
                                    str >> ll;
                                    if (strncmp(ll.c_str(), "NOT", ll.size()) == 0){
                                        tbinfoff.cannull[tbinfoff.columns] = true;
                                        str >> ll;//NULL
                                        str >> ll;//, or )
                                    }
                                    tbinfoff.columns++;
                                    if (strncmp(ll.c_str(), ",", ll.size()) == 0){
                                        continue;
                                    }
                                }
                                SM_Manager::GetInstance().CreateTable(out.c_str(), &tbinfoff);
                            }
                        }
                    }
                    else if (strncmp(out.c_str(), "DROP", out.size()) == 0)
                    {
                        str >> out;
                        if (strncmp(out.c_str(), "DATABASE", out.size()) == 0)
                        {
                            str >> out;
                            SM_Manager::GetInstance().DropDb(out.c_str());
                        }
                        else if (strncmp(out.c_str(), "TABLE", out.size()) == 0)
                        {
                            str >> out;
                            SM_Manager::GetInstance().DropTable(out.c_str());
                        }
                    }
                    else if (strncmp(out.c_str(), "SHOW", out.size()) == 0)
                    {
                        str >> out;
                        if (strncmp(out.c_str(), "TABLES", out.size()) == 0)
                        {
                            SM_Manager::GetInstance().ShowTb();
                        }
                    }
                    else if (strncmp(out.c_str(), "DESC", out.size()) == 0)
                    {
                        str >> out;
                        if (out.size() != 0)
                        {
                            SM_Manager::GetInstance().Descv(out.c_str());
                        }
                    }
                    else if (strncmp(out.c_str(), "INSERT", out.size()) == 0)
                    {
                        //todo
                    }
                    else if (strncmp(out.c_str(), "DELETE", out.size()) == 0)
                    {
                        //todo
                    }
                    else if (strncmp(out.c_str(), "UPDATE", out.size()) == 0)
                    {
                        //todo
                    }
                    else if (strncmp(out.c_str(), "SELECT", out.size()) == 0)
                    {
                        //todo
                    }
                    else if (strncmp(out.c_str(), "ALTER", out.size()) == 0)
                    {
                        //mm todo
                    }
                    else
                    {
                        cout << "do not has this command" << endl;
                    }
                }
                /*while (str >> out)
                {
                    cout << "out:"<<out<<";"<< endl;
                }*/
            }
        }
    }

    return 0;
}