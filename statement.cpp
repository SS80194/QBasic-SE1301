#include "statement.h"
#include "program.h"
#include "expression.h"
#include <QDebug>
#include <QRegularExpression>

Statement::Statement(Program* parent)
{
    this->parent = parent;
}

void Statement::setStatement(const QString& s)
{
    this->s = s;
    parse();
}

QString Statement::getStatement()
{
    return s;
}

/*
 * Statement::parse
 * Parse the statement into a tree.
*/
void Statement::parse(){
    //clear old data
    for(auto exp : expressions){
        delete exp;
    }
    expressions.clear();
    statementTree = "";
    
    //split the command into two parts,seperated by the first space
    //store in argv0 and argv1.
    QString trimmed = s.trimmed();
    int firstSpaceIndex = trimmed.indexOf(' ');
    QString argv0,argv1;
    argv0 = trimmed.left(firstSpaceIndex);
    if(firstSpaceIndex != -1) argv1 = trimmed.mid(firstSpaceIndex + 1).trimmed();
    else argv1 = "";

    if(QString::compare(argv0,"PRINT") == 0){
        Expression* evaluator = new Expression(argv1,parent);
        expressions.push_back(evaluator);
        statementTree = "PRINT\n" + evaluator->getExpressionTree();
        pc = 0;
    }
    else if(QString::compare(argv0,"INPUT") == 0){
        
        statementTree = "INPUT\n    " + argv1;
        pc = 0;
    }
    else if(QString::compare(argv0,"LET") == 0){    
        int equalIndex = argv1.indexOf('=');
        if (equalIndex == -1) {
            throw std::invalid_argument("Error: Invalid LET statement format: = not found.");
        }

        QString varName = argv1.left(equalIndex).trimmed();
        QString expression = argv1.mid(equalIndex + 1).trimmed();

        Expression* evaluator = new Expression(expression,parent);
        expressions.push_back(evaluator);

        statementTree = "LET =\n    " + varName + "\n" + evaluator->getExpressionTree();
        pc = 0;
    }
    else if(QString::compare(argv0,"GOTO") == 0){
        bool ok;
        int lineNumber = argv1.toInt(&ok);
        if (ok) {
            pc = lineNumber;
            statementTree = "GOTO\n    " + QString::number(lineNumber)+"\n";
        } else {
            throw std::invalid_argument("Error: Invalid GOTO statement format: invalid line number.");
        }
    }
    else if(QString::compare(argv0,"IF") == 0){
        int thenIndex = argv1.indexOf("THEN");
        if (thenIndex == -1) throw std::invalid_argument("Error: Invalid IF statement format: THEN not found.");

        //Split the condition and the line number
        QString condition = argv1.left(thenIndex).trimmed();
        QString lineNumberStr = argv1.mid(thenIndex + 4).trimmed();

        //Split the condition into two parts ,seperated by '=' or '>' or '<',and mark the operator
        int optIndex = condition.indexOf(QRegularExpression("[><=]"));
        if(optIndex == -1) throw std::invalid_argument("Error: Invalid IF statement format: operator not found.");
        QString exp1 = condition.left(optIndex);
        QString exp2 = condition.mid(optIndex + 1).trimmed();
        QString opt = condition.mid(optIndex, 1);

        Expression* evaluator1 = new Expression(exp1,parent);
        Expression* evaluator2 = new Expression(exp2,parent);
        expressions.push_back(evaluator1);
        expressions.push_back(evaluator2);
        statementTree = "IF THEN\n" + evaluator1->getExpressionTree() + "    " + opt + "\n" + evaluator2->getExpressionTree() + "    " + lineNumberStr+"\n";
        pc = 0;
    }
    else if(QString::compare(argv0,"END") == 0){
        //END statement:return -2
        statementTree = "END\n";
        pc = -2;
    }
    else if(QString::compare(argv0,"REM") == 0){
        //REM: do nothing
        statementTree = "REM\n    " + argv1 + "\n";
        pc = 0;
    }
    pc = 0;
}

/* Statement::execute.
* Execute the statement.
* Return -1 if the statement execution failed.(Actually, this should not happen.)
* Return -2 if the statement touches the END statement.
* Return 0 if the next statement index is not set.
* If the next statement is set, return the next statement index.
*/
int Statement::execute()
{
    //split the command into two parts,seperated by the first space
    //store in argv0 and argv1.
    QString trimmed = s.trimmed();
    int firstSpaceIndex = trimmed.indexOf(' ');
    QString argv0,argv1;
    argv0 = trimmed.left(firstSpaceIndex);
    if(firstSpaceIndex != -1) argv1 = trimmed.mid(firstSpaceIndex + 1).trimmed();
    else argv1 = "";

    if(QString::compare(argv0,"PRINT") == 0){
        Expression evaluator(argv1,parent);
        int result = evaluator.evaluate();
        parent->output(QString::number(result));
        //qDebug() << result;
        return 0;
    }
    else if(QString::compare(argv0,"INPUT") == 0){
        //TODO: implement INPUT statement
        parent->input(argv1);
        return 0;
    }
    else if(QString::compare(argv0,"LET") == 0){    
        int equalIndex = argv1.indexOf('=');
        if (equalIndex == -1) {
            throw std::invalid_argument("Error: Invalid LET statement format: = not found.");
        }
        QString varName = argv1.left(equalIndex).trimmed();
        QString expression = argv1.mid(equalIndex + 1).trimmed();

        Expression evaluator(expression,parent);
        int result = evaluator.evaluate();
        parent->variables[varName] = result;
        return 0;
    }
    else if(QString::compare(argv0,"GOTO") == 0){
        bool ok;
        int lineNumber = argv1.toInt(&ok);
        if (ok) {
            return lineNumber;
        } else {
            throw std::invalid_argument("Error: Invalid GOTO statement format: invalid line number.");
        }
    }
    else if(QString::compare(argv0,"IF") == 0){
        int thenIndex = argv1.indexOf("THEN");
        if (thenIndex == -1) throw std::invalid_argument("Error: Invalid IF statement format: THEN not found.");

        //Split the condition and the line number
        QString condition = argv1.left(thenIndex).trimmed();
        QString lineNumberStr = argv1.mid(thenIndex + 4).trimmed();

        //Split the condition into two parts ,seperated by '=' or '>' or '<',and mark the operator
        int optIndex = condition.indexOf(QRegularExpression("[><=]"));
        if(optIndex == -1) throw std::invalid_argument("Error: Invalid IF statement format: operator not found.");
        QString exp1 = condition.left(optIndex);
        QString exp2 = condition.mid(optIndex + 1).trimmed();
        QString opt = condition.mid(optIndex, 1);

        if (judgeCondition(exp1,exp2,opt)) {
            bool ok;
            int lineNumber = lineNumberStr.toInt(&ok);
            if (ok) {
                return lineNumber;
            } else {
                throw std::invalid_argument("Error: Invalid IF statement format: invalid line number.");
            }
        }
        return 0;
    }
    else if(QString::compare(argv0,"END") == 0){
        //END statement:return -2
        return -2;
    }
    else if(QString::compare(argv0,"REM") == 0){
        //REM: do nothing
        return 0;
    }
    return 0;
}

bool Statement::judgeCondition(const QString& exp1, const QString& exp2, const QString& opt)
{
    //implement condition judgment
    Expression evaluator1(exp1,parent);
    Expression evaluator2(exp2,parent);
    int value1 = evaluator1.evaluate();
    int value2 = evaluator2.evaluate();
    if(opt == "=") return value1 == value2;
    else if(opt == ">") return value1 > value2;
    else if(opt == "<") return value1 < value2;
    else throw std::invalid_argument("Invalid operator");
    return false;
}

QString Statement::getStatementTree(){
    return statementTree;
}

Statement::~Statement(){
    for(auto exp : expressions){
        delete exp;
    }
    expressions.clear();
}

