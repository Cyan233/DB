#include <utility>

#include <memory>
#include <map>
#include <utility>

#include "QueryManager.h"
#include "../rm/RM_FileHandle.h"
#include "../rm/RecordManager.h"
#include "../rm/RM_FileScan.h"
#include "../parser/Expr.h"

#include <set>


int QL_Manager::openTables(const std::vector<string> &tableNames, std::vector<std::unique_ptr<Table>> &tables) {
    try {
        for (const auto &ident : tableNames) {
            tables.push_back(std::unique_ptr<Table>(new Table(ident)));
        }
    }
    catch (const string &error) {
        cerr << error;
        return QL_TABLE_FAIL;
    }
    return 0;
}

int QL_Manager::whereBindCheck(Expr *whereClause, std::vector<std::unique_ptr<Table>> &tables) {
    try {
        bindAttribute(whereClause, tables);
        whereClause->type_check();
        if (whereClause->dataType != AttrType::BOOL) {
            if (whereClause->dataType != AttrType::NO_ATTR) {
                cerr << "where clause type error\n";
            }
            return QL_TYPE_CHECK;
        }
        return 0;
    } catch (AttrBindException &exception) {
        printException(exception);
        return QL_TABLE_FAIL;
    }
}

std::set<int> collectTable(Expr &expr, const std::set<int> &before) {
    //return the tables in expr
    std::set<int> tableNums;
    expr.postorder([&tableNums, &before](Expr *expr) -> void {
        if (expr->nodeType == NodeType::ATTR_NODE) {
            if (before.find(expr->tableIndex) == before.end()) {
                tableNums.insert(expr->tableIndex);
            }
        }
    });
    return tableNums;
}

void optimizeIteration(std::vector<std::unique_ptr<Table>> &tables, Expr *condition, std::vector<int> &iterationRank,
                       std::list<Expr *> &indexExprs, std::set<int> &before) {
    std::set<int> bestRight;
    Expr *indexAim = nullptr;
    Expr *current = condition;
    while (current != nullptr) {
        Expr *aim;
        if (current->calculated) {
            break;
        }
        if (current->nodeType == NodeType::COMP_NODE) {
            aim = current;
        } else if (current->nodeType == NodeType::LOGIC_NODE) {
            if (current->oper.logic != LogicOp::AND_OP) {
                break;
            }
            aim = current->right;
        } else {
            break;
        }
        if (not aim->calculated and aim->nodeType == NodeType::COMP_NODE and isComparison(aim->oper.comp) and
            aim->left->nodeType == NodeType::ATTR_NODE) {
            // the left of comparison must be attribute
            // the left has not been iterated
            if (before.find(aim->left->tableIndex) == before.end()) {
                Table &table = *tables[aim->left->tableIndex];
                // the left is indexed
                if (table.getIndexAvailable(aim->left->columnIndex)) {
                    std::set<int> rightTables = collectTable(*aim->right, before);
                    // the left table not appear in right
                    if (rightTables.find(aim->left->tableIndex) == rightTables.end() and
                        (indexAim == nullptr or rightTables.size() < bestRight.size())) {
                        indexAim = aim;
                        bestRight = rightTables;
                    }
                }
            }
        }
        current = current->left;
    }
    if (indexAim != nullptr) {
        for (auto &index: bestRight) {
            iterationRank.push_back(index);
            before.insert(index);
        }
        iterationRank.push_back(indexAim->left->tableIndex);
        before.insert(indexAim->left->tableIndex);
        indexExprs.push_back(indexAim);
        optimizeIteration(tables, condition, iterationRank, indexExprs, before);
    } else {
        for (int i = 0; i < tables.size(); ++i) {
            if (before.find(i) == before.end()) {
                iterationRank.push_back(i);
            }
        }
    }
}


void QL_Manager::printException(const AttrBindException &exception) {
    switch (exception.type) {
        case EXPR_NO_SUCH_TABLE:
            fprintf(stderr, "Can't find the table %s\n", exception.table.c_str());
            break;
        case EXPR_NO_SUCH_ATTRIBUTE:
            fprintf(stderr, "Can't find the attribute %s in table %s\n", exception.attribute.c_str(),
                    exception.table.c_str());
            break;
        case EXPR_AMBIGUOUS:
            fprintf(stderr, "Reference to the attribute %s is ambiguous\n", exception.attribute.c_str());
            break;
        default:
            fprintf(stderr, "Unknown error\n");
            break;
    }
}

int QL_Manager::iterateTables(Table &table, Expr *condition, QL_Manager::CallbackFunc callback) {
    int rc;
    RM_FileScan fileScan;
    IX_FindSpeRec indexScan;
    bool use_index = false;
    Expr *current = condition;
    Expr *aim;
    while (current != nullptr) {
        if (current->calculated) {
            break;
        }
        if (current->nodeType == NodeType::COMP_NODE) {
            aim = current;
        } else if (current->nodeType == NodeType::LOGIC_NODE) {
            if (current->oper.logic != LogicOp::AND_OP) {
                break;
            }
            aim = current->right;
        } else {
            break;
        }
        // the left of comparison must be attribute
        if (aim->left->nodeType == NodeType::ATTR_NODE and aim->right->calculated) {
            if (aim->left->attrInfo.tableName == table.tableName and table.getIndexAvailable(aim->left->columnIndex) and
                isComparison(aim->oper.comp)) {
                indexScan.OpenScan(*table.indexHandles[aim->left->columnIndex], aim->oper.comp,
                                   aim->right->getValue());
                use_index = true;
                break;
            }
        }
        current = current->left;
    }
    if (not use_index) {
        fileScan.startScan(table.fileHandle, nullptr, table.tableName);
    }
    while (true) {
        Record record;
        if (use_index) {
            RID rid;
            rc = indexScan.FindNextRec(*table.indexHandles[aim->left->columnIndex],rid);
            if (rc == 0) {
                table.fileHandle.getRecord(rid, record);
            }
        } else {
            rc = fileScan.getNextRecord(record);
        }
        if (rc) {
            break;
        }
        condition->calculate(record.data, table.tableName);
        if (condition->is_true())
            callback(record);
        condition->init_calculate(table.tableName);
    }
    return 0;
}

void QL_Manager::bindAttribute(Expr *expr, const std::vector<std::unique_ptr<Table>> &tables) {
    expr->postorder([&tables](Expr *expr1) {
        if (expr1->nodeType == NodeType::ATTR_NODE) {
            if (!expr1->attribute->table.empty()) {
                int tableIndex = 0;
                for (const auto &table: tables) {
                    if (table->tableName == expr1->attribute->table) {
                        int index = table->getColumnIndex(expr1->attribute->attribute);
                        if (index >= 0) {
                            expr1->attrInfo = table->attrInfos[index];
                            expr1->dataType = expr1->attrInfo.attrType;
                            expr1->tableIndex = tableIndex;
                            expr1->columnIndex = index;
                            return;
                        } else {
                            throw AttrBindException{"", expr1->attribute->attribute, EXPR_NO_SUCH_ATTRIBUTE};
                        }
                    }
                    tableIndex += 1;
                }
                throw AttrBindException{expr1->attribute->table, "", EXPR_NO_SUCH_TABLE};
            } else {
                expr1->attrInfo.attrOffset = -1;
                int tableIndex = 0;
                for (const auto &table: tables) {
                    int index = table->getColumnIndex(expr1->attribute->attribute);
                    if (index >= 0) {
                        if (not expr1->attrInfo.tableName.empty()) {
                            throw AttrBindException{"", expr1->attribute->attribute, EXPR_AMBIGUOUS};
                        } else {
                            expr1->attrInfo = table->attrInfos[index];
                            expr1->dataType = expr1->attrInfo.attrType;
                            expr1->tableIndex = tableIndex;
                            expr1->columnIndex = index;
                        }
                    }
                    tableIndex += 1;
                }
                if (expr1->attrInfo.tableName.empty()) {
                    throw AttrBindException{"", expr1->attribute->attribute, EXPR_NO_SUCH_ATTRIBUTE};
                }
            }
        }
    });
}

QL_Manager &QL_Manager::getInstance() {
    static QL_Manager instance;
    return instance;
}

int QL_Manager::iterateTables(std::vector<std::unique_ptr<Table>> &tables, int current, Expr *condition,
                              QL_Manager::MultiTableFunc callback, std::list<Expr *> &indexExprs) {
    if (current == tables.size()) {
        callback(recordCaches);
        return 0;
    }
    int rc;
    RM_FileScan fileScan;
    IX_FindSpeRec indexScan;
    Table &table = *tables[current];
    Expr *indexExpr = nullptr;
    bool use_index = false;
    if (!indexExprs.empty() and indexExprs.front()->left->tableIndex == current) {
        indexExpr = indexExprs.front();
        indexExprs.pop_front();
        indexScan.OpenScan(*table.indexHandles[indexExpr->left->columnIndex], indexExpr->oper.comp,
                           indexExpr->right->getValue());
        use_index = true;
    } else {
        fileScan.startScan(table.fileHandle, nullptr, table.tableName);
    }
    while (true) {
        Record record;
        if (use_index) {
            RID rid;
            rc = indexScan.FindNextRec(*table.indexHandles[indexExpr->left->columnIndex],rid);
            if (rc == 0) {
                if (table.fileHandle.getRecord(rid, record) != 0) {
                    cerr << "Can't find an entry in the index\n";
                    return -1;
                }
            }
        } else {
            rc = fileScan.getNextRecord(record);
        }
        if (rc) {
            break;
        }
        condition->calculate(record.data, table.tableName);
        if (not condition->calculated or condition->is_true()) {
            recordCaches.push_back(std::move(record));
            iterateTables(tables, current + 1, condition, callback, indexExprs);
            recordCaches.pop_back();
        }
        condition->init_calculate(table.tableName);
    }
    if (indexExpr != nullptr) {
        indexExprs.push_back(indexExpr);
    }
    return 0;
}


int QL_Manager::exeSelect(vector<AttributeNode *>& attributes, vector<std::string>& relations, Expr *whereClause, const string &groupAttrName)
{
    // if groupAttrName not empty, there must be statistics and only one non-statistics attribute (checked before)
    // statistics can't appear along with normal attribute (checked before)
    // aggregate can only appear for numeric column
    std::vector<std::unique_ptr<Table>> tables;
    int rc = 0;
    rc = openTables(relations, tables);
    if (rc != 0) {
        return rc;
    }
    rc = whereBindCheck(whereClause, tables);
    if (rc != 0) {
        return rc;
    }
    // optimize the iteration
    std::vector<int> iterationRank;
    std::list<Expr *> indexExprs;
    std::set<int> before;
    optimizeIteration(tables, whereClause, iterationRank, indexExprs, before);
    std::vector<std::unique_ptr<Table>> newTables;
    newTables.reserve(iterationRank.size());
    for (int i : iterationRank) {
        newTables.push_back(std::move(tables[i]));
    }
    whereClause->postorder([&newTables](Expr *expr) -> void {
        if (expr->nodeType == NodeType::ATTR_NODE) {
            for (int i = 0; i < newTables.size(); ++i) {
                if (expr->attrInfo.tableName == newTables[i]->tableName) {
                    expr->tableIndex = i;
                    break;
                }
            }
        }
    });
    // bind the attributes
    std::vector<Expr> attributeExprs;
    bool isStatistic = false;
    int total_count = 0;
    AttributeNode groupAttribute{groupAttrName.c_str()};
    Expr groupAttrExpr{&groupAttribute};
    std::set<string> groupSet;
    try {
        if (attributes.size()) {
            if (!groupAttrName.empty()) {
                bindAttribute(&groupAttrExpr, newTables);
                groupAttrExpr.type_check();
            }
            for (auto &attribute: attributes) {
                attributeExprs.emplace_back(attribute);
                bindAttribute(&attributeExprs.back(), newTables);
                attributeExprs.back().type_check();
            }
        } else {
            for (int j = 0; j < newTables.size(); ++j) {
                const auto &table = newTables[j];
                for (auto attr: table->attrInfos) {
                    attributeExprs.emplace_back(attr);
                    attributeExprs.back().tableIndex = j;
                }
            }
        }

    } catch (AttrBindException &exception) {
        printException(exception);
        return QL_TABLE_FAIL;
    } catch (const string &error) {
        cerr << error;
        return QL_TYPE_CHECK;
    }
    // begin iterate
    iterateTables(newTables,
                  0, whereClause,
                  [isStatistic, &attributeExprs, &groupAttrName, &total_count, &groupSet, &groupAttrExpr](
                          const std::vector<Record> &caches
                  ) -> void {
                      total_count += 1;
                      const char *group_data;
                      string groupAttrStr;
                      if (not groupAttrName.empty()) {
                          int groupAttrTableIndex = groupAttrExpr.tableIndex;
                          group_data = caches[groupAttrTableIndex].data;
                          groupAttrExpr.calculate(group_data);
                          groupAttrStr = groupAttrExpr.to_string();
                          groupSet.insert(groupAttrStr);
                          groupAttrExpr.init_calculate();
                      }
                      for (int i = 0; i < attributeExprs.size(); ++i) {
                          const char *data;
                          Expr &attributeExpr = attributeExprs[i];
                          int tableIndex = attributeExpr.tableIndex;
                          data = caches[tableIndex].data;
                          attributeExpr.calculate(data);

                              if (i == attributeExprs.size() - 1) {
                                  cout << attributeExpr.to_string() << "\n";
                              } else {
                                  cout << attributeExpr.to_string() << "  |  ";
                              }

                          attributeExpr.init_calculate();
                      }
                  }, indexExprs);

    cout << "total count " << total_count << "\n";
    return 0;
}

int QL_Manager::exeInsert(string relation, vector<vector<Expr *>>& insertValueTree) {
    std::vector<std::unique_ptr<Table>> tables;
    int rc;
    rc = openTables(std::vector<string>{std::move(relation)}, tables);
    if (rc != 0) {
        return rc;
    }
    try {
        for (const auto &values: insertValueTree) {
            tables[0]->insertData(values);
        }
    }
    catch (const string &strerror) {
        cerr << strerror;
    }
    return 0;
}

int QL_Manager::exeUpdate(string relationName, vector<std::pair<AttributeNode *, Expr *>>& setClauses, Expr *whereClause) {
    std::vector<std::unique_ptr<Table>> tables;
    int rc;
    rc = openTables(std::vector<string>{std::move(relationName)}, tables);
    if (rc != 0) {
        return rc;
    }
    std::vector<int> attributeIndexs;
    rc = whereBindCheck(whereClause, tables);
    if (rc != 0) {
        return rc;
    }
    try {
        for (const auto &it: setClauses) {
            bindAttribute(it.second, tables);
            it.second->type_check();
            if (it.second->dataType == AttrType::NO_ATTR) {
                return QL_TYPE_CHECK;
            }
            int index = tables[0]->getColumnIndex(it.first->attribute);
            if (index < 0) {
                throw AttrBindException{"", it.first->attribute, EXPR_NO_SUCH_ATTRIBUTE};
            }
            const BindAttribute &info = tables[0]->attrInfos[index];
            if (info.attrType != it.second->dataType) {
                if (info.attrType == AttrType::FLOAT and it.second->dataType == AttrType::INT) {
                    it.second->convert_to_float();
                } else {
                    cerr << "Unsupported attribute assignment for column " << info.attrName << "\n";
                }
            }
            attributeIndexs.push_back(index);
        }
    } catch (AttrBindException &exception) {
        printException(exception);
        return QL_TABLE_FAIL;
    }
    std::vector<RID> toBeUpdated;
    iterateTables(*tables[0], whereClause,
                  [&toBeUpdated](const Record &record) -> void {
                      toBeUpdated.push_back(record.rid);
                  });
    for (const auto &rid: toBeUpdated) {
        Record record;
        tables[0]->fileHandle.getRecord(rid, record);
        for (auto &it: setClauses) {
            it.second->init_calculate();
            it.second->calculate(record.data);
        }
        tables[0]->updateData(record, attributeIndexs, setClauses);
    }
    return 0;
}

int QL_Manager::exeDelete(string relationName, Expr *whereClause) {
    std::vector<std::unique_ptr<Table>> tables;
    int rc;
    rc = openTables(std::vector<string>{std::move(relationName)}, tables);
    if (rc != 0) {
        return rc;
    }
    rc = whereBindCheck(whereClause, tables);
    if (rc != 0) {
        return rc;
    }
    std::vector<RID> toBeDeleted;
    iterateTables(*tables[0], whereClause,
                  [&toBeDeleted](const Record &record) -> void {
                      toBeDeleted.push_back(record.rid);
                  });
    for (auto &it: toBeDeleted) {
        rc = tables[0]->deleteData(it);
        if (rc != 0) {
            cerr << "Delete error\n";
            return -1;
        }
    }
    return 0;
}
