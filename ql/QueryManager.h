
#ifndef MYDB_QL_MANAGER_H
#define MYDB_QL_MANAGER_H

//#include "../parser/Tree.h"
#include "../rm/struct.h"
#include "../rm/RM_FileHandle.h"
#include "../sm/SM_Manager.h"
#include "Table.h"
#include "../Constants.h"
#include <functional>
#include <memory>
#include <list>

class QL_Manager {
private:
    SM_Manager sm;

    vector<Record> recordCaches;

    using CallbackFunc = function<void(const Record &)>;

    using MultiTableFunc = function<void(const vector<Record> &)>;


    void printException(const AttrBindException &exception);

    int iterateTables(Table &table, Expr *condition, CallbackFunc callback);

    int iterateTables(vector<unique_ptr<Table>> &tables, int current, Expr *condition, MultiTableFunc callback,
                  list<Expr *> &indexExprs);

    void bindAttribute(Expr *expr, const vector<unique_ptr<Table>> &tables);

    int openTables(const vector<string> &tableNames, vector<unique_ptr<Table>> &tables);

    int whereBindCheck(Expr *expr, vector<unique_ptr<Table>> &tables);

public:
    // select attributes on relations with where condition and group attributes
    int exeSelect(vector<AttributeNode *>& attributes, vector<std::string>& relations, Expr *whereClause, const string &groupAttrName);

    // insert into relation table(可以用columnList指定要插入的列)
    int exeInsert(string relation, vector<vector<Expr *>>& insertValueTree);

    // update relation table as setClauses with where condition
    int exeUpdate(string relation, vector<std::pair<AttributeNode *, Expr *>>& setClauses, Expr *whereClause);

    // delete the data in relation table with where condition
    int exeDelete(string relation, Expr *whereClause);

    static QL_Manager &getInstance();
};

#define QL_TABLE_FAIL (START_QL_WARN + 1)
#define QL_TYPE_CHECK (START_QL_WARN + 2)

#endif //MYDB_QL_MANAGER_H
