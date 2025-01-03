#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QString>
#include <QVector>

class Program;

enum ExpNodeType{
    variable,
    number,
    operation,
    bracket,
};

enum ExpOperation{
    add,
    sub,
    mul,
    divide,
    mod,
    power,
};

struct Token{
    QString s;
    ExpNodeType type;
    ExpOperation opt;
    long long num;
};


class Tokenizer
{
public:
    Tokenizer(const QString& s,Program* program);
    void tokenize(QVector<Token>& tokens);

private:
    QString s;
    Program* program;
    
private://helper functions used to tokenize.
    QChar currentChar(int pos);
    QChar nextChar(int pos);
    bool isOperator(QChar c);
    int skipBlank(int pos);
    bool isDigit(QChar c);
    bool isLetter(QChar c);
};

#endif