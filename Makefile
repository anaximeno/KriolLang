.DEFAULT_GOAL := kriol

.PHONY: kriol debug release clean test

CC = clang++
CC_C = cc

OUTPUT = kriol

EXAMPLES = examples/hello-world.kl \
	   examples/example01.kl \
	   examples/example02.kl \
	   examples/example03.kl

LLVM_CXXFLAGS := $(filter-out -fno-exceptions, $(shell llvm-config --cxxflags))
LLVM_LDFLAGS  := $(shell llvm-config --ldflags --libs core native --system-libs 2>/dev/null)

FLAGS = -std=c++17

DBG_FLAGS = $(FLAGS) -g -Wall

RLS_FLAGS = $(FLAGS) -O3 -DNDEBUG

OBJS = main.o ast.o cli.o llvm_codegen.o parser.o scanner.o

# Sources that must NOT see LLVM headers (bison/flex generated code has
# namespace-level std:: references that clash with LLVM's extra defines)
SRCS = main.cpp \
	   src/kriol/ast.cc \
	   src/kriol/cli.cc \
	   parser.cc scanner.cc

LLVM_SRC = src/kriol/llvm_codegen.cc

# The default build is debug.
# Change to release if wanted.
kriol: debug
	@echo "\n\nRun the compiler with:"
	@echo "\n  ./kriol --help\n"

dbg-obj: $(SRCS) $(LLVM_SRC)
	@echo "~~ Debug build ~~"
	$(CC) -c $(DBG_FLAGS) $(SRCS)
	$(CC) -c $(DBG_FLAGS) $(LLVM_CXXFLAGS) $(LLVM_SRC)

debug: dbg-obj
	$(CC) -o $(OUTPUT) $(DBG_FLAGS) $(OBJS) $(LLVM_LDFLAGS)

rls-obj: $(SRCS) $(LLVM_SRC)
	@echo "~~ Release build ~~"
	$(CC) -c $(RLS_FLAGS) $(SRCS)
	$(CC) -c $(RLS_FLAGS) $(LLVM_CXXFLAGS) $(LLVM_SRC)

release: rls-obj
	$(CC) -o $(OUTPUT) $(RLS_FLAGS) $(OBJS) $(LLVM_LDFLAGS)

parser.cc parser.hh:
	bison -dt rules/parser.y -o parser.cc

scanner.cc:
	flex -o scanner.cc rules/scanner.l

clean:
	rm *.o kriol parser.cc parser.hh scanner.cc

test: kriol
	@echo "\n~~ Running tests ~~\n"; \
	pass=0; fail=0; \
	for f in $(EXAMPLES); do \
		printf "  %-38s" "$$f"; \
		tmpbin=$$(mktemp /tmp/kriol_XXXX); \
		if ./kriol "$$f" -o "$$tmpbin" 2>/dev/null && \
		   timeout 5 "$$tmpbin" > /dev/null 2>&1; then \
			echo " PASS"; pass=$$((pass+1)); \
		else \
			echo " FAIL"; fail=$$((fail+1)); \
		fi; \
		rm -f "$$tmpbin"; \
	done; \
	echo "\n  $$pass/$$((pass+fail)) passed\n"; \
	[ $$fail -eq 0 ]
