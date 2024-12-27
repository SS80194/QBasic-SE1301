#include "statement.h"
#include "program.h"

Statement::Statement(Program* parent)
{
    this->parent = parent;
}

void Statement::setStatement(const QString& s)
{
    this->s = s;
}

QString Statement::getStatement()
{
    return s;
}
