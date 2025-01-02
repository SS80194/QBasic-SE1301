#include "expression.h"
#include "program.h"
#include <QDebug>

/*
 * Tokenizer
*/
Tokenizer::Tokenizer(const QString& s,Program* program) : s(s),program(program) {}

void Tokenizer::tokenize(QVector<Token>& tokens){
    int p=0,pz=0;
    for(;p < s.size();){
        pz = skipBlank(p);
        if(pz >= s.size()) break;
        p = pz;
        Token temp;
        bool isLastTokenNumber = (tokens.size()>0&&tokens.back().type==ExpNodeType::number);
        if(isDigit(s[p])||(s[p]=='-'&&isDigit(nextChar(p))&&!isLastTokenNumber)){
            //find a number.
            if(s[p]=='-') pz++;
            while(pz<s.size()&&isDigit(s[pz])) pz++;
            temp.s = s.mid(p,pz-p);
            temp.type = ExpNodeType::number;
            temp.num = temp.s.toInt()*(s[p]=='-'?-1:1);
            p = pz;
        }
        else if (isLetter(s[p])){
            //find a variable or "MOD"
            while(pz<s.size()&&(isLetter(s[pz])||s[pz]=='_'||isDigit(s[pz]))) pz++;
            temp.s = s.mid(p,pz-p);
            if(temp.s=="MOD"){
                temp.type = ExpNodeType::operation;
                temp.opt = ExpOperation::mod;
            }
            else{
                temp.type = ExpNodeType::variable;
                temp.s = s.mid(p,pz-p);
                if(program->variables.find(temp.s) == program->variables.end()){
                    //Can't find the variable:The variable is not defined.
                    QString error = "Variable "+temp.s+" not defined";
                    throw std::invalid_argument(error.toStdString());   
                }
                else temp.num = program->variables[temp.s];
            }
            p = pz;
        }
        else if(isOperator(s[p])){
            //find an operator.
            temp.s = s[p];
            temp.type = ExpNodeType::operation;
            if(s[p]=='*'&&nextChar(p)=='*'){
                temp.opt = ExpOperation::power;
                pz++;
            }
            else if(s[p]=='*') temp.opt = ExpOperation::mul;
            else if (s[p]=='+') temp.opt = ExpOperation::add;
            else if (s[p]=='-') temp.opt = ExpOperation::sub;
            else if (s[p]=='/') temp.opt = ExpOperation::divide;

            p = pz+1;
        }
        tokens.push_back(temp);
    }
}

/*
 * ExpressionNode
 */
ExpressionNode::ExpressionNode(const Token&t)
{
    type = t.type;
    s = t.s;
    opt = t.opt;
    value = t.num;
}

/*
 * Expression
*/
Expression::Expression(const QString& s_res,Program* program) : s(s_res),program(program) 
{
    value = 0;
    calculated = false;
    pos = 0;
    value = evaluate();
    calculated=true;
}

int Expression::evaluate() {
    //TODO: Construct a tree and implement expression evaluation
    if(calculated) return value;
    //Step1. Tokenize the expression.
    Tokenizer tokenizer(s,program);
    tokenizer.tokenize(tokens);
    //for(auto token:tokens) qDebug() << "token:" << token.s << "type:" << token.type << "opt:" << token.opt << "num:" << token.num;
    pos=0;
    //Step2. Parse the expression to a tree.
    ExpressionNode* root = parseExp();

    //Step3. Evaluate the tree
    return evaluateTree(root);
}

/*
 * Parse the expression to a tree,Using recursive descent parsing.
 * Contains: parseExp(), parseTerm(), parsePower(), parseFactor()
*/

ExpressionNode* Expression::parseExp(){
    ExpressionNode* node = parseTerm();
    if(pos==tokens.size()) return node;
    else if(tokens[pos].s=="+"||tokens[pos].s=="-"){
        ExpressionNode* node2 = new ExpressionNode(tokens[pos]);
        consume();
        node2->children.push_back(node);
        node2->children.push_back(parseTerm());
        return node2;
    }
    else return node;
    //throw std::invalid_argument("Invalid expression");
}

ExpressionNode* Expression::parseTerm(){
    ExpressionNode* node = parsePower();
    if(pos==tokens.size()) return node;
    else if(tokens[pos].s=="*"||tokens[pos].s=="/"||tokens[pos].s=="MOD"){
        ExpressionNode* node2 = new ExpressionNode(tokens[pos]);
        consume();
        node2->children.push_back(node);
        node2->children.push_back(parsePower());
        return node2;
    }
    else return node;
}

ExpressionNode* Expression::parsePower(){
    ExpressionNode* node = parseFactor();
    if(pos==tokens.size()) return node;
    else if(tokens[pos].s=="**"){
        ExpressionNode* node2 = new ExpressionNode(tokens[pos]);
        consume();
        node2->children.push_back(node);
        node2->children.push_back(parseFactor());
        return node2;
    }
    else return node;

}

ExpressionNode* Expression::parseFactor(){
    if(tokens[pos].type==ExpNodeType::variable){
        ExpressionNode* node = new ExpressionNode(tokens[pos]);
        consume();
        return node;
    }
    else if(tokens[pos].type==ExpNodeType::number){
        ExpressionNode* node = new ExpressionNode(tokens[pos]);
        consume();
        return node;
    }
    else if(tokens[pos].s=="("){
        consume();
        ExpressionNode* node = parseExp();
        consume();//Consume the ")"
        return node;
    }
    else throw std::invalid_argument("Invalid expression");
}


/*
 * Evaluate the tree
*/
int Expression::evaluateTree(ExpressionNode* node){
    //qDebug() << "Evaluate: " << node->s ;
    if(node->type==ExpNodeType::number) return node->value;
    else if(node->type==ExpNodeType::variable) return node->value;
    else if(node->type==ExpNodeType::operation){
        int left = evaluateTree(node->children[0]);
        int right = evaluateTree(node->children[1]);
        if(node->opt==ExpOperation::add) node->value = left+right;
        else if(node->opt==ExpOperation::sub) node->value = left-right;
        else if(node->opt==ExpOperation::mul) node->value = left*right;
        else if(node->opt==ExpOperation::divide) node->value = left/right;
        else if(node->opt==ExpOperation::mod) node->value = left%right;
        else if(node->opt==ExpOperation::power) node->value = pow(left,right);
        return node->value;
    }
    else throw std::invalid_argument("Invalid expression");
}