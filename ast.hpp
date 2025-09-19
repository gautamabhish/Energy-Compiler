#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

// Base class for all AST nodes
struct ASTNode {
    virtual ~ASTNode() {}
    virtual void print(int indent = 0) const = 0;
};

using ASTNodePtr = shared_ptr<ASTNode>;

// Utility: indentation
inline void printIndent(int indent) {
    for (int i = 0; i < indent; i++) cout << "  ";
}

// ===== Statement Nodes =====
struct TensorDecl : public ASTNode {
    string name;
    int dim1, dim2;

    TensorDecl(const string &n, int d1, int d2)
        : name(n), dim1(d1), dim2(d2) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "TensorDecl " << name << " [" << dim1 << "," << dim2 << "]\n";
    }
};

struct Assign : public ASTNode {
    string name;
    ASTNodePtr expr;

    Assign(const string &n, ASTNodePtr e)
        : name(n), expr(move(e)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Assign " << name << " =\n";
        expr->print(indent + 1);
    }
};

struct PrintStmt : public ASTNode {
    string var;

    PrintStmt(const string &v) : var(v) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Print " << var << "\n";
    }
};

// ===== Expression Nodes =====
struct VarExpr : public ASTNode {
    string name;
    VarExpr(const string &n) : name(n) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Var " << name << "\n";
    }
};

struct MatmulExpr : public ASTNode {
    string lhs, rhs;
    MatmulExpr(const string &l, const string &r) : lhs(l), rhs(r) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Matmul(" << lhs << ", " << rhs << ")\n";
    }
};

struct ReluExpr : public ASTNode {
    string input;
    ReluExpr(const string &i) : input(i) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Relu(" << input << ")\n";
    }
};

struct SoftmaxExpr : public ASTNode {
    string input;
    SoftmaxExpr(const string &i) : input(i) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Softmax(" << input << ")\n";
    }
};

// ===== Program Node =====
struct Program : public ASTNode {
    vector<ASTNodePtr> statements;

    void addStmt(ASTNodePtr stmt) { statements.push_back(move(stmt)); }

    void print(int indent = 0) const override {
        printIndent(indent);
        cout << "Program\n";
        for (auto &stmt : statements) {
            stmt->print(indent + 1);
        }
    }
};

#endif // AST_HPP
