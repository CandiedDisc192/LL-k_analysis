#include "grammar.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>

struct DemoCase
{
    std::string name;
    std::string description;
    std::string grammar_text;
    int k;
};

static const std::vector<DemoCase> DEMOS = {
    {"Пример 2.2",
     "Простая LL(1)-грамматика (слайды стр.12-16)\n"
     "  G=({S,B},{a,b},P,S)  P={ S->aBS|b, B->a|bSB }",
     "S -> a B S | b\nB -> a | b S B\n", 1},
    {"Пример 2.3",
     "Несильная LL(2)-грамматика (слайды стр.49-52)\n"
     "  G=({S,A},{a,b},P,S)  P={ S->aAaa|bAba, A->b|eps }",
     "k=2\nS -> a A a a | b A b a\nA -> b | eps\n", 2},
    {"Пример 2.6",
     "LL(1)-грамматика арифметических выражений (слайды стр.92-96)\n"
     "  G=({E,E',T,T',F},{a,+,*,(,)},P,E)",
     "E  -> T E'\nE' -> + T E' | eps\nT  -> F T'\nT' -> * F T' | eps\nF  -> ( E ) | a\n", 1},
    {"Пример 2.11",
     "Вычисление sigma(A), тест LL(1) (слайды стр.201-213)\n"
     "  G=({S,A},{a,b},P,S)  P={ S->AS|eps, A->aA|b }",
     "S -> A S | eps\nA -> a A | b\n", 1},
    {"Левая рекурсия (не-LL)",
     "Леворекурсивная грамматика — не LL(k) ни при каком k\n"
     "  G=({S},{a,b},P,S)  P={ S->Sa|b }",
     "S -> S a | b\n", 1}};

void run_all(const Grammar &G, std::ostream &os = std::cout)
{
    os << "\n══════════════════════════════════════════════════════\n";
    os << "  LL(k)-анализ\n";
    os << "══════════════════════════════════════════════════════\n";
    G.print(os);

    os << "\n[Алгоритм 2.5] Вычисление FIRST_" << G.k << "\n";
    auto F = compute_first_k(G);
    print_first_k(G, F, os);

    os << "\n[Алгоритм 2.6] Вычисление sigma'(A,B) и sigma(A)\n";
    auto sp = compute_sigma_prime(G, F);
    auto sigma = compute_sigma(G, sp);
    print_sigma_prime(G, sp, os);
    print_sigma(G, sigma, os);

    os << "\n[Алгоритм 2.7] Вычисление FOLLOW_" << G.k << "\n";
    auto FL = compute_follow_k(G, sigma);
    print_follow_k(G, FL, os);

    os << "\n[Алгоритм 2.4] Тест LL(" << G.k << ")\n";
    auto ll_res = test_ll_k(G, F, sigma);
    print_ll_result(G, ll_res, os);

    os << "\n[Проверка] Сильная LL(" << G.k << ")\n";
    auto sll_res = test_strong_ll_k(G, F, FL);
    print_strong_ll_result(G, sll_res, os);

    os << "\n──────────────────────────────────────────────────────\n";
    os << "  ИТОГ:  LL(" << G.k << ") = " << (ll_res.is_ll ? "ДА" : "НЕТ");
    os << "   |   Сильная LL(" << G.k << ") = " << (sll_res.is_strong ? "ДА" : "НЕТ");
    if (ll_res.is_ll && !sll_res.is_strong)
        os << "\n  (LL(" << G.k << "), но не сильная — см. Пример 2.3 из лекций)";
    os << "\n══════════════════════════════════════════════════════\n";
}

Grammar parse_with_k(const std::string &raw_text, int default_k)
{
    int k = default_k;
    std::string text;
    bool k_found = false;
    std::istringstream ss(raw_text);
    std::string line;
    while (std::getline(ss, line))
    {
        while (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (!k_found && (line.empty() || line[0] == '#'))
        {
            text += line + "\n";
            continue;
        }

        if (!k_found && line.size() > 2 && line[0] == 'k' && line[1] == '=')
        {
            try
            {
                k = std::stoi(line.substr(2));
            }
            catch (...)
            {
            }
            k_found = true;
            continue;
        }
        text += line + "\n";
    }
    return parse_grammar(text, k);
}

void run_demos()
{
    for (size_t i = 0; i < DEMOS.size(); i++)
    {
        const auto &d = DEMOS[i];
        std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
        std::cout << "║  ДЕМО [" << (i + 1) << "/" << DEMOS.size() << "]: " << d.name << "\n";

        {
            std::istringstream ds(d.description);
            std::string dl;
            while (std::getline(ds, dl))
                std::cout << "║  " << dl << "\n";
        }
        std::cout << "╚══════════════════════════════════════════════════════╝\n";

        Grammar G = parse_with_k(d.grammar_text, d.k);
        run_all(G);

        if (i + 1 < DEMOS.size())
        {
            std::cout << "\nНажмите Enter для следующего примера...";
            std::string dummy;
            std::getline(std::cin, dummy);
        }
    }
    std::cout << "\n[Все демо-примеры завершены]\n";
}

Grammar input_grammar_interactive(int k)
{
    std::cout << "\nВведите правила грамматики. Пустая строка — завершение.\n";
    std::cout << "Формат:   LHS -> sym1 sym2 | sym3 sym4\n";
    std::cout << "ε-правило: A -> eps\n\n";
    std::string text;
    std::string line;
    while (true)
    {
        std::cout << "  rule> ";
        std::getline(std::cin, line);
        if (line.empty() || line == "\r")
            break;
        text += line + "\n";
    }
    if (text.empty())
    {
        std::cout << "[!] Пустой ввод.\n";
        return Grammar{};
    }
    Grammar G = parse_grammar(text, k);
    if (G.VN.empty())
    {
        std::cout << "[!] Не удалось разобрать грамматику.\n";
        return Grammar{};
    }
    return G;
}

void print_help()
{
    std::cout << R"(
┌─────────────────────────────────────────────────────────────┐
│  Пакет демонстрации LL(k)-анализа КС-грамматик │
├─────────────────────────────────────────────────────────────┤
│  Реализованные алгоритмы (лекции ТФЯиТ, Часть 2):          │
│    Алг. 2.5  FIRST_k(A)                                     │
│    Алг. 2.6  sigma'(A,B) и sigma(A)                        │
│    Алг. 2.7  FOLLOW_k(A)                                    │
│    Алг. 2.4  Тест LL(k) — Теорема 2.1                      │
│              Тест сильной LL(k) — Определение 2.7           │
├─────────────────────────────────────────────────────────────┤
│  Команды:                                                    │
│    grammar       — ввести грамматику вручную                │
│    load <файл>   — загрузить грамматику из файла            │
│    k <n>         — установить lookahead (по умолч. k=1)     │
│    run           — все алгоритмы                            │
│    first         — только FIRST_k                           │
│    follow        — только FOLLOW_k                          │
│    sigma         — только sigma'(A,B) и sigma(A)           │
│    test          — тест LL(k)                               │
│    strong        — тест сильной LL(k)                      │
│    demo          — встроенные примеры из лекций             │
│    print         — показать текущую грамматику              │
│    help          — эта справка                              │
│    quit / exit   — выход                                    │
├─────────────────────────────────────────────────────────────┤
│  Формат грамматики:                                         │
│    # k задаётся в первой строке (необязательно):            │
│    k=2                                                      │
│    S -> a A a a | b A b a                                   │
│    A -> b | eps                                             │
│  Нетерминалы = все символы, стоящие слева от ->            │
└─────────────────────────────────────────────────────────────┘
)";
}

int main(int argc, char *argv[])
{
    int cur_k = 1;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--demo")
        {
            run_demos();
            return 0;
        }
        if (arg == "--help" || arg == "-h")
        {
            print_help();
            return 0;
        }

        std::ifstream f(arg);
        if (!f)
        {
            std::cerr << "Ошибка: не удалось открыть " << arg << "\n";
            return 1;
        }
        std::string text((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
        Grammar G = parse_with_k(text, cur_k);
        if (!G.VN.empty())
            run_all(G);
        else
            std::cerr << "[!] Не удалось разобрать грамматику из файла.\n";
        return 0;
    }

    if (!isatty(fileno(stdin)))
    {
        std::string text((std::istreambuf_iterator<char>(std::cin)),
                         std::istreambuf_iterator<char>());
        Grammar G = parse_with_k(text, cur_k);
        if (!G.VN.empty())
            run_all(G);
        else
            std::cerr << "[!] Не удалось разобрать грамматику из stdin.\n";
        return 0;
    }

    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "  LL(k)-анализ КС-грамматик\n";
    std::cout << "  Алгоритмы 2.4–2.7 из лекций ТФЯиТ Часть 2\n";
    std::cout << "  'help' — справка   'demo' — примеры из лекций\n";
    std::cout << "═══════════════════════════════════════════════\n";

    Grammar G;
    FirstMap F;
    SigmaPrimeMap sp;
    SigmaMap sigma;
    FollowMap FL;
    bool computed = false;

    auto ensure_computed = [&]()
    {
        if (!computed && !G.VN.empty())
        {
            F = compute_first_k(G);
            sp = compute_sigma_prime(G, F);
            sigma = compute_sigma(G, sp);
            FL = compute_follow_k(G, sigma);
            computed = true;
        }
    };

    std::string line;
    while (true)
    {
        std::cout << "\nLL(k)> ";
        std::cout.flush();
        if (!std::getline(std::cin, line))
            break;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();
        while (!line.empty() && line.front() == ' ')
            line.erase(line.begin());
        if (line.empty())
            continue;

        std::istringstream ls(line);
        std::string cmd;
        ls >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "quit" || cmd == "exit" || cmd == "q")
        {
            std::cout << "До свидания.\n";
            break;
        }
        else if (cmd == "help" || cmd == "h")
        {
            print_help();
        }
        else if (cmd == "demo")
        {
            run_demos();
        }
        else if (cmd == "print" || cmd == "p")
        {
            if (G.VN.empty())
                std::cout << "[!] Грамматика не задана.\n";
            else
                G.print();
        }
        else if (cmd == "k")
        {
            int n;
            if (ls >> n && n >= 1)
            {
                cur_k = n;
                if (!G.VN.empty())
                {
                    G.k = cur_k;
                    computed = false;
                }
                std::cout << "  k = " << cur_k << "\n";
            }
            else
                std::cout << "[!] Использование: k <число >= 1>\n";
        }
        else if (cmd == "grammar" || cmd == "g")
        {
            G = input_grammar_interactive(cur_k);
            computed = false;
        }
        else if (cmd == "load")
        {
            std::string fname;
            ls >> fname;
            if (fname.empty())
            {
                std::cout << "[!] Использование: load <файл>\n";
                continue;
            }
            std::ifstream f(fname);
            if (!f)
            {
                std::cout << "[!] Не удалось открыть: " << fname << "\n";
                continue;
            }
            std::string text((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
            G = parse_with_k(text, cur_k);
            computed = false;
            if (!G.VN.empty())
                std::cout << "[OK] Загружено из " << fname << "\n";
            else
                std::cout << "[!] Ошибка разбора.\n";
        }
        else if (cmd == "run" || cmd == "all")
        {
            if (G.VN.empty())
            {
                std::cout << "[!] Введите грамматику ('grammar' или 'load')\n";
                continue;
            }
            G.k = cur_k;
            computed = false;
            run_all(G);
            ensure_computed();
        }
        else if (cmd == "first")
        {
            if (G.VN.empty())
            {
                std::cout << "[!] Введите грамматику.\n";
                continue;
            }
            G.k = cur_k;
            ensure_computed();
            print_first_k(G, F);
        }
        else if (cmd == "follow")
        {
            if (G.VN.empty())
            {
                std::cout << "[!] Введите грамматику.\n";
                continue;
            }
            G.k = cur_k;
            ensure_computed();
            print_follow_k(G, FL);
        }
        else if (cmd == "sigma")
        {
            if (G.VN.empty())
            {
                std::cout << "[!] Введите грамматику.\n";
                continue;
            }
            G.k = cur_k;
            ensure_computed();
            print_sigma_prime(G, sp);
            print_sigma(G, sigma);
        }
        else if (cmd == "test")
        {
            if (G.VN.empty())
            {
                std::cout << "[!] Введите грамматику.\n";
                continue;
            }
            G.k = cur_k;
            ensure_computed();
            print_ll_result(G, test_ll_k(G, F, sigma));
        }
        else if (cmd == "strong")
        {
            if (G.VN.empty())
            {
                std::cout << "[!] Введите грамматику.\n";
                continue;
            }
            G.k = cur_k;
            ensure_computed();
            print_strong_ll_result(G, test_strong_ll_k(G, F, FL));
        }
        else
        {
            std::cout << "[?] Неизвестная команда: " << cmd << ". Введите 'help'.\n";
        }
    }
    return 0;
}
