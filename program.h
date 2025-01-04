#ifndef PROGRAM_H
#define PROGRAM_H

#include <QString>
#include <map>
#include <set>
#include "statement.h"

class MainWindow;
class Tokenizer;

class Program
{
private:
    bool background=false;
    MainWindow *parent;
    const std::set<QString> keywords = {
        "LOAD", "RUN", "CLEAR", "QUIT", "LIST", "ADD", "DELETE", "PRINT", "LET", "INPUT",
        "GOTO", "IF", "THEN", "END", "REM", "MOD"
    };
    bool isValidVariableName(const QString& name) const;
/* Statements of the program.*/
    int pc;//program counter: the current line number of the program
    std::map<int, Statement*> statements;
/* Pool of variables.*/
    std::map<QString, int> variables;
/* Useful in debug mode*/
    bool debug=false;
    volatile bool breakpoint_blocked=false;
    volatile bool ended=false;
    std::set<int> breakpoints;
friend class Statement;
friend class Tokenizer;
friend class Expression;

public:
    void blockTillFalse(volatile bool &var);

public:
    Program(MainWindow *parent, bool background = false);
    bool updateStatement(int line, const QString& s);
    bool execute();
    void init();
    void clear();
    void update();
    void updateTreeDisplay();
    void output(const QString& s);
    void input(const QString& s);//Ask a value and store it in the variable s
    ~Program();
/* Debug mode*/
    bool inDebugMode();
    void setDebugMode(bool debug);
    void setBreakpoint(int line);
    void removeBreakpoint(int line);
    void clearBreakpoints();
    bool isBreakpoint(int line);
    QString showVariables();
    QString showBreakpoints();
    void exitDebug();
    void resume();
    void executeStatement(const QString& s);
    bool parseAllStatements();
};

#endif // PROGRAM_H
