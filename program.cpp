#include "program.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statement.h"
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QMessageBox>

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
            QString trimmed_s;
            for(int i = 0; i < s.size(); i++){
                if(s[i] != ' '||(i>1&&s[i-1] != ' '))
                    trimmed_s += s[i];
            }

            if(statements.find(line) != statements.end()) {
                statements[line]->setStatement(trimmed_s);
            }
            else{
                statements[line] = new Statement(this);
                statements[line]->setStatement(trimmed_s);
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
        QMessageBox::critical(parent, "Error", QString(e.what()));
    }
}
/* Program::execute
* Execute the program.
*/
bool Program::execute()
{
    //TODO
    init();
    if(!parseAllStatements()) return false;
    else updateTreeDisplay();
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
                if(statements.find(retpc) == statements.end()){
                    QMessageBox::critical(parent, "Error", QString("Invalid GOTO line number %1 on Line %2").arg(retpc).arg(pc));
                    return false;
                }
                pc = retpc;//retpc != 0: control flow change
            }
        }
        return true;
    }
    catch(std::exception& e){
        QMessageBox::critical(parent, "Error", QString("Line %1: %2").arg(pc).arg(e.what()));
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
    updateTreeDisplay();
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
        for(auto it = statements.begin(); it != statements.end(); ++it) {
            parent->ui->CodeDisplay->append(QString::number(it->first) + " " + it->second->getStatement());
        }
        //updateTreeDisplay();
    }
}

/* Program::updateTreeDisplay
* Update the syntax tree display to the UI.
*/
void Program::updateTreeDisplay()
{
    if(parent != nullptr && !background) {
        parent->ui->treeDisplay->clear();
        for(auto it = statements.begin(); it != statements.end(); ++it) {
            parent->ui->treeDisplay->append(QString::number(it->first) + " " + it->second->getStatementTree());
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
    if (!isValidVariableName(s)) {
        throw std::invalid_argument("Invalid variable name: " + s.toStdString());
    }

    
    parent->waitInput = true;
    parent->ui->cmdLineEdit->setText("?");
    blockTillFalse(parent->waitInput);
    variables[s] = parent->inputValue;
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
    ended = true;
    if (!background) {
        parent->updateOutput(QString());
        parent->ui->treeDisplay->clear();
        parent->waitInput = false;
        //qDebug()<<"waitInput unbanned";
        parent->ui->cmdLineEdit->setText("");
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

/* Program::parseAllStatements
* Parse all statements in order of line number.
* Return false if any statement has syntax error.
*/
bool Program::parseAllStatements()
{
    for(auto it = statements.begin(); it != statements.end(); ++it) {
        try {
            Statement* stmt = it->second;
            if (stmt) {
                stmt->parse();  // 这会抛出异常如果语法错误
            }
        } catch (const std::exception& e) {
            if (!background) {
                QMessageBox::critical(parent, "Syntax Error", 
                    QString("Line %1: %2").arg(it->first).arg(e.what()));
            }
            return false;
        }
    }
    return true;
}

/* Program::isValidVariableName
* Check if the variable name is valid.
* Rules:
* 1. Must start with a letter or underscore
* 2. Can only contain letters, numbers, and underscores
* 3. Cannot be a keyword
*/
bool Program::isValidVariableName(const QString& name) const {
    if (name.isEmpty()) return false;
    
    // 检查是否是关键字
    if (keywords.find(name) != keywords.end()) return false;
    
    // 检查首字符
    QChar first = name[0];
    if (!first.isLetter() && first != '_') return false;
    
    // 检查其他字符
    for (int i = 1; i < name.length(); i++) {
        QChar c = name[i];
        if (!c.isLetterOrNumber() && c != '_') return false;
    }
    
    return true;
}