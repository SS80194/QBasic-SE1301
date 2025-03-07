#include "tokenizer.h"
#include <QDebug>

/*
 * Tokenizer
*/
Tokenizer::Tokenizer(const QString& s,Program* program) : s(s),program(program) {}

//helper functions
QChar Tokenizer::currentChar(int pos) 
{
        return pos < s.size() ? s[pos] : '\0';
}

QChar Tokenizer::nextChar(int pos) {
    return pos + 1 < s.size() ? s[pos + 1] : '\0';
}

bool Tokenizer::isOperator(QChar c) {
    return c == '+' || c == '-' || c == '*' || c == '/' ;
}

//skipBlank:return the first non-blank character after pos
int Tokenizer::skipBlank(int pos) {
    while (pos<s.size() && currentChar(pos).isSpace()) {
        pos++;
    }
    return pos;
}

bool Tokenizer::isDigit(QChar c){
    return c >= '0' && c <= '9';
}

bool Tokenizer::isLetter(QChar c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void Tokenizer::tokenize(QVector<Token>& tokens){
    int p=0,pz=0;
    for(;p < s.size();){
        //qDebug() << "p: " << p << "s[p]: " << s[p];
        pz = skipBlank(p);
        if(pz >= s.size()) break;
        p = pz;
        Token temp;
        bool isLastTokenNumber = (tokens.size()>0&&tokens.back().type==ExpNodeType::number);
        if(isDigit(s[p])||(s[p]=='-'&&isDigit(nextChar(p))&&!isLastTokenNumber)){
            //find a number.
            bool isNegative = false;
            if(s[p]=='-'){
                isNegative = true;
                p++;
                pz=skipBlank(p);
            }
            while(pz<s.size()&&isDigit(s[pz])) pz++;
            temp.s = s.mid(p,pz-p);
            if(isNegative) temp.s = "-" + temp.s;
            temp.type = ExpNodeType::number;
            temp.num = temp.s.toInt();
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
            }
            p = pz;
        }
        else if(isOperator(s[p])){
            //find an operator.
            temp.s = s[p];
            temp.type = ExpNodeType::operation;
            if(s[p]=='*'&&nextChar(p)=='*'){
                temp.s = "**";
                temp.opt = ExpOperation::power;
                pz++;
            }
            else if(s[p]=='*') temp.opt = ExpOperation::mul;
            else if (s[p]=='+') temp.opt = ExpOperation::add;
            else if (s[p]=='-') temp.opt = ExpOperation::sub;
            else if (s[p]=='/') temp.opt = ExpOperation::divide;

            p = pz+1;
        }
        else if(s[p]=='('||s[p]==')'){
            temp.s = s[p];
            temp.type = ExpNodeType::bracket;
            p = pz+1;
        }
        else{
            throw std::invalid_argument(std::string("Invalid character: ") + s[p].toLatin1());
        }
        tokens.push_back(temp);
    }
}
