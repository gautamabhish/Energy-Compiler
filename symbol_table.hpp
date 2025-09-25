// symbol_table.hpp
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <optional>
#include "ir.hpp"   // use Shape from ir.hpp

struct SymbolTable {
    std::unordered_map<std::string, Shape> table;

    bool declare(const std::string &name, const Shape &s) {
        return table.emplace(name, s).second;
    }

    bool exists(const std::string &name) const {
        return table.find(name) != table.end();
    }

    const Shape* lookup(const std::string &name) const {
        auto it = table.find(name);
        return (it != table.end()) ? &it->second : nullptr;
    }
};

#endif
