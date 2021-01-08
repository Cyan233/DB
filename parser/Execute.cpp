//
// Created by cyan on 2021/1/7.
//

#include "Execute.h"
#include "../ql/QueryManager.h"
#include <utility>
#include <cstdlib>
#include <cstring>
#include "../sm/SM_Manager.h"

using std::string;
using std::vector;

#include <stdarg.h>
#define DebugPrintf


/* SelectTree */
Select::Select(IdentList *relations, Expr *whereClause) {
    this->attributes = nullptr;
    this->relations = relations;
    this->whereClause = whereClause;
    this->groupAttrName = string();
}

Select::Select(AttributeList *attributes,
               IdentList *relations,
               Expr *whereClause,
               const char *groupAttrName) {
    this->attributes = attributes;
    this->relations = relations;
    this->whereClause = whereClause;
    this->groupAttrName = string(groupAttrName);
}

Select::~Select() {
    delete attributes;
    delete relations;
    delete whereClause;
}

void Select::visit() {
    printf("select\n");
    bool statistics = false;
    if (this->attributes != nullptr) {
        for (const auto &attribute: this->attributes->attributes) {
            if (attribute->aggregationType != AggregationType::T_NONE) {
                statistics = true;
                break;
            }
        }
    }
    if (statistics) {
        for (const auto &attribute: this->attributes->attributes) {
            if (attribute->aggregationType == AggregationType::T_NONE) {
                if (not(attribute->attribute == groupAttrName)) {
                    cerr << "Can't select column " + attribute->attribute + " along with aggregating function\n";
                    return;
                }
            }
        }
    }
    QL_Manager::getInstance().exeSelect(attributes, relations, whereClause, groupAttrName);
}


/* InsertTree */
Insert::Insert(const char *relationName, ConstValueLists *insertValueTree) {
    this->relationName = string(relationName);
    this->insertValueTree = insertValueTree;
}

Insert::~Insert() {
    delete insertValueTree;
}

void Insert::execute() {
    printf("insert\n");
    QL_Manager::getInstance().exeInsert(relationName, insertValueTree);
    printf("insert complete\n");
}

/* UpdateTree */
Update::Update(string relationName,
               SetClauseList *setClauseList,
               Expr *whereClause) {
    this->relationName = std::move(relationName);
    this->setClauses = setClauseList;
    this->whereClause = whereClause;
}

Update::~Update() {
    delete setClauses;
    delete whereClause;
}

void Update::visit() {
    printf("update\n");
    QL_Manager::getInstance().exeUpdate(relationName, setClauses, whereClause);
    printf("update complete\n");
}

/* DeleteTree */
Delete::Delete(const char *relationName, Expr *whereClause) {
    this->relationName = string(relationName);
    this->whereClause = whereClause;
}

Delete::~Delete() {
    delete whereClause;
}

void Delete::visit() {
    printf("delete\n");
    QL_Manager::getInstance().exeDelete(relationName, whereClause);
    printf("delete complete\n");
}



/* ColumnNode */
ColumnNode::ColumnNode(const char *columnName, AttrType type, int length,
                       int columnFlag) {
    this->columnName = string(columnName);
    this->type = type;
    this->length = length;
    this->columnFlag = columnFlag;
    switch (type) {
        case AttrType::DATE:
        case AttrType::INT:
            this->size = sizeof(int);
            break;
        case AttrType::FLOAT:
            this->size = sizeof(float);
            break;
        case AttrType::VARCHAR:
        case AttrType::STRING:
            this->size = length + 1;
            break;
        case AttrType::BOOL:
        case AttrType::NO_ATTR:
            break;
    }
}

ColumnNode::~ColumnNode() = default;

AttrInfo ColumnNode::getAttrInfo() const {
    AttrInfo attrInfo{};
    strncpy(attrInfo.attrName, columnName.c_str(), MAX_NAME);
    attrInfo.attrType = type;
    attrInfo.attrSize = size;
    attrInfo.attrLength = length;
    attrInfo.columnFlag = columnFlag;
    return attrInfo;
}


/* AttributeList */
AttributeList::AttributeList() = default;

AttributeList::~AttributeList() {
    for (const auto &attribute : attributes)
        delete attribute;
}

void AttributeList::addAttribute(AttributeNode *attribute) {
    attributes.push_back(attribute);
}

vector<AttributeNode::AttributeDescriptor> AttributeList::getDescriptors() const {
    vector<AttributeNode::AttributeDescriptor> attrs;
    for (auto attribute : attributes)
        attrs.push_back(attribute->getDescriptor());
    return attrs;
}

/* AttributeTree */
AttributeNode::AttributeNode(const char *relationName, const char *attributeName) {
        this->table = string(relationName);
        this->attribute = string(attributeName);
}

AttributeNode::AttributeNode(const char *attributeName) {
    this->attribute = string(attributeName);
}

AttributeNode::AttributeDescriptor AttributeNode::getDescriptor() const {
    return AttributeDescriptor(table, attribute);
}

bool AttributeNode::operator==(const AttributeNode &attribute) const {
    return this->table == attribute.table &&
           this->attribute == attribute.attribute ;
}

AttributeNode::~AttributeNode() = default;


/* ConstValuesTree */
ConstValueList::ConstValueList() = default;

ConstValueList::~ConstValueList() {
    for (const auto &constValue: constValues)
        delete constValue;
}

void ConstValueList::addConstValue(Expr *constValue) {
    constValues.push_back(constValue);
}

ConstValueLists::ConstValueLists() = default;

ConstValueLists::~ConstValueLists() {
    for (auto v : values) {
        delete v;
    }
}

void ConstValueLists::addConstValues(ConstValueList *constValuesTree) {
    values.push_back(constValuesTree);
}

SetClauseList::SetClauseList() = default;

void SetClauseList::addSetClause(AttributeNode *attr, Expr *value) {
    this->clauses.emplace_back(attr, value);
}

IdentList::IdentList() = default;

void IdentList::addIdent(const char *ident) {
    this->idents.emplace_back(ident);
}

IdentList::~IdentList() = default;

ColumnDecsList::ColumnDecsList() = default;

ColumnDecsList::~ColumnDecsList() {
    for (auto v: columns) {
        delete v;
    }
}

void ColumnDecsList::addColumn(ColumnNode *column) {
    columns.push_back(column);
}

int ColumnDecsList::getColumnCount() {
    return columns.size();
}

AttrInfo *ColumnDecsList::getAttrInfos() {
    if (attrInfos == nullptr) {
        attrInfos = new AttrInfo[columns.size()];
        for (int i = 0; i < columns.size(); ++i) {
            attrInfos[i] = columns[i]->getAttrInfo();
        }
    }
    return attrInfos;
}

void ColumnDecsList::deleteAttrInfos() {
    delete attrInfos;
}

std::pair<AttrInfo, int> ColumnDecsList::findColumn(std::string name) {
    int offset = 0;
    for (const auto &it: columns) {
        if (it->columnName == name) {
            return {it->getAttrInfo(), offset};
        } else {
            offset += it->size;
        }
    }
    return {AttrInfo{}, -1};
}

// Primary key
TableConstraint::TableConstraint(IdentList *column_list) : type(ConstraintType::PRIMARY_CONSTRAINT),
                                                           column_list(column_list) {

}

// Foreign key
TableConstraint::TableConstraint(const char *column_name, const char *table, const char *column) : type(
        ConstraintType::FOREIGN_CONSTRAINT),
                                                                                                   column_name(
                                                                                                           column_name),
                                                                                                   foreign_table(table),
                                                                                                   foreign_column(
                                                                                                           column) {

}

// Check key
TableConstraint::TableConstraint(const char *column_name, ConstValueList *const_values) : type(
        ConstraintType::CHECK_CONSTRAINT), const_values(const_values), column_name(column_name) {

}

TableConstraint::~TableConstraint() {
    delete column_list;
    delete const_values;
}

void TableConstraintList::addTbDec(TableConstraint *dec) {
    tbDecs.push_back(dec);
}

TableConstraintList::~TableConstraintList() {
    for (auto v: tbDecs) {
        delete v;
    }
}
