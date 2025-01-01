#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>

class Expression
{
public:
    Expression(const QString& s);
    int evaluate();
    int value;bool calculated;
private:
    QString s;
};

#endif