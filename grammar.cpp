#include "grammar.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

Word word_trunc(const Word &w, int k)
{
    if ((int)w.size() <= k)
        return w;
    return Word(w.begin(), w.begin() + k);
}

Word word_k_concat(const Word &a, const Word &b, int k)
{
    Word r = a;
    r.insert(r.end(), b.begin(), b.end());
    return word_trunc(r, k);
}

Language k_concat(const Language &L1, const Language &L2, int k)
{
    if (L1.empty() || L2.empty())
        return Language{};
    Language result;
    for (const auto &a : L1)
        for (const auto &b : L2)
            result.insert(word_k_concat(a, b, k));
    return result;
}

void Grammar::add_rule(const std::string &lhs,
                       const std::vector<std::string> &rhs)
{
    int n = (int)rules.size() + 1;
    rules.push_back({n, lhs, rhs});
    VN.insert(lhs);
}

void Grammar::finalize()
{
    by_lhs.clear();
    for (int i = 0; i < (int)rules.size(); i++)
        by_lhs[rules[i].lhs].push_back(i);

    for (const auto &A : VN)
        if (!by_lhs.count(A))
            by_lhs[A] = {};

    for (const auto &r : rules)
        for (const auto &sym : r.rhs)
            if (!sym.empty() && !VN.count(sym))
                VT.insert(sym);
}

void Grammar::print(std::ostream &os) const
{
    os << "Грамматика G = (VN, VT, P, " << S << "),  k = " << k << "\n";
    os << "VN = { ";
    for (const auto &a : VN)
        os << a << " ";
    os << "}\nVT = { ";
    for (const auto &a : VT)
        os << a << " ";
    os << "}\nПравила P:\n";
    for (const auto &r : rules)
    {
        os << "  " << r.num << ")  " << r.lhs << "  ->  ";
        if (r.rhs.empty())
        {
            os << "eps";
        }
        else
        {
            for (const auto &s : r.rhs)
                os << s << " ";
        }
        os << "\n";
    }
}

Grammar parse_grammar(const std::string &text, int k)
{
    Grammar G;
    G.k = k;
    bool first_rule = true;

    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line))
    {

        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
            line.erase(line.begin());
        if (line.empty() || line[0] == '#')
            continue;

        auto arrow = line.find("->");
        if (arrow == std::string::npos)
            continue;

        std::string lhs = line.substr(0, arrow);
        while (!lhs.empty() && (lhs.back() == ' ' || lhs.back() == '\t'))
            lhs.pop_back();
        if (lhs.empty())
            continue;

        if (first_rule)
        {
            G.S = lhs;
            first_rule = false;
        }
        G.VN.insert(lhs);

        std::string rhs_all = line.substr(arrow + 2);
        std::vector<std::string> alternatives;
        {
            std::string alt;
            for (char c : rhs_all)
            {
                if (c == '|')
                {
                    alternatives.push_back(alt);
                    alt = "";
                }
                else
                    alt += c;
            }
            alternatives.push_back(alt);
        }

        for (auto &alt : alternatives)
        {
            std::vector<std::string> syms;
            std::istringstream ts(alt);
            std::string tok;
            while (ts >> tok)
            {

                if (tok == "eps" || tok == "epsilon" || tok == "\xce\xb5")
                    ;
                else
                    syms.push_back(tok);
            }
            G.rules.push_back({(int)G.rules.size() + 1, lhs, syms});
        }
    }
    G.finalize();
    return G;
}
