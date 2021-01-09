//
// Created by cyan on 2021/1/8.
//

#ifndef DB_QUERYMANAGER_H
#define DB_QUERYMANAGER_H
#include "../sm/SM_type.h"
#include "../rm/struct.h"
using namespace std;


class QueryManager {
//    int Select(const string& dbname, const vector<vector<value>>& records);

    int Insert(const string& dbname, const vector<vector<value>>& records);

//    int Update(std::string relation, SetClauseList *setClauses, Expr *whereClause);

//    int Delete(std::string relation, Expr *whereClause);
};


#endif //DB_QUERYMANAGER_H
