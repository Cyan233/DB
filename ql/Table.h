#ifndef DB_TABLE_H
#define DB_TABLE_H

#include "../rm/RM_FileHandle.h"
#include "../rm/RecordManager.h"
#include "../rm/struct.h"
#include "../ix/IX_IndexHandle.h"
#include "../ix/IX_FindSpeRec.h"
#include "../parser/Expr.h"
#include "../parser/type.h"

#include <vector>
#include <map>
#include <memory>

using std::vector;
using std::string;

class Table {
public:
    explicit Table(const string &tableName);

    ~Table();

    int getColumnIndex(const string &attribute) const;

    bool getIndexAvailable(int index);

    string checkData(char *data);  // 更新或加入前检查是否符合条件

    int deleteData(const RID &rid);

    int insertData(const vector<Expr *>& constValues);

    int updateData(const Record &record, const vector<int> &attrIndexes, vector<pair<AttributeNode *, Expr *>> clauses);

    int insertIndex(char *data, const RID &rid);

    int deleteIndex(char *data, const RID &rid);


    string tableName;
    RM_FileHandle fileHandle;
    vector<BindAttribute> attrInfos;  //属性的列表
    vector<IX_indexHandle *> indexHandles;
private:

    int tryOpenIndex(int indexNo);

    int tryOpenForeignIndex(int constNo);

    int recordSize;

    vector<ColumnNode *> columns;
    vector<TableConstraint *> tableConstraints;
    vector<unique_ptr<Table>> foreignTables;
    vector<int> foreignAttrInt;
    vector<int> constrAttrI;
};


#endif
