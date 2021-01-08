
#include "Table.h"
#include "../rm/RecordManager.h"
#include "../sm/SM_Manager.h"
#include "../parser/Expr.h"
#include <memory>
#include <string>
#include <sstream>
#include <iostream>

#include <stdio.h>

bool IsLeapYear(int year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

bool IsLegalDate(int year, int mon, int day) {
    //大：1 3 5 7 8 10 12
    //小：4 6 9 11
    //平：2

    if (year < 0 || mon <= 0 || mon > 12 || day <= 0 || day > 31)return false;

    if (1 == mon || 3 == mon || 5 == mon || 7 == mon || 8 == mon || 10 == mon || 12 == mon) {
        return true;
    }
    if (mon != 2) {
        return day != 31;
    }
    if (IsLeapYear(year)) {
        return !(30 == day || 31 == day);
    } else {
        return !(29 == day || 30 == day || 31 == day);
    }
}

int parseData(const char *date) {
    int year, month, day;
    char *str;
    year = static_cast<int>(strtol(date, &str, 10));
    if (*str == '-') {
        str++;
        month = static_cast<int>(strtol(str, &str, 10));
        if (*str == '-') {
            str++;
            day = static_cast<int>(strtol(str, &str, 10));
            if (IsLegalDate(year, month, day)) {
                return (year - 1) * 31 * 12 + (month - 1) * 31 + (day - 1);
            }
        }
    }
    return -1;

}

int Table::getColumnIndex(const string &attribute) const {
    for (int i = 0; i < attrInfos.size(); i++) {
        if (attribute == attrInfos[i].attrName) {
            return i;
        }
    }
    return -1;
}

bool Table::getIndexAvailable(int index) {
    return not tryOpenIndex(index);
}

int attributeAssign(void *data, const Expr &value, AttrType type, int length) {   // data=value
    switch (type) {
        case AttrType::DATE:
            if (value.dataType == AttrType::DATE) {
                *reinterpret_cast<int *>(data) = value.value.i;
            } else if (value.dataType == AttrType::VARCHAR or value.dataType == AttrType::STRING) {
                int date = parseData(value.value_s.c_str());
                if (date >= 0) {
                    *reinterpret_cast<int *>(data) = date;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
            break;
        case AttrType::INT:
            if (value.dataType == AttrType::INT) {
                *reinterpret_cast<int *>(data) = value.value.i;
            } else if (value.dataType == AttrType::FLOAT) {
                *reinterpret_cast<int *>(data) = (int)(value.value.f);
            } else {
                return -1;
            }
            break;
        case AttrType::FLOAT:
            if (value.dataType == AttrType::FLOAT) {
                *reinterpret_cast<float *>(data) = value.value.f;
            } else if (value.dataType == AttrType::INT) {
                *reinterpret_cast<float *>(data) = value.value.i;
            } else {
                return -1;
            }
            break;
        case AttrType::STRING:
        case AttrType::VARCHAR:
            if (value.dataType == AttrType::STRING || value.dataType == AttrType::VARCHAR) {
                strncpy(static_cast<char *>(data), value.value_s.c_str(), length);
            } else {
                return -1;
            }
            break;
        case AttrType::BOOL:
        case AttrType::NO_ATTR:
            return -1;
    }
    return 0;
}


Table::Table(const string &tableName) {
    this->tableName = tableName;
    this->recordSize = 0;
    int rc = 0;
//    rc = SM_Manager::GetInstance()->GetTableInfo(tableName.c_str(), columns, tableConstraints);
//    if (rc != 0) {
//        throw "The record of relation table " + tableName + " does not exist\n";
//    }
    if (not fileHandle.is_open) {
        rc = RecordManager::getInstance().openFile(tableName + "_Record", fileHandle);
    }
    if (rc != 0) {
        throw "The file of relation table " + tableName + " does not exist\n";
    }
    int offset = 0;
    for (const auto &it: columns) {
        BindAttribute attrInfo;
        attrInfo.tableName = tableName;
        attrInfo.attrName = it->columnName;
        attrInfo.attrType = it->type;
        if (it->type == AttrType::STRING) {
            attrInfo.attrLength = it->size - 1;
        } else {
            attrInfo.attrLength = it->size;
        }
        if (it->type == AttrType::INT) {
            attrInfo.attrSize = sizeof(int);
        } else {
            attrInfo.attrSize = it->size;
        }
        attrInfo.notNull = static_cast<bool>(it->columnFlag & COLUMN_FLAG_NOTNULL);
        attrInfo.withIndex = true;
        attrInfo.attrOffset = offset + 1;
        offset += attrInfo.attrSize + 1;
        this->recordSize += attrInfo.attrSize + 1;
        indexHandles.push_back(nullptr);
        attrInfos.push_back(move(attrInfo));
    }
    for (const auto &it: tableConstraints) {
        foreignTables.emplace_back();
        switch (it->type) {
            case ConstraintType::CHECK_CONSTRAINT:
                for (int i = 0; i < attrInfos.size(); ++i) {
                    if (attrInfos[i].attrName == it->column_name) {
                        constrAttrI.push_back(i);
                        break;
                    }
                }
                foreignAttrInt.push_back(-1);
                break;
            case ConstraintType::PRIMARY_CONSTRAINT:
                for (int i = 0; i < attrInfos.size(); ++i) {
                    if (attrInfos[i].attrName == it->column_list[0]) {
                        constrAttrI.push_back(i);
                        break;
                    }
                }
                foreignAttrInt.push_back(-1);
                break;
            case ConstraintType::FOREIGN_CONSTRAINT: {
                for (int i = 0; i < attrInfos.size(); ++i) {
                    if (attrInfos[i].attrName == it->column_name) {
                        constrAttrI.push_back(i);
                        break;
                    }
                }
//                ColumnDecsList foreign_columns;
//                TableConstraintList foreign_constraint;
//                rc = SM_Manager::GetInstance()->GetTableInfo(it->foreign_table.c_str(), foreign_columns,
//                                                             foreign_constraint);
//                if (rc != 0) {
//                    throw "The record of relation table " + it->foreign_table + " does not exist\n";
//                }
//                bool found = false;
//                for (int i = 0; i < foreign_columns.size(); ++i) {
//                    if (foreign_columns[i]->columnName == it->foreign_column) {
//                        foreignAttrInt.push_back(i);
//                        found = true;
//                        break;
//                    }
//                }
//                if (not found) {
//                    throw "Can't find column " + it->foreign_column + " in table " + it->foreign_table + "\n";
//                }
//                break;
            }

        }
    }
}

Table::~Table() {
    for (auto &it: indexHandles) {
//        if (it != nullptr) {
//            it->CloseIndex();
//        }
        delete it;
    }
}

int Table::tryOpenIndex(int indexNo) {
    if (indexHandles[indexNo] == nullptr) {
        auto *indexHandle = new IX_indexHandle();
        int rc = IX_Manager::GetInstance().OpenIndex(tableName.c_str(), indexNo, *indexHandle);
//        int rc = IX_Manager::GetInstance().OpenIndex(tableName.c_str(), indexNo, *indexHandle, fileHandle, attrInfos[indexNo].attrOffset);
        if (rc != 0) {
            delete indexHandle;
            return -1;
        }
        indexHandles[indexNo] = indexHandle;
    }
    return 0;
}

int Table::tryOpenForeignIndex(int constNo) {
    if (!foreignTables[constNo]) {
        int rc;
        try {
            foreignTables[constNo] = std::unique_ptr<Table>(new Table(tableConstraints[constNo]->foreign_table));
        }
        catch (const string &error) {
            cout << error;
            return -1;
        }
        rc = foreignTables[constNo]->tryOpenIndex(foreignAttrInt[constNo]);
        if (rc != 0) {
            return -1;
        }
    }
    return 0;
}



string Table::checkData(char *data) {
    int rc, i = 0;
    for (const auto &it: tableConstraints) {
        int indexNo = constrAttrI[i];
        void *value = data + attrInfos[indexNo].attrOffset;
        const auto &info = attrInfos[indexNo];
        switch (it->type) {
            case ConstraintType::PRIMARY_CONSTRAINT: {
                if (data[info.attrOffset - 1] == 0) {
                    return "The primary key " + info.attrName + " can't be null\n";
                }
                rc = tryOpenIndex(indexNo);
//                if (rc != 0) {
//                    if (IX_Manager::indexAvailable())
//                        return "We are doomed! Can't find the index for the primary key\n";
//                    else
//                        continue;
//                }
                IX_FindSpeRec indexScan;
                indexScan.OpenScan(*indexHandles[indexNo], CompOp::EQ_OP, value);
                RID rid;
                rc = indexScan.FindNextRec(*indexHandles[indexNo],rid);
//                indexScan.CloseScan();
                if (rc == 0) {
                    return "The primary key " + info.attrName + " alrealy exists\n";
                }

            }
                break;
            case ConstraintType::FOREIGN_CONSTRAINT: {
                rc = tryOpenForeignIndex(i);
                if (rc != 0) {
                    return "Can't find the index for the foreign key\n";
                }
                IX_FindSpeRec indexScan;
                indexScan.OpenScan(*foreignTables[i]->indexHandles[foreignAttrInt[i]], CompOp::EQ_OP, value);
                RID rid;
                rc = indexScan.FindNextRec(*foreignTables[i]->indexHandles[foreignAttrInt[i]], rid);
//                indexScan.CloseScan();
                if (rc != 0) {
                    return "The foreign key for " + info.attrName + " doesn't exist\n";
                }
            }
                break;
            case ConstraintType::CHECK_CONSTRAINT: {
                if (data[info.attrOffset - 1] == 0) {
                    return "The value for " + info.attrName + " can't be null\n";
                }
//                if (not checkValue(data + info.attrOffset, *it->const_values, info.attrType,
//                                     info.attrSize)) {
//                    return "The value for " + info.attrName + " is invalid\n";
//                }
            }
                break;
        }
        i += 1;
    }
    return string();
}


int Table::insertData(const vector<Expr *>& constValues) {
    int rc;

        if (constValues.size() != attrInfos.size()) {
            cout << string("The number of inserted values doesn't match that of columns\n");
            return -1;
        }
    unique_ptr<char[]> data{new char[recordSize]};
    for (int i = 0; i < attrInfos.size(); ++i) {
        const auto &info = attrInfos[i];
        int valueIndex;
        bool isNull;
            valueIndex = i;
            isNull = constValues[i]->is_null;

        if (isNull) {
            if (info.notNull) {
                cout << "The column " << info.attrName << " can't be null\n";
                return -1;
            } else {
                data.get()[info.attrOffset - 1] = 0;
            }
        } else {
            const Expr &value = *(constValues[valueIndex]);
            data[info.attrOffset - 1] = 1;
            rc = attributeAssign(data.get() + info.attrOffset, value, info.attrType, info.attrSize);
            if (rc != 0) {
                cout << string("The type of inserted value doesn't match that of column " + info.attrName + "\n");
                return -1;
            }
        }
    }
    auto result = checkData(data.get());
    if (!result.empty()) {
        cout << string{result};
        return -1;
    }
    // insert record will never fail
    RID rid;
    rc = fileHandle.insertRecord(data.get(),rid);
    rc = insertIndex(data.get(), rid);
    if (rc != 0) {
        fileHandle.deleteRecord(rid);
        return -1;
    }
    return 0;
}

int Table::deleteData(const RID &rid) {
    int rc;
    Record record;
    rc = fileHandle.getRecord(rid, record);
    if (rc != 0) {
        return -1;
    }
    rc = deleteIndex(record.data, rid);
    if (rc != 0) {
        return -1;
    }
    rc = fileHandle.deleteRecord(rid);
    if (rc != 0) {
        insertIndex(record.data, rid);
        return -1;
    }
    return 0;
}

int Table::insertIndex(char *data, const RID &rid) {
    int rc;
    for (int i = 0; i < attrInfos.size(); ++i) {
        if (not data[attrInfos[i].attrOffset - 1]) {
            continue;
        }
        rc = tryOpenIndex(i);
        if (rc == 0) {
            rc = indexHandles[i]->InsertRecord(rid);
            if (rc != 0) {
                for (int indexNo = 0; indexNo < i; ++indexNo) {
                    rc = tryOpenIndex(indexNo);
                    if (rc == 0) {
                        indexHandles[indexNo]->DeleteRecord(rid);
                    }
                }
                cout << "Index insert failed for column " + attrInfos[i].attrName + "\n";
                return rc;
            }
        }
    }
    return 0;
}

int Table::deleteIndex(char *data, const RID &rid) {
    int rc;
    for (int i = 0; i < attrInfos.size(); ++i) {
        if (not data[attrInfos[i].attrOffset - 1]) {
            continue;
        }
        rc = tryOpenIndex(i);
        if (rc == 0) {
            rc = indexHandles[i]->DeleteRecord(rid);
            if (rc != 0) {
                for (int indexNo = 0; indexNo < i; ++indexNo) {
                    rc = tryOpenIndex(indexNo);
                    if (rc == 0) {
                        indexHandles[indexNo]->InsertRecord(rid);
                    }
                }
                cout << "Index delete failed for column " + attrInfos[i].attrName + "\n";
                return rc;
            }
        }
    }
    return 0;
}

int Table::updateData(const Record &record, const vector<int> &attrIndexes, vector<pair<AttributeNode *, Expr *>> clauses) {
    int rc;
    string result;
    char *data = record.data;
    for (int i = 0; i < attrIndexes.size(); ++i) {
        int indexNo = attrIndexes[i];
        const auto &info = attrInfos[indexNo];
        // the type should have been checked
        if (clauses[i].second->is_null) {
            if (info.notNull) {
                cout << "The column " << info.attrName << " can't be null\n";
                return -1;
            } else {
                data[info.attrOffset - 1] = 0;
            }
        } else {
            data[info.attrOffset - 1] = 1;
            rc = attributeAssign(data + info.attrOffset, *clauses[i].second, info.attrType, info.attrSize);
            if (rc != 0) {
                cout << "The type of updated value doesn't match that of column " << info.attrName << "\n";
                return rc;
            }
        }
    }
    Record old_record;
    fileHandle.getRecord(record.rid, old_record);
    rc = deleteIndex(old_record.data, record.rid);
    if (rc != 0) {
        return -1;
    }
    result = checkData(data);
    if (!result.empty()) {
        insertIndex(old_record.data, record.rid);
        cout << string(result);
        return -1;
    }
    rc = fileHandle.updateRecord(record);
    if (rc != 0) {
        insertIndex(old_record.data, record.rid);
        return -1;
    }
    rc = insertIndex(data, record.rid);
    if (rc != 0) {
        fileHandle.updateRecord(old_record);
        insertIndex(old_record.data, record.rid);
        return -1;
    }
    return 0;
}



