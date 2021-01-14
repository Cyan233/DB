//
// Created by cyan on 2021/1/8.
//

#ifndef DB_QUERYMANAGER_H
#define DB_QUERYMANAGER_H
#include "../sm/SM_type.h"
#include "../rm/struct.h"
using namespace std;


class QueryManager {
    int Select(vector<char*>& tbnames, vector<vector<Col>>& selector,
            vector<Condition_joint> &conditions_joint, vector<vector<Condition>> &conditions);

    int Insert(const string& tbname, const vector<vector<value>>& records);

    int Update(const string& tbname, vector<Condition> &where_conditions, vector<SetClause>& set_clauses);

    int Delete(const string& tbname, vector<Condition> &where_conditions);
};


#endif //DB_QUERYMANAGER_H
