#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <iostream>

using Word = std::vector<std::string>;

using Language = std::set<Word>;

using LangSet = std::set<Language>;

Word word_trunc(const Word &w, int k);

Word word_k_concat(const Word &a, const Word &b, int k);

Language k_concat(const Language &L1, const Language &L2, int k);

struct Rule
{
    int num;
    std::string lhs;
    std::vector<std::string> rhs;
};

struct Grammar
{
    std::set<std::string> VN;
    std::set<std::string> VT;
    std::string S;
    int k;
    std::vector<Rule> rules;

    std::map<std::string, std::vector<int>> by_lhs;

    void add_rule(const std::string &lhs, const std::vector<std::string> &rhs);

    void finalize();

    bool is_NT(const std::string &x) const { return VN.count(x) > 0; }
    bool is_T(const std::string &x) const { return VT.count(x) > 0; }

    void print(std::ostream &os = std::cout) const;
};

Grammar parse_grammar(const std::string &text, int k = 1);
