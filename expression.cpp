#include "expression.h"
#include "program.h"
#include <QDebug>
#include <QQueue>
#include "config.h"

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
    //Step1. Tokenize the expression.
    Tokenizer tokenizer(s,program);
    tokenizer.tokenize(tokens);
    if(debugMode){
        qDebug() << "s: " << s;
        for(Token t : tokens){
            qDebug() <<"token: " << t.s << " " << t.type << " " << t.opt << " " << t.num;
        }
    }
    pos=0;
    //Step2. Parse the expression to a tree.
    root = parseExp();
    //Be careful.There is no need to calculate the tree here.
}

int Expression::evaluate() {
    //TODO: Construct a tree and implement expression evaluation
    if(calculated) return value;

    //Step3. Evaluate the tree
    value = calculateTree(root);
    calculated = true;
    return value;
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
    if(pos==tokens.size()) throw std::invalid_argument("Invalid expression");
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
 * Calculate the tree
*/
int Expression::calculateTree(ExpressionNode* node){
    //qDebug() << "Evaluate: " << node->s ;
    if(node->type==ExpNodeType::number) return node->value;
    else if(node->type==ExpNodeType::variable){
        if(program->variables.find(node->s)==program->variables.end()) 
            throw std::invalid_argument("Variable not found");
        else return program->variables[node->s];
    }
    else if(node->type==ExpNodeType::operation){
        int left = calculateTree(node->children[0]);
        int right = calculateTree(node->children[1]);
        if(node->opt==ExpOperation::add) node->value = left+right;
        else if(node->opt==ExpOperation::sub) node->value = left-right;
        else if(node->opt==ExpOperation::mul) node->value = left*right;
        else if(node->opt==ExpOperation::divide){   
            if(right==0) throw std::invalid_argument("Division by zero");
            node->value = left/right;
        }
        else if(node->opt==ExpOperation::mod){
            if(right==0) throw std::invalid_argument("Division by zero");
            node->value = left%right;
        }
        else if(node->opt==ExpOperation::power) node->value = pow(left,right);
        return node->value;
    }
    else throw std::invalid_argument("Invalid expression");
}

/*
 * Get the expression tree in string format.
 */
QString Expression::getExpressionTree(){
    if (!root) return "";
    QString result;
    QQueue<ExpressionNode*> q;
    QQueue<int> offset_q;
    q.enqueue(root);
    offset_q.enqueue(0);
    
    while(!q.isEmpty()){
        ExpressionNode* node = q.dequeue();
        int offset = offset_q.dequeue();
        for(ExpressionNode* child : node->children){
            q.enqueue(child);
            offset_q.enqueue(offset+1);
        }
        QString str = "";
        for(int i=0;i<offset+1;i++) str += "    ";
        str += node->s;
        result += str + "\n";
    }
    
    return result;
}