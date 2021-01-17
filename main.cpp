#include <iostream>
#include <sstream>
#include <string>
using namespace std;

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
    value v;v.i = 5;rec.push_back(v);v.f = 1.2;rec.push_back(v);
    records.push_back(rec);
//    rec[0].i = 10; rec[1].f = 5.5;
//    records.push_back(rec);
    qm.Insert("query", records);
    cout<<"insert"<<endl;
//    vector<char*> tbnames;
//    tbnames.push_back("query");
//    vector<vector<Col>> selector;
//    vector<Col> sel;
//    selector.push_back(sel);
//    vector<Condition_joint> conditions_joint;
//    vector<vector<Condition>> conditions;
//    vector<Condition> a;
//    conditions.push_back(a);
//    vector<Condition> where;
//    Condition w; strcpy(w.col, "id"); w.compOp = CompOp::EQ_OP; w.compare_value.i = 5;
//    where.push_back(w);
//    conditions.push_back(where);
//    qm.Select(tbnames, selector, conditions_joint, conditions);
    cout<<"*****"<<endl;

    // delete
//    qm.Delete("query", where);
//    vector<SetClause> st;
//    SetClause cl;strcpy(cl.attr_name, "id"); cl.set_value.i = 20;
//    st.push_back(cl);
//    qm.Update("query", where,st);
//    conditions.push_back(where);
//    qm.Select(tbnames, selector, conditions_joint, conditions);
//    tbinfo.columns = 2;
//    tbinfo.colattr[0] = AttrType::INT;
//    tbinfo.colattr[1] = AttrType::INT;
//    tbinfo.attrsize[0] = tbinfo.attrsize[0] = 4;
//    strcpy(tbinfo.attrname[0],"s_id");
//    strcpy(tbinfo.attrname[1],"t_id");
//    SM_Manager::GetInstance().CreateTable("id", &tbinfo);
//    SM_Manager::GetInstance().ShowTb();
//    records.clear();
//    v.i = 5;rec.push_back(v);v.i = 10;rec.push_back(v);
//    records.push_back(rec);
//    rec[0].i = 10;rec[1].i=100;
//    records.push_back(rec);
//    qm.Insert("id", records);
//    tbnames.push_back("id");
//    vector<Col> sel2;
//    Col col; strcpy(col.attr_name,"s_id");
//    sel2.push_back(col);
//    selector.push_back(sel2);
//    vector<Condition> c2;
//    conditions.push_back(c2);
//    qm.Select(tbnames, selector, conditions_joint, conditions);
    SM_Manager::GetInstance().DropTable("query");
}

Condition parseWhere(string s){
    Condition where = Condition();
    int position = 0;
    if(s.find('=')!=string::npos){
        position = s.find('=');
        where.compOp = CompOp ::EQ_OP;
    } else if(s.find('<')!=string::npos){
        position = s.find('<');
        where.compOp = CompOp ::EQ_OP;
    } else if(s.find('>')!=string::npos){
        position = s.find('>');
        where.compOp = CompOp ::EQ_OP;
    }
    strcpy(where.col, s.substr(0, position).c_str());
    s = s.substr(position);
    where.compare_value.i = atoi(s.c_str());
    where.compare_value.f =  atof(s.c_str());
    strcpy(where.compare_value.s, s.c_str());
    return where;
}

int main(int args, char **argv)
{
    QueryManager qm;
    if (args <= 1)
    {
        cout << "init" << endl;
        MyBitMap::initConst();
        test_query();
//        // test record manager
//        RM_FileHandle rm;
//        RecordManager::getInstance().createFile("ff", 5, 1);
//        RecordManager::getInstance().openFile("ff", rm);
//        char a[5] = "df";
//        RID rid_one, rid_two;
//        Record record;
//        rm.insertRecord(a, rid_one);
//        rm.insertRecord("hello", rid_two);
//        rm.getRecord(rid_one, record);
//        cout << "record_one geted. record.data is" << record.data << endl;
//        rm.getRecord(rid_two, record);
//        cout << "record_two geted. record.data is" << record.data << endl;
//        //rm.deleteRecord(rid_two);
//        RecordManager::getInstance().closeFile(rm);
//        //RecordManager::getInstance().destroyFile("ff");
//
//        //test ix manager
//        IX_Manager::GetInstance().CreateIndex("ff", 1, AttrType::INT, 10, 10);
//        IX_indexHandle indexhandle;
//        IX_Manager::GetInstance().OpenIndex("ff", 1, indexhandle);
//        IX_Manager::GetInstance().CloseIndex(indexhandle);
//        IX_Manager::GetInstance().DestroyIndex("ff", 1);
//
//        //test sm manager
//        SM_Manager::GetInstance().initDbInfo();
//        SM_Manager::GetInstance().ShowDb();
//        SM_Manager::GetInstance().CreateDb("db111");
//        SM_Manager::GetInstance().ShowDb();
//        SM_Manager::GetInstance().CreateDb("db222");
//        SM_Manager::GetInstance().ShowDb();
//        SM_Manager::GetInstance().DropDb("db111");
//        SM_Manager::GetInstance().ShowDb();
//        SM_Manager::GetInstance().UseDb("db222");
//        SM_Manager::GetInstance().InitTbInfo();
//        SM_Manager::GetInstance().ShowTb();
//        tbinfos tbinfoff;
//        SM_Manager::GetInstance().CreateTable("ddddddd", &tbinfoff);
//        SM_Manager::GetInstance().ShowTb();
//        SM_Manager::GetInstance().CreateTable("aaaaaaa", &tbinfoff);
//        SM_Manager::GetInstance().ShowTb();
//        SM_Manager::GetInstance().DropTable("ddddddd");
//        SM_Manager::GetInstance().ShowTb();
//        SM_Manager::GetInstance().CreatePK("aaaaaaa", 10);
//        //SM_Manager::GetInstance().DropPK("aaaaaaa");
//        SM_Manager::GetInstance().CreateFK("aaaaaaa", 10, "ddddddd");
//        SM_Manager::GetInstance().DropFK("aaaaaaa", 10, "ddddddd");
//        SM_Manager::GetInstance().AddColumn("aaaaaaa", AttrType::INT, "id", 10, true, 10);
//        SM_Manager::GetInstance().Descv("aaaaaaa");
//        //SM_Manager::GetInstance().DropColumn("aaaaaaa", AttrType::INT, "id");
//        SM_Manager::GetInstance().ModifiedColumn("aaaaaaa", AttrType::INT, "id", AttrType::INT, "ie", 10, true, 10);
//        SM_Manager::GetInstance().Descv("aaaaaaa");
//        SM_Manager::GetInstance().CheckoutDb();

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
                                        iss >> llimit;
                                        tbinfoff.length_limit[tbinfoff.columns] = llimit;
                                        cout << llimit << endl;
                                    }
                                    else
                                    {
                                        tbinfoff.length_limit[tbinfoff.columns] = -1;
                                    }
                                    str >> ll;
                                    if (strncmp(ll.c_str(), "NOT", ll.size()) == 0)
                                    {
                                        tbinfoff.cannull[tbinfoff.columns] = false;
                                        str >> ll; //NULL
                                        str >> ll; //, or )
                                    }else{
                                        tbinfoff.cannull[tbinfoff.columns] = true;
                                    }
                                    tbinfoff.columns++;
                                    if (strncmp(ll.c_str(), ",", ll.size()) == 0)
                                    {
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
                    else if (strncmp(out.c_str(), "INSERT", out.size()) == 0)  //INSERT INTO nation VALUES (0,'America');
                    {
                        str>>out;
                        string tbname;
                        str>>tbname;
                        cout<<tbname;
                        str>>out;
                        string values;
                        vector<vector<value>> records;
                        vector<value> record;
                        RM_FileHandle file_handle;
                        int rc = RecordManager::getInstance().openFile(tbname, file_handle);
                        TEST_RC_NOT_ZERO_ERROR
                        tbinfos tb_info = file_handle.getHeader().tb_info;
                        int i=0;
                        while(getline(str,values,',')){
                            if(values[0]==' ') values = values.substr(1, values.length()-1);
                            if(values[0]=='(') values = values.substr(1, values.length()-1);
                            if(values[values.length()-1]==' ') values = values.substr(0, values.length()-1);
                            if(values[values.length()-1]==')') values = values.substr(0, values.length()-1);
                            if(values[values.length()-1]==';') values = values.substr(0, values.length()-2);
                            cout<<values;
                            value _value{};
                            if(tb_info.colattr[i]==AttrType::INT){
                                _value.i =  atoi(values.c_str());
                            }else if(tb_info.colattr[i]==AttrType::FLOAT){
                                _value.f =  atof(values.c_str());
                            }else{
                                strcpy(_value.s, values.c_str());
                            }
                            record.push_back(_value);
                            i++;
                        }
                        cout<<endl;
                        records.push_back(record);
                        qm.Insert(tbname, records);
                    }
                    else if (strncmp(out.c_str(), "DELETE", out.size()) == 0) // DELETE FROM customer WHERE c_custkey=5;
                    {
                        string tbname;
                        str>>out;
                        str>>tbname;
                        cout<<tbname;
                        str>>out;
                        string where;
                        vector<Condition> wheres;
                        while(getline(str,where,',')){
                            if(where[where.length()-1]==';') where = where.substr(0, where.length()-1);
                            wheres.push_back(parseWhere(where));
                        }
                        qm.Delete(tbname, wheres);
                    }
                    else if (strncmp(out.c_str(), "UPDATE", out.size()) == 0)  // UPDATE nation SET n_regionkey = 316001 WHERE n_nationkey=15;
                    {
                        string tbname;
                        str>>tbname;
                        cout<<tbname;
                        //set
                        str>>out;
                        SetClause setcl;
                        str>>setcl.attr_name;
                        str>>out; str>>out;
                        setcl.set_value.i = atoi(out.c_str());
                        setcl.set_value.f =  atof(out.c_str());
                        strcpy(setcl.set_value.s, out.c_str());
                        vector<SetClause> setcls;
                        setcls.push_back(setcl);
                        //where
                        str>>out;
                        string where;
                        vector<Condition> wheres;
                        while(getline(str,where,',')){
                            if(where[where.length()-1]==';') where = where.substr(0, where.length()-1);
                            wheres.push_back(parseWhere(where));
                        }
                        qm.Delete(tbname, wheres);
                    }
                    else if (strncmp(out.c_str(), "SELECT", out.size()) == 0)
                        // SELECT o_orderdate,o_totalprice FROM orders,uu WHERE o_orderdate='1996-01-02';
                    {
                        vector<char*> tbnames;
                        string cols;
                        str>>cols;
                        str>>out;
                        str>>out; // tbnames
                        vector<vector<Col>> selector;
                        int last_pos=0,pos;
                        while((pos=out.find(','))!=string::npos){  //每个表
                            char* cur = const_cast<char *>(out.substr(last_pos, pos - last_pos).c_str());
                            tbnames.push_back(cur);
                            last_pos = pos+1;
                            vector<Col> select;
                            selector.push_back(select);
                        }
                        last_pos=0;
                        while((pos=cols.find(','))!=string::npos){
                            string sub = cols.substr(last_pos, pos-last_pos);
                            if(sub.find('.')==string::npos){
                                Col col;
                                strcpy(col.attr_name, sub.c_str());
                                selector[0].push_back(col);
                            }else{
                                Col col;
                                int cur_pos = sub.find('.');
                                strcpy(col.tb_name, sub.substr(0, cur_pos).c_str());
                                strcpy(col.attr_name, sub.substr(cur_pos+1).c_str());
                                for(int i=0;i<tbnames.size();i++){
                                    if(strcmp(tbnames[i], col.tb_name)==0){
                                        selector[i].push_back(col);
                                        break;
                                    }
                                }
                            }
                            last_pos = pos+1;
                        }
                        //where
                        str>>out;
                        string where;
                        vector<vector<Condition>> wheress;
                        vector<Condition> wheres;
                        while(getline(str,where,',')){
                            if(where[where.length()-1]==';') where = where.substr(0, where.length()-1);
                            wheres.push_back(parseWhere(where));
                        }
                        wheress.push_back(wheres);
                        vector<Condition_joint> cj;
                        qm.Select(tbnames, selector, cj, wheress);
                    }
                    else if (strncmp(out.c_str(), "ALTER", out.size()) == 0)
                    {
                        str >> out;
                        if (strncmp(out.c_str(), "TABLE", out.size()) == 0)
                        {
                            string tablename;
                            str >> tablename;
                            cout<<"tablename is "<<tablename<<endl;
                            str >> out;
                            if (strncmp(out.c_str(), "ADD", out.size()) == 0)
                            {
                                str >> out;
                                if (strncmp(out.c_str(), "COLUMN", out.size()) == 0)
                                {
                                    string column_name;
                                    str >> column_name;
                                    str >> out;
                                    if (strncmp(out.c_str(), ";", out.size()) != 0)
                                    {
                                        string ll;
                                        string buffer;
                                        str >> ll;
                                        istringstream ill(ll);
                                        AttrType at;
                                        int llimit;
                                        bool cannull;
                                        if (getline(ill, buffer, '('))
                                        {
                                            cout << buffer << endl;
                                            if (strncmp(buffer.c_str(), "INT", buffer.size()) == 0)
                                            {
                                                at = AttrType::INT;
                                            }
                                            else if (strncmp(buffer.c_str(), "FLOAT", buffer.size()) == 0)
                                            {
                                                at = AttrType::FLOAT;
                                            }
                                            else
                                            {
                                                at = AttrType::STRING;
                                            }
                                        }
                                        if (getline(ill, buffer, ')'))
                                        {
                                            istringstream iss(buffer);
                                            int llimit;
                                            iss >> llimit;
                                            cout << llimit << endl;
                                        }
                                        else
                                        {
                                            llimit = -1;
                                        }
                                        str >> ll;
                                        if (strncmp(ll.c_str(), "NOT", ll.size()) == 0)
                                        {
                                            str >> ll; //NULL
                                            str >> ll; //, or )
                                        }
                                        cout<<"addcolumn"<<endl;
                                        SM_Manager::GetInstance().AddColumn(tablename.c_str(), at, column_name.c_str(), 6, 0, llimit);
                                    }
                                }
                                else if (strncmp(out.c_str(), "PRIMARY", out.size()) == 0)
                                {
                                    str >> out;
                                    str >> out;
                                    string colname;
                                    str >> colname;
                                    SM_Manager::GetInstance().CreatePK(tablename.c_str(), colname.c_str());
                                }
                                else if (strncmp(out.c_str(), "FOREIGN", out.size()) == 0)
                                {
                                    str >> out;
                                    str >> out;
                                    str >> out;
                                    str >> out;
                                    str >> out;
                                    string buffer;
                                    istringstream ddd(out);
                                    getline(ddd, buffer, '(');
                                    SM_Manager::GetInstance().CreateFK(tablename.c_str(), 10, buffer.c_str());
                                }
                                else if (strncmp(out.c_str(), "INDEX", out.size()) == 0)
                                {
                                    str >> out;
                                    string buffer;
                                    string nameee;
                                    istringstream ddd(out);
                                    getline(ddd, buffer, '(');
                                    getline(ddd, nameee, ')');
                                    IX_Manager::GetInstance().CreateIndex(tablename.c_str(), buffer.size(), AttrType::INT, 6, 6);
                                }
                                else
                                {
                                    //todo nothing
                                }
                            }
                            else if (strncmp(out.c_str(), "DROP", out.size()) == 0)
                            {
                                str >> out;
                                if (strncmp(out.c_str(), "COLUMN", out.size()) == 0)
                                {
                                    string column_name;
                                    str >> column_name;
                                    cout<<"drop column"<<endl;
                                    SM_Manager::GetInstance().DropColumn(tablename.c_str(), AttrType::INT, column_name.c_str());
                                }
                                else if (strncmp(out.c_str(), "PRIMARY", out.size()) == 0)
                                {
                                    SM_Manager::GetInstance().DropPK(tablename.c_str());
                                }
                                else if (strncmp(out.c_str(), "FOREIGN", out.size()) == 0)
                                {
                                    SM_Manager::GetInstance().DropFK(tablename.c_str(), 10, "ddddddd");
                                }
                                else if (strncmp(out.c_str(), "INDEX", out.size()) == 0)
                                {
                                    str >> out;
                                    IX_Manager::GetInstance().DestroyIndex(tablename.c_str(), out.size());
                                }
                                else
                                {
                                    //todo nothing
                                }
                            }
                            else if (strncmp(out.c_str(), "RENAME", out.size()) == 0)
                            {
                                str >> out;
                                string newname;
                                str >> newname;
                                char cmd[200];
                                sprintf(cmd, "mv %s %s", tablename.c_str(), newname.c_str());
                                system(cmd);
                            }
                        }
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