#include "program.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statement.h"
#include <QFile>
#include <QTextStream>


/*Program::Program
* Initialize the program.parent is the pointer to mainwindow.
*/
Program::Program(MainWindow *parent)
{
    this->parent = parent;
    pc = 0;
}

/* Program::updateStatement
* Input:
*   line: the line number of the statement to update
*   s: the new statement context(not including the line number)
* Notice:
*   If the line number is already used, the old statement will be covered.
*   If the line number is not used, a new statement will be created.
*   If s is empty, the statement will be deleted and the line number will be freed.
*/
bool Program::updateStatement(int line, const QString& s)
{
    if(line <= 0) return false;
    if(s.isEmpty()) {
        delete statements[line];
        statements.erase(line);
    }
    else{
        if(statements.find(line) != statements.end()) {
            statements[line]->setStatement(s);
        }
        else{
            statements[line] = new Statement(this);
            statements[line]->setStatement(s);
        }
    }
    update();
    return true;
}
/* Program::execute
* Execute the program.
*/
bool Program::execute()
{
    //TODO
    return true;
}

/* Program::clear
* Clear the program and the variables.
*/
void Program::clear()
{
    for(auto it = statements.begin(); it != statements.end(); ++it) {
        delete it->second;
    }
    statements.clear();
    variables.clear();
    pc = 0;
    update();
}

Program::~Program()
{
    clear();
}

/* Program::update
* Update the program to the UI.
*/
void Program::update()
{
    if(parent != nullptr) {
        parent->ui->CodeDisplay->clear();
        for(auto it = statements.begin(); it != statements.end(); ++it) {
            parent->ui->CodeDisplay->append(QString::number(it->first) + " " + it->second->getStatement());
        }
    }
}
