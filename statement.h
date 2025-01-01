#ifndef STATEMENT_H
#define STATEMENT_H

#include <QString>

class Program;
class Statement
{
private:
    Program* parent;
    QString s;
public:
    Statement(Program* parent);
    QString getStatement();
    void setStatement(const QString& s);
    int execute();
    bool judgeCondition(const QString& exp1, const QString& exp2, const QString& opt);
};
#endif // STATEMENT_H
