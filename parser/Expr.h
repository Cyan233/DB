#ifndef MYDB_EXPR_H
#define MYDB_EXPR_H

#include "../Constants.h"
#include <string>
#include <vector>
#include <functional>
#include <regex>

#define EXPR_NO_SUCH_TABLE -1
#define EXPR_NO_SUCH_ATTRIBUTE -2
#define EXPR_AMBIGUOUS -3

struct AttrBindException {
    std::string table;
    std::string attribute;
    int type;
};

enum class NodeType {
    ARITH_NODE,
    COMP_NODE,
    LOGIC_NODE,
    CONST_NODE,
    ATTR_NODE
};

struct BindAttribute {
    std::string attrName;
    std::string tableName;
    int attrSize;
    int attrLength;
    AttrType attrType = AttrType::NO_ATTR;
    bool notNull{};
    bool withIndex;
    int attrOffset{};
};

class AttributeNode;

class ColumnDecsList;

class TableConstraintList;

class Expr {
public:

    Expr();

    explicit Expr(int i);

    explicit Expr(float f);

    explicit Expr(const char *s, bool is_date = false);

    explicit Expr(bool b);

    Expr(Expr *left, ArithOp op, Expr *right = nullptr);

    Expr(Expr *left, CompOp op, Expr *right);

    Expr(Expr *left, LogicOp op, Expr *right);

    explicit Expr(const AttributeNode *);

    explicit Expr(const BindAttribute &attribute);

    const void *getValue();

    void postorder(const std::function<void(Expr *)>& callback, const std::function<bool(Expr *)>& stop_condition = nullptr);

    void calculate(const char *data, const std::string &relationName = "");

    void type_check();

    void convert_to_float();

    void init_calculate(const std::string &tableName = "");

    std::string to_string() const;

    bool is_true();

    bool operator<(const Expr &expr) const;

    Expr &operator+=(const Expr &expr);

    void assign(const Expr &expr);

    ~Expr();

    union {
        int i;
        float f;
        bool b;
    } value;

    std::string value_s = "";

    const AttributeNode *attribute = nullptr;
    BindAttribute attrInfo;
    bool is_null = true;
    bool calculated = false;
    int tableIndex = -1;
    int columnIndex = -1;

    Expr *left = nullptr, *right = nullptr;
    union {
        ArithOp arith = ArithOp::NO_OP;
        CompOp comp;
        LogicOp logic;
        AttrType constType;
    } oper;

    NodeType nodeType = NodeType::CONST_NODE;
    AttrType dataType;
};

class AttributeNode  {
public:
    AttributeNode() = default;
    AttributeNode(const char *relationName, const char *attributeName) {}

    explicit AttributeNode(const char *attributeName) {}


    bool operator==(const AttributeNode &attribute) const;

    struct AttributeDescriptor {
        std::string relName;
        std::string attrName;

        explicit AttributeDescriptor(std::string relName = "",
                                     std::string attrName = "") :
                relName(std::move(relName)), attrName(std::move(attrName)) {}
    };

    AttributeDescriptor getDescriptor() const;
    std::string table;
    std::string attribute;
};




std::string AttrTypeStr[] = {
        "INT", "FLOAT", "STRING", "DATE", "VARCHAR", "BOOL", "NO_ATTR"
};

std::string CompOpStr[] = {
        "=", "<", ">", "<=", ">=", "!=", "IS", "IS NOT", "LIKE", "NO_OP"
};
std::string ArithOpStr[] = {
        "+", "-", "*", "/", "-", "NO"
};
std::string LogicOpStr[] = {
        "&&", "||", "!", "NO"
};

template<typename T>
bool comp_function(const T &a, const T &b, CompOp compOp) {
    switch (compOp) {
        case CompOp::EQ_OP:
            return a == b;
        case CompOp::GE_OP:
            return not(a < b);
        case CompOp::GT_OP:
            return not(a < b) and not(a == b);
        case CompOp::LE_OP:
            return a < b or a == b;
        case CompOp::LT_OP:
            return a < b;
        case CompOp::NE_OP:
            return not(a == b);
        case CompOp::NO_OP:
            return true;
        case CompOp::IS_OP:
            // todo implement this
            break;
        default:
            return false;
    }
    return false;
}

bool comp_function(const std::string &a, const std::string &b, CompOp compOp);

template<typename T>
T arith_function(T a, ArithOp arithOp, T b = 0) {
    switch (arithOp) {
        case ArithOp::ADD_OP:
            return a + b;
        case ArithOp::SUB_OP:
            return a - b;
        case ArithOp::MUL_OP:
            return a * b;
        case ArithOp::DIV_OP:
            return a / b;
        case ArithOp::MINUS_OP:
            return -a;
        case ArithOp::NO_OP:
            break;
    }
    return T{};
}


bool comp_function(const std::string &a, const std::string &b, CompOp compOp) {
    if (compOp == CompOp::LIKE_OP) {
        std::string regexStr;
        char status = 'A';
        for (char i : b) {
            if (status == 'A') {
                // common status
                if (i == '\\') {
                    status = 'B';
                } else if (i == '[') {
                    regexStr += "[";
                    status = 'C';
                } else if (i == '%') {
                    regexStr += ".*";
                } else if (i == '_') {
                    regexStr += ".";
                } else {
                    regexStr += i;
                }
            } else if (status == 'B') {
                // after '\'
                if (i == '%' || i == '_' || i == '!') {
                    regexStr += i;
                } else {
                    regexStr += "\\";
                    regexStr += i;
                }
                status = 'A';
            } else {
                // after '[' inside []
                if (i == '!') {
                    regexStr += "^";
                } else {
                    regexStr += i;
                }
                status = 'A';
            }
        }
        std::regex reg(regexStr);
        return std::regex_match(a, reg);
    } else {
        switch (compOp) {
            case CompOp::EQ_OP:
                return a == b;
            case CompOp::GE_OP:
                return a >= b;
            case CompOp::GT_OP:
                return a > b;
            case CompOp::LE_OP:
                return a <= b;
            case CompOp::LT_OP:
                return a < b;
            case CompOp::NE_OP:
                return a != b;
            case CompOp::NO_OP:
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool logic_function(bool a, bool b, LogicOp logicOp) {
    switch (logicOp) {
        case LogicOp::AND_OP:
            return a and b;
        case LogicOp::OR_OP:
            return a or b;
        case LogicOp::NOT_OP:
            return not a;
        case LogicOp::NO_OP:
            break;
    }
    return false;
}

bool isComparison(CompOp op) {
    switch (op) {
        case CompOp::EQ_OP:
        case CompOp::GE_OP:
        case CompOp::GT_OP:
        case CompOp::LE_OP:
        case CompOp::LT_OP:
        case CompOp::NE_OP:
            return true;
        case CompOp::NO_OP:
        case CompOp::LIKE_OP:
        case CompOp::IS_OP:
        case CompOp::ISNOT_OP:
            return false;
    }
    return false;
}

#endif //MYDB_EXPR_H
