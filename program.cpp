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
Program::Program(MainWindow *parent, bool background)
{
    this->parent = parent;
    this->background = background;
    pc = 0;
}

/* Program::init
* Initialize the program:
* - clear all variables.
* - set pc to the first line of the program.
*/
void Program::init(){
    pc = statements.begin()->first;
    variables.clear();
    ended = false;
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
        try{
            if(statements.find(line) != statements.end()) {
                statements[line]->setStatement(s);
            }
            else{
                statements[line] = new Statement(this);
                statements[line]->setStatement(s);
            }
        }
        catch(std::exception& e){
            //qDebug() << e.what();
            return false;
        }

    }

    update();
    return true;
}

void Program::executeStatement(const QString& s){
    try{
        Statement * st = new Statement(this);
        st->setStatement(s);
        st->parse();
        st->execute();
        delete st;
    }
    catch(std::exception& e){
        //qDebug() << e.what();
    }
}
/* Program::execute
* Execute the program.
*/
bool Program::execute()
{
    //TODO
    init();
    try{
        while(pc != -1){
            //qDebug() << "pc: " << pc<<"DEBUG MODE: "<<debug;
            if(ended) return true;
            if(statements.find(pc) == statements.end())
                return false;
            if(debug&&isBreakpoint(pc)){//in debug mode
                //TODO: handle the breakpoint function.
                //qDebug()<<"breakpoint reached";
                breakpoint_blocked = true;
                parent->updateVariables(showVariables());

                blockTillFalse(breakpoint_blocked);
                if(!debug||ended) return true;
                //If the block is ended by "EXIT" command, end the execution.
            }
            int retpc = statements[pc]->execute();
            if(ended) return true;
            //qDebug()<<statements[pc]->getStatement();
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
        //qDebug() << e.what();
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
    ended = true;
    debug = false;
    parent->waitInput = false;
    breakpoint_blocked = false;
    parent->ui->cmdLineEdit->setText("");
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
    if(parent != nullptr && !background) {
        parent->ui->CodeDisplay->clear();
        parent->ui->treeDisplay->clear();
        for(auto it = statements.begin(); it != statements.end(); ++it) {
            parent->ui->CodeDisplay->append(QString::number(it->first) + " " + it->second->getStatement());
            parent->ui->treeDisplay->append(QString::number(it->first) + " " +it->second->getStatementTree());
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
    if (background) {
        variables[s] = parent->inputValue;
    } else {
        parent->waitInput = true;
        parent->ui->cmdLineEdit->setText("?");
        blockTillFalse(parent->waitInput);
        variables[s] = parent->inputValue;
    }
    //qDebug() << "input variable: " << s << "input value: " << parent->inputValue;
}

/* Program::blockTillFalse
 * Block the program until the variable var is false.
 */
void Program::blockTillFalse(volatile bool &var){
    QEventLoop loop;
    //set a timer to check if the var is false
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        if (var == false) {
            loop.quit();
        }
    });
    timer.start(10);
    loop.exec();
}

/*------Debug mode------*/

/* Program::inDebugMode
* Enter the debug mode.
*/
bool Program::inDebugMode(){
    return debug;
}

/* Program::setDebugMode
* Set the debug mode.
*/
void Program::setDebugMode(bool debug_res){
    this->debug = debug_res;
    ended = false;
    if (!background) {
        parent->updateOutput(QString());
        parent->ui->cmdLineEdit->setText("");
        parent->waitInput = false;
    }
    init();
}

/* Program::setBreakpoint
* Set a breakpoint at the line number line.
*/
void Program::setBreakpoint(int line){
    breakpoints.insert(line);
    if (!background) {
        parent->updateBreakPoint(showBreakpoints());
    }
}

/* Program::removeBreakpoint
* Remove a breakpoint at the line number line.
*/
void Program::removeBreakpoint(int line){
    if(breakpoints.find(line) != breakpoints.end()){
        breakpoints.erase(line);
        if (!background) {
            parent->updateBreakPoint(showBreakpoints());
        }
    }
}       

/* Program::clearBreakpoints
* Clear all breakpoints.
*/
void Program::clearBreakpoints(){
    breakpoints.clear();
    if (!background) {
        parent->updateBreakPoint(showBreakpoints());
    }
}

/* Program::isBreakpoint
* Check if the line number line is a breakpoint.
*/
bool Program::isBreakpoint(int line){
    return breakpoints.find(line) != breakpoints.end();
}

/*Program::showVariables
* Show all variables.
*/
QString Program::showVariables(){
    QString res;
    for(auto it = variables.begin(); it != variables.end(); ++it) {
        res += it->first + " = " + QString::number(it->second) + "\n";
    }
    return res;
}

/* Program::showBreakpoints
 * Show all breakpoints.
 */
QString Program::showBreakpoints(){
    QString res;
    for(auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
        res += QString::number(*it) + "\n";
    }
    return res;
}

/* Program::exitDebug
* Exit the debug mode.
*/
void Program::exitDebug(){
    ended = true;
    debug = false;
    breakpoint_blocked = false;
    parent->waitInput = false;
    clearBreakpoints();
    if (!background) {
        parent->updateOutput(QString());
        parent->ui->cmdLineEdit->setText("");
    }
}

/* Program::resume
* Resume the program.
*/
void Program::resume(){
    debug = true;
    breakpoint_blocked = false;
}