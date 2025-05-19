%{
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "../src/compiler.hpp"
#include "../src/error.hpp"

void yyerror(const char* s) {
  syntaxError(s);
}
int yylex();
BlockNode* program = nullptr;
extern char* yytext;
extern int line_num;
extern int column_num;
extern std::string current_file;
extern void set_current_file(const char* filename);

// Buffer to accumulate source code for error reporting
std::string source_buffer;
std::string source_content;
%}

%glr-parser
%expect 1
%expect-rr 0

%union {
  Node *node;
  char* str;
  int num;
  Type type;
}

%token <num> NUMBER
%token <str> IDENTIFIER STRING MACRO_IDENTIFIER
%token PLUS MINUS STAR SLASH MODULO BREAK CONTINUE
%token SEMICOLON COLON FOR LOOP
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MODULO_ASSIGN
%token EQ LT GT LEQ GEQ NEQ AND OR NOT
%token IF ELSE WHILE LET
%token I32_TYPE STR_TYPE
%token OPEN_PARENTHESES CLOSE_PARENTHESES OPEN_BRACKET CLOSE_BRACKET OPEN_SUBSCRIPT CLOSE_SUBSCRIPT

%type <node> program items item statements statement expression loop_expression
%type <node> expression_statement block 
%type <node> if_statement while_statement for_statement loop_statement declaration assignment
%type <node> macro_expression
%type <node> precedence_max precedence15 precedence14 precedence10 precedence9 precedence6 precedence5 precedence3 precedence2 precedence0
%type <type> type_annotation

%nonassoc IF
%nonassoc ELSE
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MODULO_ASSIGN
%left OR
%left AND
%left EQ NEQ
%left LT GT LEQ GEQ
%left PLUS MINUS
%left STAR SLASH MODULO
%right UMINUS NOT

%%

program:
  items { 
    $$ = program;
  }
  ;

items:
  item { 
    if (!program) {
      program = new BlockNode();
    }
    
    program->addStatement($1);
  }
  | items item {
    program->addStatement($2);
  }
  ;

item:
  statement { $$ = $1; }
  ;

statements:
  /* empty */ { 
    auto block = new BlockNode(); 
    $$ = block;
  }
  | statements statement { 
    auto block = dynamic_cast<BlockNode*>($1);
    block->addStatement($2); 
    $$ = block;
  }
  | statements expression { 
    auto block = dynamic_cast<BlockNode*>($1);
    block->addStatement($2); 
    block->setReturnsValue(true);
    $$ = block;
  }
  ;

statement:
  expression_statement SEMICOLON { $$ = $1; }
  | declaration SEMICOLON { $$ = $1; }
  | assignment SEMICOLON { $$ = $1; }
  | if_statement { $$ = $1; }
  | loop_statement { $$ = $1; }
  | for_statement { $$ = $1; }
  | while_statement { $$ = $1; }
  | CONTINUE SEMICOLON { $$ = new ContinueNode(); }
  | BREAK SEMICOLON { $$ = new BreakNode(); }
  | block { $$ = $1; }
  ;

block:
  OPEN_BRACKET statements CLOSE_BRACKET { $$ = $2; }
  ;

expression_statement:
  expression { $$ = $1; }
  ;

declaration:
  LET IDENTIFIER ASSIGN expression { 
    $$ = new VarDeclNode($2, Type::UNKNOWN, $4); 
  }
  | LET IDENTIFIER COLON type_annotation ASSIGN expression { 
    $$ = new VarDeclNode($2, $4, $6); 
  }
  ;

assignment:
  IDENTIFIER ASSIGN expression { 
    $$ = new AssignNode($1, $3); 
  }
  | IDENTIFIER PLUS_ASSIGN expression { 
    $$ = new AssignNode($1, new BinaryNode("+", new VariableNode($1), $3)); 
  }
  | IDENTIFIER MINUS_ASSIGN expression { 
    $$ = new AssignNode($1, new BinaryNode("-", new VariableNode($1), $3)); 
  }
  | IDENTIFIER STAR_ASSIGN expression { 
    $$ = new AssignNode($1, new BinaryNode("*", new VariableNode($1), $3)); 
  }
  | IDENTIFIER SLASH_ASSIGN expression { 
    $$ = new AssignNode($1, new BinaryNode("/", new VariableNode($1), $3)); 
  }
  | IDENTIFIER MODULO_ASSIGN expression { 
    $$ = new AssignNode($1, new BinaryNode("%", new VariableNode($1), $3)); 
  }
  ;

if_statement:
  IF expression block ELSE block { 
    $$ = new IfNode($2, $3, $5); 
  }
  | IF expression block %prec IF { 
    $$ = new IfNode($2, $3, nullptr); 
  }
  ;

while_statement:
  WHILE expression block { 
    $$ = new LoopNode($2, $3); 
  }
  ;


loop_expression:
  /* empty */ { 
    $$ = nullptr;
  }
  | expression { $$ = $1; }
  | declaration { $$ = $1; }
  | assignment { $$ = $1; }
  ;

loop_statement:
  LOOP block { 
    $$ = new LoopNode(nullptr, $2); 
  }
  ;

for_statement:
  FOR loop_expression SEMICOLON loop_expression SEMICOLON loop_expression block { 
    $$ = new LoopNode($4, $7, $2, $6); 
  }
  ;

macro_expression:
  MACRO_IDENTIFIER OPEN_PARENTHESES expression CLOSE_PARENTHESES { 
    $$ = new MacroNode($1, $3); 
  }
  ;

type_annotation:
  I32_TYPE { $$ = Type::I32; }
  | STR_TYPE { $$ = Type::STR; }
  ;

expression:
  precedence_max { $$ = $1; }
  | macro_expression {$$ = $1; }
  ;

precedence_max:
  precedence15 { $$ = $1; }
  ;

precedence15:
  precedence15 OR precedence14 { $$ = new BinaryNode("||", $1, $3); }
  | precedence14 { $$ = $1; }
  ;

precedence14:
  precedence14 AND precedence10 { $$ = new BinaryNode("&&", $1, $3); }
  | precedence10 { $$ = $1; }
  ;

precedence10:
  precedence10 EQ precedence9 { $$ = new BinaryNode("==", $1, $3); }
  | precedence10 NEQ precedence9 { $$ = new BinaryNode("!=", $1, $3); }
  | precedence9 { $$ = $1; }
  ;

precedence9:
  precedence9 LT precedence6 { $$ = new BinaryNode("<", $1, $3); }
  | precedence9 GT precedence6 { $$ = new BinaryNode(">", $1, $3); }
  | precedence9 GEQ precedence6 { $$ = new BinaryNode(">=",$1, $3); }
  | precedence9 LEQ precedence6 { $$ = new BinaryNode("<=",$1, $3); }
  | precedence6 { $$ = $1; }
  ;

precedence6:
  precedence6 PLUS precedence5 { $$ = new BinaryNode("+", $1, $3); }
  | precedence6 MINUS precedence5 { $$ = new BinaryNode("-", $1, $3); }
  | precedence5 { $$ = $1; }
  ;

precedence5:
  precedence5 STAR precedence3 { $$ = new BinaryNode("*", $1, $3); }
  | precedence5 SLASH precedence3 { $$ = new BinaryNode("/", $1, $3); }
  | precedence5 MODULO precedence3 { $$ = new BinaryNode("%", $1, $3); }
  | precedence3 { $$ = $1; }
  ;

precedence3:
  MINUS precedence3 %prec UMINUS { $$ = new UnaryNode("-", $2); }
  | NOT precedence3 { $$ = new UnaryNode("!", $2); }
  | precedence2 { $$ = $1; }
  ;

precedence2:
  precedence2 OPEN_SUBSCRIPT precedence_max CLOSE_SUBSCRIPT { $$ = new BinaryNode("[]", $1, $3); }
  | precedence0 { $$ = $1; }
  ;

precedence0:
  NUMBER { $$ = new NumberNode($1); }
  | IDENTIFIER { $$ = new VariableNode($1); }
  | STRING { $$ = new StringNode($1); }
  | OPEN_PARENTHESES precedence_max CLOSE_PARENTHESES { $$ = $2; }
  ;

%%

extern FILE * yyin;

#ifdef __EMSCRIPTEN__
bool FORCE_STDIN = true;
#else
bool FORCE_STDIN = false;
#endif

int main(int argc, char **argv) {
  program = nullptr;
  yytext = nullptr;
  line_num = 0;
  column_num = 0;
  current_file = "";

  source_buffer = "";
  source_content = "";

  const auto f = FORCE_STDIN ? "/input.txt" : argv[1];

  if (argc > 1 || FORCE_STDIN) {
    set_current_file(f);
    yyin = fopen(f, "r");
    if (yyin == NULL) {
        printf("syntax: %s filename\n", argv[0]);
        return 1;
    }
    
    load_source_file(f);
  } else {
    set_current_file("<stdin>");
    
    std::string line;
    while (std::getline(std::cin, source_content)) {
        source_buffer += source_content + "\n";
    }
    
    std::istringstream input_stream(source_buffer);
    yyin = tmpfile();
    if (yyin == NULL) {
        std::cerr << "Error: Could not create temporary file for input" << std::endl;
        return 1;
    }
    
    fputs(source_buffer.c_str(), yyin);
    rewind(yyin);
    
    load_source_from_string(source_buffer);
  }
  
  if (yyparse() == 0 && program) {
    std::cerr << "\nParsing completed successfully. AST:" << std::endl;
    program->print();
    std::string asm_code = compile(program);
    std::cout << asm_code << std::endl;;
  }
  
  return 0;
}
