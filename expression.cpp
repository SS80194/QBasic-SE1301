#include "expression.h"

Expression::Expression(const QString& s) : s(s) 
{
    value = 0;
    calculated = false;
    value = evaluate();
    calculated=true;
}

int Expression::evaluate() {
    //TODO: Construct a tree and implement expression evaluation
    if(calculated) return value;
    return s.toInt();
}