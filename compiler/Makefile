CC = g++
LEX = flex
BISON = bison
CFLAGS = -std=c++20
INTERPRETER = out/js

.PHONY: run build

build: $(INTERPRETER)

run: build
	$(INTERPRETER) example.js

$(INTERPRETER): out/lexer.tab.cpp out/parser.tab.cpp src/interpreter.cpp src/error.cpp
	$(CC) $(CFLAGS) out/lexer.tab.cpp out/parser.tab.cpp src/interpreter.cpp src/error.cpp -o $(INTERPRETER)

out/lexer.tab.cpp: src/lexer.l
	$(LEX) -o out/lexer.tab.cpp src/lexer.l

out/parser.tab.cpp: src/parser.y
	$(BISON) -Wno-other -d -o out/parser.tab.cpp src/parser.y

dev: build
	@echo "Running dev test...\n"
	$(INTERPRETER) < test.rs;

test: build
	@echo "Running tests...\n"
	@for test in tests/legit/**; do \
		echo "Running $$test..."; \
		$(INTERPRETER) < $$test; \
		echo " "; \
	done

test_bad_examples: build
	@echo "Running bad examples (these should fail with detailed errors)...\n"
	@for test in tests/bad_examples/**; do \
		echo "Running $$test..."; \
		$(INTERPRETER) < $$test || echo "Test failed as expected"; \
		echo " "; \
	done

clean:
	rm -rf out/*
	mkdir -p out
