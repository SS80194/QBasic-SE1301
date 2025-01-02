#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>
#include <QVector>

class Program;

enum ExpNodeType{
    variable,
    number,
    operation,
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
    QChar currentChar(int pos) {
        return pos < s.size() ? s[pos] : '\0';
    }

    QChar nextChar(int pos) {
        return pos + 1 < s.size() ? s[pos + 1] : '\0';
    }

    bool isOperator(QChar c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
    }

    //skipBlank:return the first non-blank character after pos
    int skipBlank(int pos) {
        while (pos<s.size() && currentChar(pos).isSpace()) {
            pos++;
        }
        return pos;
    }

    bool isDigit(QChar c){
        return c >= '0' && c <= '9';
    }

    bool isLetter(QChar c){
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
};

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
    int pos;
    void tokenize();

private:
    void consume(){pos++;}
    ExpressionNode* parseExp();
    ExpressionNode* parseTerm();
    ExpressionNode* parsePower();
    ExpressionNode* parseFactor();

    int evaluateTree(ExpressionNode* node);
};

#endif