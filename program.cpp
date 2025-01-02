#include "program.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statement.h"
#include <QFile>
#include <QTextStream>
#include <QTimer>


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
    if(!pc){
        pc = statements.begin()->first;
        qDebug() << "pc: " << pc;
    }
    try{
        while(pc != -1){
            qDebug() << "pc: " << pc;
            if(statements.find(pc) == statements.end())
                return false;
            int retpc = statements[pc]->execute();
            qDebug()<<statements[pc]->getStatement();
            if(retpc == -1)
                return false;
            else if(retpc == 0){
                //retpc = 0: no coontrol flow change
                auto itr = statements.find(pc);
                ++itr;
                if(itr == statements.end())
                    pc = -1;
                else
                    pc = itr->first;
            }
            else if(retpc == -2){
                //retpc = -2: END statement
                return true;
            }
            else{
                pc = retpc;//retpc != 0: control flow change
                if(statements.find(pc) == statements.end())
                    throw std::invalid_argument("Invalid line number");
            }
        }
        return true;
    }
    catch(std::exception& e){
        qDebug() << e.what();
        return false;
    }
    
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

/* Program::output
* Output the string s to the output window.
*/
void Program::output(const QString& s)
{
    if(parent != nullptr) {
        parent->ui->textBrowser->append(s);
    }
}

/* Program::input
* Ask a value and store it in the variable s
*/
void Program::input(const QString& s)
{
    //TODO: implement input
    //WAIT TO BE IMPLEMENTED
    parent->waitInput = true;
    
    parent->ui->cmdLineEdit->setText("?");

    QEventLoop loop;
    //set a timer to check if the parent->waitInput is false
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        if (parent->waitInput == false) {
            loop.quit();
        }
    });
    timer.start(10);
    loop.exec();

    variables[s] = parent->inputValue;
}