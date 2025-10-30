#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
enum class CoreType { HighPerf, EnergyEfficient };

struct Shape {
    std::vector<int> dims;
    long long size() const {
        if (dims.empty()) return 0;
        long long s = 1;
        for (int d : dims) s *= d;
        return s;
    }
    std::string toString() const {
        std::string out;
        for (size_t i=0;i<dims.size();++i){
            out += std::to_string(dims[i]);
            if (i+1<dims.size()) out += "x";
        }
        return out;
    }
};

enum class IROp {
    TensorDecl, MatMul, Conv2D, Relu, Softmax, Add, Print, Unknown
};

struct IRNode {
    int id = 0;
    IROp op = IROp::Unknown;
    std::string name;                  // output name (decl or assigned)
    std::vector<std::string> inputs;   // input names
    Shape shape;                       // inferred shape if known
    long long flops = 0;               // cost estimate
    CoreType assignedCore = CoreType::EnergyEfficient;
};

using IRNodePtr = std::shared_ptr<IRNode>;

struct IRGraph {
    std::vector<IRNodePtr> nodes;
    std::unordered_map<std::string, IRNodePtr> byName; // map output name -> node

    IRNodePtr addNode(const IRNodePtr &n){
        nodes.push_back(n);
        if (!n->name.empty()) byName[n->name] = n;
        return n;
    }

    IRNodePtr findByName(const std::string &nm) const {
        auto it = byName.find(nm);
        if (it==byName.end()) return nullptr;
        return it->second;
    }
};
