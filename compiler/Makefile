CC = g++
EM_CC = em++
LEX = flex
BISON = bison
CFLAGS = -std=c++20
COMPILER = ./out/sus
COMPILER_EM = out/web.js
EM_FLAGS = -s INVOKE_RUN=0 -s EXPORTED_FUNCTIONS='["_main", "_emscripten_force_exit", "FS_quit", "FS_init"]' -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s ALLOW_MEMORY_GROWTH=1
SOURCE = out/lexer.tab.cpp out/parser.tab.cpp src/compiler.cpp src/error.cpp
HEADERS = src/compiler.hpp src/error.hpp
.PHONY: run build web web-clean

build: $(COMPILER)

web: $(COMPILER_EM)

web-clean:
	rm out/web.js
	rm out/web.wasm
run: build
	$(COMPILER) example.js

$(COMPILER): $(SOURCE) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCE) -o $(COMPILER)

$(COMPILER_EM): $(SOURCE) $(HEADERS)
	$(EM_CC) $(CFLAGS) $(SOURCE) -o $(COMPILER_EM) $(EM_FLAGS)

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
