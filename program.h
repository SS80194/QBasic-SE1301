#ifndef PROGRAM_H
#define PROGRAM_H

#include <QString>
#include <map>
#include "statement.h"

class MainWindow;
class Tokenizer;

class Program
{
private:
    MainWindow *parent;
/* Statements of the program.*/
    int pc;//program counter: the current line number of the program
    std::map<int, Statement*> statements;
/* Pool of variables.*/
    std::map<QString, int> variables;
friend class Statement;
friend class Tokenizer;
friend class Expression;

public:
    Program(MainWindow *parent);
    bool updateStatement(int line, const QString& s);
    bool execute();
    void clear();
    void update();
    void output(const QString& s);
    void input(const QString& s);//Ask a value and store it in the variable s
    ~Program();
};

#endif // PROGRAM_H
