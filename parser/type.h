
struct ColumnNode {
    ColumnNode(const char *columnName, AttrType type, int length = 4, int columnFlag = 0){
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
    std::string columnName;
    AttrType type;
    int size;
    int length;
    int columnFlag;
};

struct TableConstraint{
    ConstraintType type;
    vector<Expr *> const_values ;
    vector<std::string> column_list ;
    std::string column_name;
    std::string foreign_table;
    std::string foreign_column;
};
