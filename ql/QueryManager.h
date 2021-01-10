//
// Created by cyan on 2021/1/8.
//

#ifndef DB_QUERYMANAGER_H
#define DB_QUERYMANAGER_H
#include "../sm/SM_type.h"
#include "../rm/struct.h"
using namespace std;


class QueryManager {
    int Select(vector<string>& dbnames, vector<Condition_joint> &where_conditions, vector<Col>& selector);

    int Insert(const string& dbname, const vector<vector<value>>& records);

    int Update(const string& dbname, vector<Condition> &where_conditions, vector<SetClause>& set_clauses);

    int Delete(const string& dbname, vector<Condition> &where_conditions);
};


#endif //DB_QUERYMANAGER_H
