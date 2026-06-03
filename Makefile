.DEFAULT_GOAL := kriol

.PHONY: kriol debug release clean test
.PHONY: kriol debug release clean test valgrind

CC = clang++-19
CC_C = clang-19

OUTPUT = kriol

LLVM_CXXFLAGS := $(filter-out -fno-exceptions -std=%, $(shell llvm-config --cxxflags))
LLVM_LDFLAGS  := $(shell llvm-config --ldflags --libs core native --system-libs 2>/dev/null)

CPP_FLAGS = -std=c++17

DBG_FLAGS = -g -Wall

RLS_FLAGS = -O2 -DNDEBUG

OBJS = main.o cli.o sema.o codegen.o parser.o scanner.o

RUNTIME_OBJ = runtime/kriol_runtime.o

# KRIOL_RUNTIME_OBJ is a path relative to the kriol executable.
# codegen.cc resolves it at runtime against the executable's directory.
RUNTIME_DEFINE = -DKRIOL_RUNTIME_OBJ='"$(RUNTIME_OBJ)"'

# Sources that must NOT see LLVM headers (bison/flex generated code has
# namespace-level std:: references that clash with LLVM's extra defines)
SRCS = main.cc \
	   src/kriol/sema.cc \
	   parser.cc scanner.cc

LLVM_SRC = src/kriol/cli.cc \
	       src/kriol/codegen.cc

RUNTIME_SRC = runtime/kriol_runtime.c

kriol: debug
	@echo "\n\nRun the compiler with:"
	@echo "\n  ./kriol --help\n"

dbg-obj: $(SRCS) $(LLVM_SRC) $(RUNTIME_SRC)
	@echo "~~ Debug build ~~"
	$(CC_C) -c $(DBG_FLAGS) $(RUNTIME_SRC) -o $(RUNTIME_OBJ)
	$(CC) -c $(CPP_FLAGS) $(DBG_FLAGS) $(SRCS)
	$(CC) -c $(CPP_FLAGS) $(DBG_FLAGS) $(LLVM_CXXFLAGS) $(RUNTIME_DEFINE) $(LLVM_SRC)

debug: dbg-obj
	$(CC) -o $(OUTPUT) $(DBG_FLAGS) $(OBJS) $(LLVM_LDFLAGS)

rls-obj: $(SRCS) $(LLVM_SRC) $(RUNTIME_SRC)
	@echo "~~ Release build ~~"
	$(CC_C) -c $(RLS_FLAGS) $(RUNTIME_SRC) -o $(RUNTIME_OBJ)
	$(CC) -c $(CPP_FLAGS) $(RLS_FLAGS) $(SRCS)
	$(CC) -c $(CPP_FLAGS) $(RLS_FLAGS) $(LLVM_CXXFLAGS) $(RUNTIME_DEFINE) $(LLVM_SRC)

release: rls-obj
	$(CC) -o $(OUTPUT) $(CPP_FLAGS) $(RLS_FLAGS) $(OBJS) $(LLVM_LDFLAGS)

parser.cc parser.hh: rules/parser.y
	bison -dt rules/parser.y -o parser.cc

scanner.cc: rules/scanner.l
	flex -o scanner.cc rules/scanner.l

clean:
	rm -f *.o runtime/*.o kriol parser.cc parser.hh scanner.cc

valgrind: debug
	@if [ -z "$(FILE)" ]; \
	then \
		echo "Usage: make valgrind FILE=<path/to/file.kl>"; \
		exit 1; \
	fi
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./kriol $(FILE)

test: kriol
	@echo "\n~~ Running tests ~~\n"; \
	pass=0; fail=0; \
	for f in examples/*.kl; do \
		printf "  %-44s" "$$f"; \
		tmpbin=$$(mktemp /tmp/kriol_XXXX); \
		if ./kriol "$$f" -o "$$tmpbin" 2>/dev/null && \
		   timeout 5 "$$tmpbin" > /dev/null 2>&1; then \
			echo " PASS"; pass=$$((pass+1)); \
		else \
			echo " FAIL"; fail=$$((fail+1)); \
		fi; \
		rm -f "$$tmpbin"; \
	done; \
	if [ -d tests/pass ]; then \
		for f in tests/pass/*.kl; do \
			[ -f "$$f" ] || continue; \
			printf "  %-44s" "$$f"; \
			tmpbin=$$(mktemp /tmp/kriol_XXXX); \
			if ./kriol "$$f" -o "$$tmpbin" 2>/dev/null && \
			   timeout 5 "$$tmpbin" > /dev/null 2>&1; then \
				echo " PASS"; pass=$$((pass+1)); \
			else \
				echo " FAIL"; fail=$$((fail+1)); \
			fi; \
			rm -f "$$tmpbin"; \
		done; \
	fi; \
	if [ -d tests/fail ]; then \
		for f in tests/fail/*.kl; do \
			[ -f "$$f" ] || continue; \
			printf "  %-44s" "$$f"; \
			tmpbin=$$(mktemp /tmp/kriol_fail_bin_XXXX); \
			tmperr=$$(mktemp /tmp/kriol_fail_err_XXXX); \
			expect="$$f.err"; \
			if ./kriol "$$f" -o "$$tmpbin" > /dev/null 2>"$$tmperr"; then \
				echo " FAIL (should have been rejected)"; fail=$$((fail+1)); \
			else \
				if [ -f "$$expect" ]; then \
					missing=0; \
					while IFS= read -r needle || [ -n "$$needle" ]; do \
						case "$$needle" in \
							''|'#'*) continue ;; \
						esac; \
						if ! grep -Fq "$$needle" "$$tmperr"; then \
							missing=1; \
							echo " FAIL (missing diagnostic fragment: $$needle)"; \
							break; \
						fi; \
					done < "$$expect"; \
					if [ $$missing -eq 0 ]; then \
						echo " PASS (rejected, diagnostics match)"; pass=$$((pass+1)); \
					else \
						echo "      stderr:"; sed 's/^/      /' "$$tmperr"; fail=$$((fail+1)); \
					fi; \
				else \
					echo " PASS (rejected)"; pass=$$((pass+1)); \
				fi; \
			fi; \
			rm -f "$$tmpbin" "$$tmperr"; \
		done; \
	fi; \
	echo "\n  $$pass/$$((pass+fail)) passed\n"; \
	[ $$fail -eq 0 ]
