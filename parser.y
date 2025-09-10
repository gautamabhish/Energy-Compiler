%{
#include <iostream>
#include <string>
#include "ast.hpp"
int yylex(void);
void yyerror(const char *s);
using namespace std;
%}

%union {
    int num;
    char* str;
}

%token MODEL TENSOR PRINT MATMUL CONV2D RELU SOFTMAX
%token LBRACE RBRACE LBRACKET RBRACKET LPAREN RPAREN COMMA ASSIGN PLUS
%token <str> ID
%token <num> NUM

%%

program:
    MODEL LBRACE statements RBRACE { cout << "Parsed program!" << endl; }
    ;

statements:
    /* empty */
    | statements statement
    ;

statement:
    TENSOR ID LBRACKET NUM COMMA NUM RBRACKET   
        { cout << "Tensor decl: " << $2 << " [" << $4 << "," << $6 << "]" << endl; }
    | ID ASSIGN expr                            
        { cout << "Assign: " << $1 << endl; }
    | PRINT LPAREN ID RPAREN                    
        { cout << "Print: " << $3 << endl; }
    ;

expr:
    MATMUL LPAREN ID COMMA ID RPAREN            
        { cout << "Matmul(" << $3 << "," << $5 << ")" << endl; }
    | RELU LPAREN ID RPAREN                     
        { cout << "Relu(" << $3 << ")" << endl; }
    | SOFTMAX LPAREN ID RPAREN                  
        { cout << "Softmax(" << $3 << ")" << endl; }
    | ID                                        
        { cout << "Var: " << $1 << endl; }
    ;

%%

int main() {
    return yyparse();
}

void yyerror(const char *s) {
    cerr << "Syntax error: " << s << endl;
}
