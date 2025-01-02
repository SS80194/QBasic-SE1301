#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>
#include <QVector>
#include "tokenizer.h"

class Program;
class Token;
class Tokenizer;

class ExpressionNode
{
private:
    QString s;//the string of the node
    QVector<ExpressionNode*> children;
    ExpNodeType type;
    long long value;
    ExpOperation opt;


public:
    ExpressionNode(const QString& s);
    ExpressionNode(const Token& t);
    //~ExpressionNode();
    friend class Expression;
};

class Expression
{
public:
    Expression(const QString& s_res,Program* program);
    int evaluate();
    int value;bool calculated;
private:
    QString s;
    QVector<Token> tokens;
    Program* program;
    ExpressionNode* root;
    int pos;
    void tokenize();

private:
    void consume(){pos++;}
    ExpressionNode* parseExp();
    ExpressionNode* parseTerm();
    ExpressionNode* parsePower();
    ExpressionNode* parseFactor();

    int calculateTree(ExpressionNode* node);
};

#endif