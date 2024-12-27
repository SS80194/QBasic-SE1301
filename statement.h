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
    void execute();
};
#endif // STATEMENT_H
