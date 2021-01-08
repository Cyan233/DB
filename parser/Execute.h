//
// Created by cyan on 2021/1/7.
//

#ifndef DB_EXECUTE_H
#define DB_EXECUTE_H


#include <utility>
#include <string>
#include <vector>
#include <cassert>
#include "../Constants.h"
#include "Expr.h"
using namespace std;

class Tree;

//Sys Statement
class ShowDatabase;

// Table Statement
class Select;

class Insert;

class Update;

class Delete;

// DataType
class ColumnDecsList;

class ColumnNode;

class AttributeList;

class AttributeNode;

class IdentList;

class ConstValueList;

class ConstValueLists;

class SetClauseList;

class TableConstraint;

class TableConstraintList;



class Select {
public:
    Select(AttributeList *attributes, IdentList *relations, Expr *whereClause, const char *groupAttrName = "");

    Select(IdentList *relations, Expr *whereClause);

    ~Select();

    void visit();

    AttributeList *attributes;
    IdentList *relations;
    Expr *whereClause;
    string groupAttrName;
};


class Insert {
public:
    Insert(const char *relationName, ConstValueLists *insertValueTree);

    ~Insert() ;

    void execute() ;

    string relationName;
    IdentList * columnList;
    ConstValueLists *insertValueTree;
};


class Update {
public:
    Update(string relationName,
           SetClauseList *setClauses,
           Expr *whereClause);

    ~Update() ;

    void visit() ;

    string relationName;
    SetClauseList *setClauses;
    Expr *whereClause;
};


class Delete {
public:
    Delete(const char *relationName, Expr *whereClause);

    ~Delete() ;

    void visit() ;

    string relationName;
    Expr *whereClause;
};

class ColumnDecsList {
public:
    ColumnDecsList();

    ~ColumnDecsList() ;

    void addColumn(ColumnNode *);

    int getColumnCount();

    AttrInfo *getAttrInfos();

    void deleteAttrInfos();

    pair<AttrInfo, int> findColumn(string name);

    vector<ColumnNode *> columns;
    AttrInfo *attrInfos = nullptr;
};


class ColumnNode {
public:
    ColumnNode(const char *columnName, AttrType type, int length = 4,
               int columnFlag = 0);

    ~ColumnNode() ;

    AttrInfo getAttrInfo() const;

    friend class ColumnDecsList;

    string columnName;
    AttrType type;
    int size;
    int length;
    int columnFlag;
};

class AttributeList {
public:
    AttributeList();

    ~AttributeList() ;

    void addAttribute(AttributeNode *attribute);

    vector<AttributeNode::AttributeDescriptor> getDescriptors() const;

    vector<AttributeNode *> attributes;
};


class IdentList {
public:
    IdentList();

    ~IdentList() ;

    void addIdent(const char *ident);

    vector<string> idents;
};

class ConstValueList {
public:
    ConstValueList();

    ~ConstValueList();

    void addConstValue(Expr *constValue);

//    vector<AttrValue> getConstValues();

    vector<Expr *> constValues;
};

// For insert values
class ConstValueLists {
public:
    explicit ConstValueLists();

    ~ConstValueLists() ;

    void addConstValues(ConstValueList *constValuesTree);

    vector<ConstValueList *> values;
};

class SetClauseList {
public:
    SetClauseList();

    void addSetClause(AttributeNode *, Expr *);

    vector<pair<AttributeNode *, Expr *>> clauses;
};

class TableConstraint {
public:
    explicit TableConstraint(IdentList *column_list);

    TableConstraint(const char *column_name, const char *table, const char *column);

    TableConstraint(const char *column_name, ConstValueList *const_values);

    ~TableConstraint() ;

    ConstraintType type;
    ConstValueList *const_values = nullptr;
    IdentList *column_list = nullptr;
    string column_name;
    string foreign_table;
    string foreign_column;
};

class TableConstraintList {
public:
    TableConstraintList() = default;

    ~TableConstraintList() ;

    void addTbDec(TableConstraint *dec);

    vector<TableConstraint *> tbDecs;
};


#endif //DB_EXECUTE_H
