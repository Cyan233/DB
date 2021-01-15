//
// Created by cyan on 2021/1/15.
//

#ifndef DB_COMPARE_H
#define DB_COMPARE_H

#include "struct.h"
#include "vector"

template<typename T>
bool compare(const T &a, const T &b, CompOp compOp);
CompOp switchLeftRight(CompOp compOp);

#endif //DB_COMPARE_H
