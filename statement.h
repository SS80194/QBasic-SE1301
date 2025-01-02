#ifndef STATEMENT_H
#define STATEMENT_H

#include <QString>
#include "expression.h"

class Program;

class Statement
{
private:
    Program* parent;
    QString s;
    QString statementTree;
    int pc;
    QVector<Expression*> expressions;
public:
    Statement(Program* parent);
    ~Statement();
    QString getStatement();
    QString getStatementTree();
    void setStatement(const QString& s);
    void parse();
    int execute();
    bool judgeCondition(const QString& exp1, const QString& exp2, const QString& opt);

};
#endif // STATEMENT_H
