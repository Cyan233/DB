//
// Created by cyan on 2021/1/15.
//

#include "Compare.h"

template<typename T>
bool compare(const T &a, const T &b, CompOp compOp) {
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
        case CompOp::NO_OP:  // not null
            return true;
        case CompOp::IS_OP:  // is null
            // todo implement this
            break;
        default:
            return false;
    }
    return false;
}

CompOp switchLeftRight(CompOp compOp) {
    switch (compOp) {
        case CompOp::GE_OP:
            return CompOp::LE_OP;
        case CompOp::GT_OP:
            return CompOp::LT_OP;
        case CompOp::LE_OP:
            return CompOp::GE_OP;
        case CompOp::LT_OP:
            return CompOp::GT_OP;
            break;
        default:
            return compOp;
    }
}


bool satisfy(const vector<Condition> &conditions, char* data, tbinfos tb_info){
    for(auto &condition: conditions){ //对于每一个条件，都要满足
        int offset = 0;
        for(int i=0; i<tb_info.columns; i++) {  //查找该条件对应的列
            if(strcmp(condition.col.attr_name, tb_info.attrname[i])==0){
                bool ok = false;
                switch (tb_info.colattr[i]){
                    case AttrType::DATE :
                    case AttrType::INT :{
                        int vi = *reinterpret_cast<int *>(data+offset);
                        ok = compare(vi, condition.compare_value.i, condition.compOp);
                        break;
                    }
                    case AttrType::FLOAT : {
                        float vf = *reinterpret_cast<float *>(data + offset);
                        ok = compare(vf, condition.compare_value.f, condition.compOp);
                        break;
                    }
                }
                if(ok)
                    break;  //比对下一个条件
                else
                    return false;
            }
            offset += tb_info.attrsize[i];
        }
    }
    return true;
}