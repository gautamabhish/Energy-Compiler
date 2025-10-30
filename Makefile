# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread

# Flex & Bison files
LEX_SRC := lexer.l
YACC_SRC := parser.y
LEX_OUT := lex.yy.cpp
YACC_OUT := parser.tab.c
YACC_HDR := parser.tab.hpp

# Source files
SRCS := $(YACC_OUT) $(LEX_OUT) ir_builder.cpp hardware_manager.cpp core_scheduler.cpp
OBJS := $(SRCS:.cpp=.o)

# Output binary
TARGET := mycompiler

# Default target
all: $(TARGET)

# Rule to build the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Bison: generate parser
$(YACC_OUT) $(YACC_HDR): $(YACC_SRC)
	bison -d $(YACC_SRC)

# Flex: generate lexer
$(LEX_OUT): $(LEX_SRC)
	flex -o $(LEX_OUT) $(LEX_SRC)

# Generic rule for compiling C++ sources
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(LEX_OUT) $(YACC_OUT) $(YACC_HDR) $(TARGET)

# For debugging (optional)
run: $(TARGET)
	./$(TARGET)  <model.nn
