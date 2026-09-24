#pragma once

#include "grammar.hpp"

using FirstMap = std::map<std::string, Language>;
using SigmaPrimeMap = std::map<std::pair<std::string, std::string>, LangSet>;
using SigmaMap = std::map<std::string, LangSet>;
using FollowMap = std::map<std::string, Language>;

FirstMap compute_first_k(const Grammar &G);

Language first_k_seq(const std::vector<std::string> &alpha,
                     const Grammar &G, const FirstMap &F);

SigmaPrimeMap compute_sigma_prime(const Grammar &G, const FirstMap &F);
SigmaMap compute_sigma(const Grammar &G, const SigmaPrimeMap &sp);

FollowMap compute_follow_k(const Grammar &G, const SigmaMap &sigma);

struct LLViolation
{
    std::string A;
    int rule1, rule2;
    Language context_L;
    Language conflict;
};

struct LLTestResult
{
    bool is_ll;
    std::vector<LLViolation> violations;
};

LLTestResult test_ll_k(const Grammar &G, const FirstMap &F, const SigmaMap &sigma);

struct StrongLLResult
{
    bool is_strong;
    std::vector<LLViolation> violations;
};
StrongLLResult test_strong_ll_k(const Grammar &G, const FirstMap &F, const FollowMap &FL);

std::string word_str(const Word &w);
std::string lang_str(const Language &L);
std::string langset_str(const LangSet &LS);

void print_first_k(const Grammar &G, const FirstMap &F,
                   std::ostream &os = std::cout);
void print_sigma_prime(const Grammar &G, const SigmaPrimeMap &sp,
                       std::ostream &os = std::cout);
void print_sigma(const Grammar &G, const SigmaMap &sigma,
                 std::ostream &os = std::cout);
void print_follow_k(const Grammar &G, const FollowMap &FL,
                    std::ostream &os = std::cout);
void print_ll_result(const Grammar &G, const LLTestResult &res,
                     std::ostream &os = std::cout);
void print_strong_ll_result(const Grammar &G, const StrongLLResult &res,
                            std::ostream &os = std::cout);
