#include "Expr.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>




inline bool compatible(AttrType type1, AttrType type2) {
    return (type1 == AttrType::INT and type2 == AttrType::FLOAT) or
           (type1 == AttrType::FLOAT and type2 == AttrType::INT);
}

inline bool is_arithmetic(AttrType type) {
    return (type == AttrType::INT or type == AttrType::FLOAT);
}

Expr::Expr(int i) {
    is_null = false;
    calculated = true;
    value.i = i;
    nodeType = NodeType::CONST_NODE;
    oper.constType = AttrType::INT;
    dataType = AttrType::INT;
}

Expr::Expr(float f) {
    is_null = false;
    calculated = true;
    value.f = f;
    nodeType = NodeType::CONST_NODE;
    oper.constType = AttrType::FLOAT;
    dataType = AttrType::FLOAT;
}

//
//Expr::Expr(const char *s, bool is_date) {
//    is_null = false;
//    calculated = true;
//    nodeType = NodeType::CONST_NODE;
//    if (not is_date) {
//        value_s = std::string(s, 1, strlen(s) - 2);;
//        oper.constType = AttrType::STRING;
//        dataType = AttrType::STRING;
//    } else {
//        value.i = parseData(s);
//        oper.constType = AttrType::DATE;
//        dataType = AttrType::DATE;
//        if (value.i < 0) {
//            is_null = true;
//            std::cerr << "Incorrect date " << s << "\n";
//        }
//    }
//}


Expr::Expr(bool b) {
    is_null = false;
    calculated = true;
    value.b = b;
    nodeType = NodeType::CONST_NODE;
    oper.constType = AttrType::BOOL;
    dataType = AttrType::BOOL;
}


Expr::~Expr() {
    delete left;
    delete right;
}

Expr::Expr() {
    is_null = true;
    calculated = true;
    nodeType = NodeType::CONST_NODE;
}

Expr::Expr(Expr *left, ArithOp op, Expr *right) {
    is_null = false;
    this->left = left;
    this->right = right;
    this->oper.arith = op;
    this->nodeType = NodeType::ARITH_NODE;
}

Expr::Expr(Expr *left, CompOp op, Expr *right)
:is_null(false),left(left),right(right),nodeType(NodeType::COMP_NODE) {
    this->oper.comp = op;
}

Expr::Expr(Expr *left, LogicOp op, Expr *right) {
    is_null = false;
    this->left = left;
    this->right = right;
    this->oper.logic = op;
    this->nodeType = NodeType::LOGIC_NODE;
}

//Expr::Expr(const AttributeNode *attributeNode) {
//    is_null = false;
//    this->attribute = attributeNode;
//    this->nodeType = NodeType::ATTR_NODE;
//    this->attrInfo.attrName = attributeNode->attribute;
//    this->attrInfo.tableName = attributeNode->table;
//}


Expr::Expr(const BindAttribute &attribute) {
    is_null = true;
    calculated = false;
    this->nodeType = NodeType::ATTR_NODE;
    this->attrInfo = attribute;
    this->dataType = attribute.attrType;
}

void Expr::calculate(const char *data, const std::string &relationName) {
    if (calculated) {
        return;
    }
    if (left != nullptr) {
        left->calculate(data, relationName);
    }
    if (right != nullptr) {
        right->calculate(data, relationName);
    }
    switch (this->nodeType) {
        case NodeType::ARITH_NODE:
            if ((left != nullptr and !left->calculated) or (right != nullptr and !right->calculated)) {
                return;
            }
            calculated = true;
            if (left->is_null or (right != nullptr and right->is_null)) {
                is_null = true;
            } else {
                switch (this->dataType) {
                    case AttrType::INT:
                        if (this->oper.arith == ArithOp::MINUS_OP) {
                            this->value.i = arith_function(left->value.i, this->oper.arith);
                        } else {
                            this->value.i = arith_function(left->value.i, this->oper.arith, right->value.i);
                        }
                        break;
                    case AttrType::FLOAT:
                        if (this->oper.arith == ArithOp::MINUS_OP) {
                            this->value.f = arith_function(left->value.f, this->oper.arith);
                        } else {
                            this->value.f = arith_function(left->value.f, this->oper.arith, right->value.f);
                        }
                        break;
                    case AttrType::STRING:
                    case AttrType::DATE:
                    case AttrType::VARCHAR:
                    case AttrType::NO_ATTR:
                        break;
                    case AttrType::BOOL:
                        break;
                }
            }
            break;
        case NodeType::COMP_NODE:
            if ((left != nullptr and !left->calculated) or (right != nullptr and !right->calculated)) {
                return;
            }
            calculated = true;
            if (oper.comp == CompOp::IS_OP) {
                value.b = not(left->is_null ^ right->is_null);
            } else if (oper.comp == CompOp::ISNOT_OP) {
                value.b = (left->is_null ^ right->is_null);
            } else {
                if (left->is_null or (right != nullptr and right->is_null)) {
                    is_null = true;
                } else {
                    switch (left->dataType) {
                        case AttrType::INT:
                        case AttrType::DATE:
                            value.b = comp_function(left->value.i, right->value.i, oper.comp);
                            break;
                        case AttrType::FLOAT:
                            value.b = comp_function(left->value.f, right->value.f, oper.comp);
                            break;
                        case AttrType::STRING:
                            value.b = comp_function(left->value_s, right->value_s, oper.comp);
                            break;
                        case AttrType::VARCHAR:
                        case AttrType::NO_ATTR:
                            break;
                        case AttrType::BOOL:
                            break;
                    }
                }
            }

            break;
        case NodeType::LOGIC_NODE:
            if ((left == nullptr or left->calculated) and (right == nullptr or right->calculated)) {
                calculated = true;
                if (right == nullptr) {
                    if (left->is_null) {
                        this->is_null = true;
                    } else {
                        this->value.b = logic_function(left->value.b, left->value.b, oper.logic);
                    }
                } else {
                    if (left->is_null or right->is_null) {
                        this->is_null = true;
                    } else {
                        this->value.b = logic_function(left->value.b, right->value.b, oper.logic);
                    }
                }
            } else {
                switch (oper.logic) {
                    case LogicOp::AND_OP:
                        if ((left->calculated and !left->value.b) or (right->calculated and !right->value.b)) {
                            calculated = true;
                            value.b = false;
                        }
                        break;
                    case LogicOp::OR_OP:
                        if ((left->calculated and left->value.b) or (right->calculated and right->value.b)) {
                            calculated = true;
                            value.b = true;
                        }
                        break;
                    case LogicOp::NOT_OP:
                    case LogicOp::NO_OP:
                        break;
                }
            }
            break;
        case NodeType::CONST_NODE:
            break;
        case NodeType::ATTR_NODE:
            if (relationName.empty() || relationName == attrInfo.tableName) {
                calculated = true;
                if (data[this->attrInfo.attrOffset - 1] == 0) {
                    is_null = true;
                } else {
                    is_null = false;
                    switch (this->attrInfo.attrType) {
                        case AttrType::INT:
                        case AttrType::DATE:
                            this->value.i = *reinterpret_cast<const int *>(data + this->attrInfo.attrOffset);
                            if (dataType == AttrType::FLOAT) {
                                this->value.f = static_cast<float>(this->value.i);
                            }
                            break;
                        case AttrType::FLOAT:
                            this->value.f = *reinterpret_cast<const float *>(data + this->attrInfo.attrOffset);
                            break;
                        case AttrType::STRING:
                        case AttrType::VARCHAR:
                            this->value_s = std::string(data + this->attrInfo.attrOffset);
                            break;
                        case AttrType::NO_ATTR:
                            break;
                        case AttrType::BOOL:
                            break;
                    }
                }
            }
            break;
    }
}

void Expr::convert_to_float() {
    postorder([](Expr *expr) -> void {
        switch (expr->nodeType) {
            case NodeType::ARITH_NODE:
                expr->dataType = AttrType::FLOAT;
                break;
            case NodeType::LOGIC_NODE:
                break;
            case NodeType::ATTR_NODE:
                expr->dataType = AttrType::FLOAT;
                break;
            case NodeType::COMP_NODE:
                break;
            case NodeType::CONST_NODE:
                switch (expr->oper.constType) {
                    case AttrType::INT:
                    case AttrType::DATE:
                        expr->dataType = AttrType::FLOAT;
                        expr->value.f = static_cast<float>(expr->value.i);
                        break;
                    case AttrType::FLOAT:
                        break;
                    case AttrType::STRING:
                        break;
                    case AttrType::VARCHAR:
                        break;
                    case AttrType::BOOL:
                        break;
                    case AttrType::NO_ATTR:
                        break;
                }
                break;
        }
    }, [](Expr *expr) -> bool {
        return expr->dataType == AttrType::FLOAT;
    });
}

void Expr::postorder(const std::function<void(Expr *)>& callback, const std::function<bool(Expr *)>& stop_condition) {
    if (stop_condition != nullptr) {
        if (stop_condition(this)) {
            return;
        }
    }
    if (left != nullptr) {
        left->postorder(callback, stop_condition);
    }
    if (right != nullptr) {
        right->postorder(callback, stop_condition);
    }
    callback(this);
}

bool judge_calculated(Expr *expr) {
    if (expr->left != nullptr) {
        if (expr->right != nullptr) {
            return expr->left->calculated and expr->right->calculated;
        } else {
            return expr->left->calculated;
        }
    }
    return false;
}

void Expr::init_calculate(const std::string &tableName) {   //???
    if (tableName.empty()) {
        postorder([](Expr *expr) -> void {
            switch (expr->nodeType) {
                case NodeType::ARITH_NODE:
                case NodeType::COMP_NODE:
                case NodeType::LOGIC_NODE:
                    expr->calculated = judge_calculated(expr);
                    break;
                case NodeType::CONST_NODE:
                    expr->calculated = true;
                    break;
                case NodeType::ATTR_NODE:
                    expr->calculated = false;
                    break;
            }
        });
    } else {
        postorder([&tableName](Expr *expr) -> void {
            switch (expr->nodeType) {
                case NodeType::ARITH_NODE:
                case NodeType::COMP_NODE:
                case NodeType::LOGIC_NODE:
                    expr->calculated = judge_calculated(expr);
                    break;
                case NodeType::CONST_NODE:
                    expr->calculated = true;
                    break;
                case NodeType::ATTR_NODE:
                    if (expr->attrInfo.tableName == tableName) {
                        expr->calculated = false;
                    }
            }
        });
    }
}

bool Expr::operator<(const Expr &expr) const {
    if (calculated and expr.calculated) {
        switch (dataType) {
            case AttrType::INT:
            case AttrType::DATE:
                return value.i < expr.value.i;
            case AttrType::FLOAT:
                return value.f < expr.value.f;
            case AttrType::STRING:
            case AttrType::VARCHAR:
                return value_s < expr.value_s;
            case AttrType::BOOL:
                return value.b < expr.value.b;
            default:
                return false;
        }
    } else {
        throw "Can't perform comparison because the value is uncertain\n";
    }
}

Expr &Expr::operator+=(const Expr &expr) {
    if (calculated and expr.calculated) {
        switch (dataType) {
            case AttrType::INT:
            case AttrType::DATE:
                value.i += expr.value.i;
                break;
            case AttrType::FLOAT:
                value.f += expr.value.f;
                break;
            case AttrType::STRING:
            case AttrType::VARCHAR:
            case AttrType::BOOL:
            case AttrType::NO_ATTR:
                // unsupported
                break;
        }
        return *this;
    } else {
        throw "Can't perform computation because the value is uncertain\n";
    }
}

void Expr::assign(const Expr &expr) {
    if (calculated and expr.calculated) {
        switch (dataType) {
            case AttrType::INT:
            case AttrType::DATE:
                value.i = expr.value.i;
                break;
            case AttrType::FLOAT:
                value.f = expr.value.f;
                break;
            case AttrType::STRING:
                value_s = expr.value_s;
                break;
            case AttrType::VARCHAR:
            case AttrType::BOOL:
            case AttrType::NO_ATTR:
                // unsupported
                break;
        }
    } else {
        throw "Can't perform computation because the value is uncertain\n";
    }
}

std::string Expr::to_string() const {
    if (is_null) {
        return "";
    } else {
        std::ostringstream sstream;
        switch (dataType) {
            case AttrType::INT:
                if (nodeType == NodeType::ATTR_NODE) {
                    sstream.width(attrInfo.attrLength);
                    sstream.fill('0');
                }
                sstream << value.i;
                break;
            case AttrType::FLOAT:
                sstream << value.f;
                break;
            case AttrType::STRING:
            case AttrType::VARCHAR:
                sstream << std::setiosflags(std::ios::left);
                sstream.width(attrInfo.attrLength / 2);
                sstream.fill(' ');
                sstream << value_s;
                break;
            case AttrType::DATE:
                sstream.width(4);
                sstream.fill('0');
                sstream << value.i / (12 * 31) + 1;
                sstream << '-';
                sstream.width(2);
                sstream.fill('0');
                sstream << (value.i % (12 * 31)) / 31 + 1;
                sstream << '-';
                sstream.width(2);
                sstream.fill('0');
                sstream << value.i % 31 + 1;
                break;
            case AttrType::BOOL:
                break;
            case AttrType::NO_ATTR:
                break;
        }
        return sstream.str();
    }
}

bool Expr::is_true() {
    return value.b and not is_null;
}

const void *Expr::getValue() {
    switch (dataType) {
        case AttrType::INT:
        case AttrType::DATE:
            return &value.i;
        case AttrType::FLOAT:
            return &value.f;
        case AttrType::STRING:
        case AttrType::VARCHAR:
            return value_s.c_str();
        case AttrType::BOOL:
            return &value.b;
        case AttrType::NO_ATTR:
            break;
    }
    return nullptr;
}

