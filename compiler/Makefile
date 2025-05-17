CC = g++
LEX = flex
BISON = bison
CFLAGS = -std=c++20
COMPILER = out/js

.PHONY: run build

build: $(COMPILER)

run: build
	$(COMPILER) example.js

$(COMPILER): out/lexer.tab.cpp out/parser.tab.cpp src/ast.cpp src/error.cpp
	$(CC) $(CFLAGS) out/lexer.tab.cpp out/parser.tab.cpp src/ast.cpp src/error.cpp -o $(COMPILER)

out/lexer.tab.cpp: src/lexer.l
	$(LEX) -o out/lexer.tab.cpp src/lexer.l

out/parser.tab.cpp: src/parser.y
	$(BISON) -Wno-other -d -o out/parser.tab.cpp src/parser.y

dev: build
	@echo "Running dev test...\n"
	$(COMPILER) < test.rs;

test: build
	@echo "Running tests...\n"
	@for test in tests/legit/**; do \
		echo "Running $$test..."; \
		$(COMPILER) < $$test; \
		echo " "; \
	done

test_bad_examples: build
	@echo "Running bad examples (these should fail with detailed errors)...\n"
	@for test in tests/bad_examples/**; do \
		echo "Running $$test..."; \
		$(COMPILER) < $$test || echo "Test failed as expected"; \
		echo " "; \
	done

clean:
	rm -rf out/*
	mkdir -p out
