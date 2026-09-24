CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = syngtp
SRCS     = main.cpp grammar.cpp algorithms.cpp
OBJS     = $(SRCS:.cpp=.o)

.PHONY: all clean demo test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


main.o       : main.cpp       grammar.hpp algorithms.hpp
grammar.o    : grammar.cpp    grammar.hpp
algorithms.o : algorithms.cpp algorithms.hpp grammar.hpp

demo: $(TARGET)
	./$(TARGET) --demo

test: $(TARGET)
	@echo "=== Тест 1: Пример 2.2 (LL(1)) ===" && \
	printf "S -> a B S | b\nB -> a | b S B\n" | ./$(TARGET) && \
	echo "=== Тест 2: Пример 2.3 (несильная LL(2)) ===" && \
	printf "k=2\nS -> a A a a | b A b a\nA -> b | eps\n" | ./$(TARGET) && \
	echo "=== Тест 3: Пример 2.6 (арифм. выражения LL(1)) ===" && \
	printf "E -> T E'\nE' -> + T E' | eps\nT -> F T'\nT' -> * F T' | eps\nF -> ( E ) | a\n" | ./$(TARGET) && \
	echo "=== Тест 4: Пример 2.11 (sigma, LL(1)) ===" && \
	printf "S -> A S | eps\nA -> a A | b\n" | ./$(TARGET) && \
	echo "=== Все тесты завершены ==="

clean:
	rm -f $(OBJS) $(TARGET)
