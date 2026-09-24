#include "algorithms.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

Language first_k_seq(const std::vector<std::string> &alpha,
                     const Grammar &G,
                     const FirstMap &F)
{
    int k = G.k;
    Language lang = Language{Word{}};

    for (const auto &sym : alpha)
    {
        Language sym_f;
        if (G.is_T(sym))
        {
            sym_f = Language{Word{sym}};
        }
        else if (G.is_NT(sym))
        {
            auto it = F.find(sym);
            sym_f = (it != F.end()) ? it->second : Language{};
        }
        else
        {

            sym_f = Language{Word{sym}};
        }
        lang = k_concat(lang, sym_f, k);
        if (lang.empty())
            break;
    }
    return lang;
}

FirstMap compute_first_k(const Grammar &G)
{
    int k = G.k;
    FirstMap F;

    for (const auto &a : G.VT)
        F[a] = Language{Word{a}};

    for (const auto &A : G.VN)
        F[A] = Language{};

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &A : G.VN)
        {
            Language new_lang = F[A];

            for (int idx : G.by_lhs.at(A))
            {
                const auto &rhs = G.rules[idx].rhs;

                if (rhs.empty())
                {

                    new_lang.insert(Word{});
                    continue;
                }

                Language lang = Language{Word{}};
                for (const auto &sym : rhs)
                {
                    Language sym_f;
                    if (G.is_T(sym))
                        sym_f = Language{Word{sym}};
                    else
                        sym_f = F[sym];
                    lang = k_concat(lang, sym_f, k);
                    if (lang.empty())
                        break;
                }

                for (const auto &w : lang)
                    new_lang.insert(w);
            }

            if (new_lang != F[A])
            {
                F[A] = new_lang;
                changed = true;
            }
        }
    }
    return F;
}

SigmaPrimeMap compute_sigma_prime(const Grammar &G, const FirstMap &F)
{
    int k = G.k;
    SigmaPrimeMap sp;

    for (const auto &A : G.VN)
        for (const auto &B : G.VN)
            sp[{A, B}] = LangSet{};

    for (const auto &A : G.VN)
    {
        for (int idx : G.by_lhs.at(A))
        {
            const auto &rhs = G.rules[idx].rhs;
            for (int p = 0; p < (int)rhs.size(); p++)
            {
                if (!G.is_NT(rhs[p]))
                    continue;
                const std::string &B = rhs[p];

                std::vector<std::string> beta(rhs.begin() + p + 1, rhs.end());
                Language L = first_k_seq(beta, G, F);
                sp[{A, B}].insert(L);
            }
        }
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &A : G.VN)
        {
            for (const auto &B : G.VN)
            {
                LangSet &cur = sp[{A, B}];
                LangSet new_set = cur;

                for (int idx : G.by_lhs.at(A))
                {
                    const auto &rhs = G.rules[idx].rhs;
                    for (int p = 0; p < (int)rhs.size(); p++)
                    {
                        if (!G.is_NT(rhs[p]))
                            continue;
                        const std::string &Xp = rhs[p];

                        const LangSet &sp_xp_b = sp[{Xp, B}];
                        if (sp_xp_b.empty())
                            continue;

                        std::vector<std::string> suffix(rhs.begin() + p + 1, rhs.end());
                        Language suffix_first = first_k_seq(suffix, G, F);

                        for (const auto &Lprime : sp_xp_b)
                        {

                            Language new_L = k_concat(Lprime, suffix_first, k);
                            new_set.insert(new_L);
                        }
                    }
                }

                if (new_set != cur)
                {
                    cur = new_set;
                    changed = true;
                }
            }
        }
    }
    return sp;
}

SigmaMap compute_sigma(const Grammar &G, const SigmaPrimeMap &sp)
{
    SigmaMap sigma;
    for (const auto &A : G.VN)
        sigma[A] = sp.at({G.S, A});

    sigma[G.S].insert(Language{Word{}});
    return sigma;
}

FollowMap compute_follow_k(const Grammar &G, const SigmaMap &sigma)
{
    FollowMap FL;
    for (const auto &A : G.VN)
    {
        Language result;
        for (const auto &L : sigma.at(A))
            for (const auto &w : L)
                result.insert(w);
        FL[A] = result;
    }
    return FL;
}

LLTestResult test_ll_k(const Grammar &G, const FirstMap &F, const SigmaMap &sigma)
{
    int k = G.k;
    LLTestResult res;
    res.is_ll = true;

    for (const auto &A : G.VN)
    {
        const auto &indices = G.by_lhs.at(A);
        if (indices.size() < 2)
            continue;

        const LangSet &ctx_set = sigma.at(A);

        for (const auto &L : ctx_set)
        {

            for (int i = 0; i < (int)indices.size(); i++)
            {
                for (int j = i + 1; j < (int)indices.size(); j++)
                {
                    const auto &rhs_i = G.rules[indices[i]].rhs;
                    const auto &rhs_j = G.rules[indices[j]].rhs;

                    Language Fi = first_k_seq(rhs_i, G, F);
                    Language Fj = first_k_seq(rhs_j, G, F);

                    Language fi = k_concat(Fi, L, k);
                    Language fj = k_concat(Fj, L, k);

                    Language inter;
                    std::set_intersection(fi.begin(), fi.end(),
                                          fj.begin(), fj.end(),
                                          std::inserter(inter, inter.begin()));

                    if (!inter.empty())
                    {
                        res.is_ll = false;
                        LLViolation v;
                        v.A = A;
                        v.rule1 = G.rules[indices[i]].num;
                        v.rule2 = G.rules[indices[j]].num;
                        v.context_L = L;
                        v.conflict = inter;
                        res.violations.push_back(v);
                    }
                }
            }
        }
    }
    return res;
}

StrongLLResult test_strong_ll_k(const Grammar &G, const FirstMap &F, const FollowMap &FL)
{
    int k = G.k;
    StrongLLResult res;
    res.is_strong = true;

    for (const auto &A : G.VN)
    {
        const auto &indices = G.by_lhs.at(A);
        if (indices.size() < 2)
            continue;

        const Language &follow_A = FL.at(A);

        for (int i = 0; i < (int)indices.size(); i++)
        {
            for (int j = i + 1; j < (int)indices.size(); j++)
            {
                Language Fi = first_k_seq(G.rules[indices[i]].rhs, G, F);
                Language Fj = first_k_seq(G.rules[indices[j]].rhs, G, F);

                Language fi = k_concat(Fi, follow_A, k);
                Language fj = k_concat(Fj, follow_A, k);

                Language inter;
                std::set_intersection(fi.begin(), fi.end(),
                                      fj.begin(), fj.end(),
                                      std::inserter(inter, inter.begin()));

                if (!inter.empty())
                {
                    res.is_strong = false;
                    LLViolation v;
                    v.A = A;
                    v.rule1 = G.rules[indices[i]].num;
                    v.rule2 = G.rules[indices[j]].num;
                    v.context_L = follow_A;
                    v.conflict = inter;
                    res.violations.push_back(v);
                }
            }
        }
    }
    return res;
}

std::string word_str(const Word &w)
{
    if (w.empty())
        return "eps";
    std::string s;
    for (size_t i = 0; i < w.size(); i++)
    {
        if (i > 0)
            s += "";
        s += w[i];
    }
    return s;
}

std::string lang_str(const Language &L)
{
    if (L.empty())
        return "{}";
    std::string s = "{ ";
    bool first = true;
    for (const auto &w : L)
    {
        if (!first)
            s += ", ";
        s += word_str(w);
        first = false;
    }
    s += " }";
    return s;
}

std::string langset_str(const LangSet &LS)
{
    if (LS.empty())
        return "{}";
    std::string s = "{ ";
    bool first = true;
    for (const auto &L : LS)
    {
        if (!first)
            s += ",  ";
        s += lang_str(L);
        first = false;
    }
    s += " }";
    return s;
}

void print_first_k(const Grammar &G, const FirstMap &F, std::ostream &os)
{
    os << "\n┌──────────────────────────────────────┐\n";
    os << "│  FIRST_" << G.k << "(A)                          │\n";
    os << "└──────────────────────────────────────┘\n";
    for (const auto &A : G.VN)
    {
        auto it = F.find(A);
        os << "  FIRST_" << G.k << "(" << std::setw(3) << std::left << A << ") = "
           << (it != F.end() ? lang_str(it->second) : "{}") << "\n";
    }
}

void print_sigma_prime(const Grammar &G, const SigmaPrimeMap &sp, std::ostream &os)
{
    os << "\n┌──────────────────────────────────────┐\n";
    os << "│  sigma'(A, B)                        │\n";
    os << "└──────────────────────────────────────┘\n";
    bool any = false;
    for (const auto &A : G.VN)
    {
        for (const auto &B : G.VN)
        {
            const auto &LS = sp.at({A, B});
            if (!LS.empty())
            {
                os << "  sigma'(" << A << ", " << B << ") = "
                   << langset_str(LS) << "\n";
                any = true;
            }
        }
    }
    if (!any)
        os << "  (все значения пусты)\n";
}

void print_sigma(const Grammar &G, const SigmaMap &sigma, std::ostream &os)
{
    os << "\n┌──────────────────────────────────────┐\n";
    os << "│  sigma(A)                            │\n";
    os << "└──────────────────────────────────────┘\n";
    for (const auto &A : G.VN)
    {
        os << "  sigma(" << std::setw(3) << std::left << A << ") = "
           << langset_str(sigma.at(A)) << "\n";
    }
}

void print_follow_k(const Grammar &G, const FollowMap &FL, std::ostream &os)
{
    os << "\n┌──────────────────────────────────────┐\n";
    os << "│  FOLLOW_" << G.k << "(A)                        │\n";
    os << "└──────────────────────────────────────┘\n";
    for (const auto &A : G.VN)
    {
        auto it = FL.find(A);
        os << "  FOLLOW_" << G.k << "(" << std::setw(3) << std::left << A << ") = "
           << (it != FL.end() ? lang_str(it->second) : "{}") << "\n";
    }
}

void print_ll_result(const Grammar &G, const LLTestResult &res, std::ostream &os)
{
    os << "\n┌──────────────────────────────────────┐\n";
    os << "│  Тест LL(" << G.k << ")                          │\n";
    os << "└──────────────────────────────────────┘\n";
    if (res.is_ll)
    {
        os << "  [OK]  Грамматика является LL(" << G.k << ")-грамматикой.\n";
    }
    else
    {
        os << "  [!!]  Грамматика НЕ является LL(" << G.k << ")-грамматикой.\n";
        os << "  Конфликты:\n";
        for (const auto &v : res.violations)
        {
            os << "    Нетерминал " << v.A
               << ":  правила " << v.rule1 << " и " << v.rule2
               << ",  контекст L = " << lang_str(v.context_L) << "\n"
               << "    Конфликтные аванцепочки: " << lang_str(v.conflict) << "\n";
        }
    }
}

void print_strong_ll_result(const Grammar &G, const StrongLLResult &res, std::ostream &os)
{
    os << "\n┌──────────────────────────────────────┐\n";
    os << "│  Проверка: сильная LL(" << G.k << ")              │\n";
    os << "└──────────────────────────────────────┘\n";
    if (res.is_strong)
    {
        os << "  [OK]  Грамматика является сильной LL(" << G.k << ")-грамматикой.\n";
    }
    else
    {
        os << "  [!!]  Грамматика НЕ является сильной LL(" << G.k << ")-грамматикой.\n";
        os << "  Конфликты:\n";
        for (const auto &v : res.violations)
        {
            os << "    Нетерминал " << v.A
               << ":  правила " << v.rule1 << " и " << v.rule2 << "\n"
               << "    Конфликтные аванцепочки (FOLLOW): "
               << lang_str(v.conflict) << "\n";
        }
    }
}
