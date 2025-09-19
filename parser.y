%{
/* Make AST types visible to the union and generated header */
#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"
using namespace std;

/* forward declarations for lexer and error handler */
int yylex(void);
void yyerror(const char *s);

Program* root = nullptr;
%}

/* semantic value union: include AST pointer types */
%union {
    int num;
    char* str;
    ASTNode* node;
    Program* program;
}

/* tokens */
%token MODEL TENSOR PRINT MATMUL CONV2D RELU SOFTMAX
%token LBRACE RBRACE LBRACKET RBRACKET LPAREN RPAREN COMMA ASSIGN PLUS
%token <str> ID
%token <num> NUM

/* nonterminal value types */
%type <program> program statements
%type <node> statement expr

%%

program:
    MODEL LBRACE statements RBRACE
        { root = $3; }
    ;

statements:
    /* empty */
        { $$ = new Program(); }
    | statements statement
        {
          Program* prog = $1;
          prog->addStmt(ASTNodePtr($2));
          $$ = prog;
        }
    ;

statement:
    TENSOR ID LBRACKET NUM COMMA NUM RBRACKET
        { $$ = new TensorDecl(string($2), $4, $6); free($2); }
    | ID ASSIGN expr
        { $$ = new Assign(string($1), ASTNodePtr($3)); free($1); }
    | PRINT LPAREN ID RPAREN
        { $$ = new PrintStmt(string($3)); free($3); }
    ;

expr:
    MATMUL LPAREN ID COMMA ID RPAREN
        { $$ = new MatmulExpr(string($3), string($5)); free($3); free($5); }
    | RELU LPAREN ID RPAREN
        { $$ = new ReluExpr(string($3)); free($3); }
    | SOFTMAX LPAREN ID RPAREN
        { $$ = new SoftmaxExpr(string($3)); free($3); }
    | ID
        { $$ = new VarExpr(string($1)); free($1); }
    ;

%%

int main() {
    if (yyparse() == 0 && root) {
        cout << "=== AST ===\n";
        root->print();
    } else {
        cerr << "Parsing failed or empty program.\n";
    }
    return 0;
}

void yyerror(const char *s) {
    cerr << "Syntax error: " << s << endl;
}
