// ir_builder.cpp
#include "ast.hpp"
#include "ir.hpp"
#include "symbol_table.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>

// forward declaration
std::string genIRFromExpr(ASTNode* expr, SymbolTable &symtab, IRGraph &graph, int &nextId, bool &ok);

// helper to create temp name
static std::string makeTemp(int id){ std::ostringstream ss; ss << "_t" << id; return ss.str(); }

IRGraph buildIRFromAST(Program* prog, SymbolTable &symtab, bool &ok_out) {
    IRGraph graph;
    int nextId = 1;
    bool ok = true;

    for (auto &stmtPtr : prog->statements) {
        ASTNode* n = stmtPtr.get();

        if (auto td = dynamic_cast<TensorDecl*>(n)) {
            Shape s;
            // assign dims safely
            s.dims.clear();
            s.dims.push_back(td->dim1);
            s.dims.push_back(td->dim2);

            if (!symtab.declare(td->name, s)) {
                std::cerr << "Semantic Error: tensor '" << td->name << "' redeclared\n";
                ok = false;
            }
            auto inode = std::make_shared<IRNode>();
            inode->id = nextId++;
            inode->op = IROp::TensorDecl;
            inode->name = td->name;
            inode->shape = s;
            inode->flops = 0;
            graph.addNode(inode);
        }
        else if (auto as = dynamic_cast<Assign*>(n)) {
            // build RHS IR
            bool subok = true;
            std::string outTemp = genIRFromExpr(as->expr.get(), symtab, graph, nextId, subok);
            if (!subok) ok = false;
            if (outTemp.empty()) {
                std::cerr << "Semantic Error: failed to compute RHS for assignment to '" << as->name << "'\n";
                ok = false;
                continue;
            }
            // find temp node and rename to target LHS name
            auto tempNode = graph.findByName(outTemp);
            if (!tempNode) {
                // it's possible RHS was just a var (no new node created)
                if (!symtab.exists(outTemp)) {
                    std::cerr << "Semantic Error: unknown symbol '" << outTemp << "' in assignment\n";
                    ok = false;
                } else {
                    // create a copy-node that names LHS
                    auto inode = std::make_shared<IRNode>();
                    inode->id = nextId++;
                    inode->op = IROp::Unknown;
                    inode->name = as->name;
                    inode->inputs = { outTemp };
                    const Shape* sPtr = symtab.lookup(outTemp);
                    if (sPtr) inode->shape = *sPtr;
                    graph.addNode(inode);
                    symtab.declare(as->name, inode->shape);
                }
            } else {
                // rename mapping: remove old key, set new name
                graph.byName.erase(tempNode->name);
                tempNode->name = as->name;
                graph.byName[as->name] = tempNode;
                // declare symbol if needed
                if (!symtab.exists(as->name)) symtab.declare(as->name, tempNode->shape);
            }
        }
        else if (auto ps = dynamic_cast<PrintStmt*>(n)) {
            auto inode = std::make_shared<IRNode>();
            inode->id = nextId++;
            inode->op = IROp::Print;
            inode->name = ""; // no output
            inode->inputs = { ps->var };
            if (!symtab.exists(ps->var)) {
                std::cerr << "Semantic Error: print of undeclared symbol '" << ps->var << "'\n";
                ok = false;
            } else {
                const Shape* sPtr = symtab.lookup(ps->var);
                if (sPtr) inode->shape = *sPtr;
            }
            graph.addNode(inode);
        }
        else {
            std::cerr << "Unknown top-level AST node encountered\n";
            ok = false;
        }
    }

    ok_out = ok;
    return graph;
}

// Generate IR node(s) for expression and return the name of produced value (either existing var or temp)
std::string genIRFromExpr(ASTNode* expr, SymbolTable &symtab, IRGraph &graph, int &nextId, bool &ok) {
    ok = true;
    if (auto ve = dynamic_cast<VarExpr*>(expr)) {
        if (!symtab.exists(ve->name)) {
            std::cerr << "Semantic Error: use of undeclared tensor '" << ve->name << "'\n";
            ok = false;
        }
        return ve->name;
    }
    if (auto mm = dynamic_cast<MatmulExpr*>(expr)) {
        // ensure inputs exist
        const Shape* L = symtab.lookup(mm->lhs);
        const Shape* R = symtab.lookup(mm->rhs);
        if (!L || !R) {
            std::cerr << "Semantic Error: matmul operand unknown (" << mm->lhs << ", " << mm->rhs << ")\n";
            ok = false;
            // still create a node with unknown shape
        }
        Shape out;
        if (L && R) {
            if (L->dims.size() != 2 || R->dims.size() != 2 || L->dims[1] != R->dims[0]) {
                std::cerr << "Shape Error: matmul incompatible shapes: " << mm->lhs << "["<<L->toString()<<"] * "
                          << mm->rhs << "["<<R->toString()<<"]\n";
                ok = false;
            } else {
                // assign dims safely
                out.dims.clear();
                out.dims.push_back(L->dims[0]);
                out.dims.push_back(R->dims[1]);
            }
        }

        auto node = std::make_shared<IRNode>();
        node->id = nextId++;
        node->op = IROp::MatMul;
        node->inputs = { mm->lhs, mm->rhs };
        node->name = makeTemp(node->id);
        node->shape = out;
        if (!L || !R || out.dims.empty()) node->flops = 0;
        else {
            long long m = L->dims[0];
            long long k = L->dims[1];
            long long n = R->dims[1];
            node->flops = 2LL * m * k * n;
        }
        graph.addNode(node);
        return node->name;
    }
    if (auto rel = dynamic_cast<ReluExpr*>(expr)) {
        auto in = rel->input;
        const Shape* sPtr = symtab.lookup(in);
        if (!sPtr) {
            std::cerr << "Semantic Error: relu operand unknown '"<< in << "'\n";
            ok = false;
        }
        Shape out;
        if (sPtr) out = *sPtr;
        auto node = std::make_shared<IRNode>();
        node->id = nextId++;
        node->op = IROp::Relu;
        node->inputs = { in };
        node->name = makeTemp(node->id);
        node->shape = out;
        node->flops = out.size();
        graph.addNode(node);
        return node->name;
    }
    if (auto sm = dynamic_cast<SoftmaxExpr*>(expr)) {
        auto in = sm->input;
        const Shape* sPtr = symtab.lookup(in);
        if (!sPtr) {
            std::cerr << "Semantic Error: softmax operand unknown '"<< in << "'\n";
            ok = false;
        }
        Shape out;
        if (sPtr) out = *sPtr;
        auto node = std::make_shared<IRNode>();
        node->id = nextId++;
        node->op = IROp::Softmax;
        node->inputs = { in };
        node->name = makeTemp(node->id);
        node->shape = out;
        node->flops = (out.size()>0) ? out.size() * 10 : 0; // rough estimate
        graph.addNode(node);
        return node->name;
    }

    // fallback
    std::cerr << "Unsupported expression node type in IR generator\n";
    ok = false;
    return std::string();
}

static const char *opToStr(IROp op) {
    switch(op){
        case IROp::TensorDecl: return "TensorDecl";
        case IROp::MatMul: return "MatMul";
        case IROp::Conv2D: return "Conv2D";
        case IROp::Relu: return "Relu";
        case IROp::Softmax: return "Softmax";
        case IROp::Add: return "Add";
        case IROp::Print: return "Print";
        default: return "Unknown";
    }
}

void dumpIR(const IRGraph &g) {
    std::cout << "=== IR DUMP ===\n";
    for (auto &n : g.nodes) {
        std::cout << "#" << n->id << " : " << opToStr(n->op)
                  << " name='"<<n->name<<"' inputs=[";
        for (size_t i=0;i<n->inputs.size();++i){
            std::cout << n->inputs[i];
            if (i+1<n->inputs.size()) std::cout << ",";
        }
        std::cout << "] shape="<< n->shape.toString()
                  << " flops="<< n->flops << "\n";
    }
}
