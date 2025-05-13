%{
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "../src/interpreter.hpp"
#include "../src/error.hpp"

void yyerror(const char* s) {
  syntaxError(s);
}
int yylex();
ProgramNode* program = nullptr;
extern char* yytext;
extern int line_num;
extern int column_num;
extern std::string current_file;
extern void set_current_file(const char* filename);

// Buffer to accumulate source code for error reporting
std::string source_buffer;
%}

%glr-parser
%expect 3
%expect-rr 15

%union {
  Node *node;
  char* str;
  int num;
  std::vector<FunctionParamNode>* params;
  Type type;
}

%token <num> NUMBER
%token <str> IDENTIFIER STRING
%token PLUS MINUS STAR SLASH MODULO
%token SEMICOLON COLON COMMA ARROW
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MODULO_ASSIGN
%token EQ LT GT LEQ GEQ NEQ AND OR NOT
%token TRUE FALSE
%token IF ELSE WHILE LET MUT FN RETURN PRINT
%token I32_TYPE STR_TYPE BOOL_TYPE UNIT_TYPE
%token OPEN_PARENTHESES CLOSE_PARENTHESES OPEN_BRACKET CLOSE_BRACKET

%type <node> program items item statements statement expression
%type <node> expression_statement block 
%type <node> if_statement while_statement declaration assignment
%type <node> function_definition return_statement
%type <node> print_statement
%type <node> precedence16 precedence15 precedence14 precedence10 precedence9 precedence6 precedence5 precedence3 precedence0
%type <node> function_call
%type <params> parameters parameter_list
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
      program = new ProgramNode();
      auto mainBlock = new BlockNode();
      program->setMainBlock(mainBlock);
    }
    
    if (auto func = dynamic_cast<FunctionNode*>($1)) {
      program->addFunction($1);
    } else {
      auto mainBlock = dynamic_cast<BlockNode*>(program->mainBlock.get());
      if (mainBlock) {
        mainBlock->addStatement($1);
      }
    }
  }
  | items item {
    if (auto func = dynamic_cast<FunctionNode*>($2)) {
      program->addFunction($2);
    } else {
      auto mainBlock = dynamic_cast<BlockNode*>(program->mainBlock.get());
      if (mainBlock) {
        mainBlock->addStatement($2);
      }
    }
  }
  ;

item:
  function_definition { $$ = $1; }
  | statement { $$ = $1; }
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
  | while_statement { $$ = $1; }
  | return_statement SEMICOLON { $$ = $1; }
  | print_statement SEMICOLON { $$ = $1; }
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
    $$ = new VarDeclNode($2, false, Type::UNKNOWN, $4); 
  }
  | LET MUT IDENTIFIER ASSIGN expression { 
    $$ = new VarDeclNode($3, true, Type::UNKNOWN, $5); 
  }
  | LET IDENTIFIER COLON type_annotation ASSIGN expression { 
    $$ = new VarDeclNode($2, false, $4, $6); 
  }
  | LET MUT IDENTIFIER COLON type_annotation ASSIGN expression { 
    $$ = new VarDeclNode($3, true, $5, $7); 
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
    $$ = new WhileNode($2, $3); 
  }
  ;

return_statement:
  RETURN expression { 
    $$ = new ReturnNode($2); 
  }
  | RETURN { 
    $$ = new ReturnNode(new UnitNode()); 
  }
  ;

print_statement:
  PRINT OPEN_PARENTHESES STRING CLOSE_PARENTHESES { 
    $$ = new PrintNode($3); 
  }
  | PRINT OPEN_PARENTHESES STRING COMMA expression CLOSE_PARENTHESES { 
    auto node = new PrintNode($3);
    node->addArgument($5);
    $$ = node;
  }
  | PRINT OPEN_PARENTHESES STRING COMMA expression COMMA expression CLOSE_PARENTHESES { 
    auto node = new PrintNode($3);
    node->addArgument($5);
    node->addArgument($7);
    $$ = node;
  }
  | PRINT OPEN_PARENTHESES STRING COMMA expression COMMA expression COMMA expression CLOSE_PARENTHESES { 
    auto node = new PrintNode($3);
    node->addArgument($5);
    node->addArgument($7);
    node->addArgument($9);
    $$ = node;
  }
  ;

function_definition:
  FN IDENTIFIER OPEN_PARENTHESES parameters CLOSE_PARENTHESES ARROW type_annotation block { 
    $$ = new FunctionNode($2, *$4, $7, $8); 
    delete $4;
  }
  | FN IDENTIFIER OPEN_PARENTHESES parameters CLOSE_PARENTHESES block { 
    $$ = new FunctionNode($2, *$4, Type::UNIT, $6); 
    delete $4;
  }
  ;

parameters:
  parameter_list { $$ = $1; }
  | { $$ = new std::vector<FunctionParamNode>(); }
  ;

parameter_list:
  IDENTIFIER COLON type_annotation { 
    $$ = new std::vector<FunctionParamNode>(); 
    $$->emplace_back($1, $3); 
  }
  | parameter_list COMMA IDENTIFIER COLON type_annotation { 
    $$ = $1; 
    $$->emplace_back($3, $5); 
  }
  ;

type_annotation:
  I32_TYPE { $$ = Type::I32; }
  | STR_TYPE { $$ = Type::STR; }
  | BOOL_TYPE { $$ = Type::BOOL; }
  | UNIT_TYPE { $$ = Type::UNIT; }
  ;

expression:
  precedence16 { $$ = $1; }
  ;

precedence16:
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
  | precedence0 { $$ = $1; }
  ;

precedence0:
  FALSE { $$ = new BoolNode(false); }
  | TRUE { $$ = new BoolNode(true); }
  | NUMBER { $$ = new NumberNode($1); }
  | IDENTIFIER { $$ = new VariableNode($1); }
  | STRING { $$ = new StringNode($1); }
  | OPEN_PARENTHESES expression CLOSE_PARENTHESES { $$ = $2; }
  | function_call { $$ = $1; }
  | block { $$ = $1; }
  ;

function_call:
  IDENTIFIER OPEN_PARENTHESES CLOSE_PARENTHESES { 
    auto node = new FunctionCallNode($1); 
    $$ = node;
  }
  | IDENTIFIER OPEN_PARENTHESES expression CLOSE_PARENTHESES { 
    auto node = new FunctionCallNode($1); 
    node->addArgument($3);
    $$ = node;
  }
  | IDENTIFIER OPEN_PARENTHESES expression COMMA expression CLOSE_PARENTHESES { 
    auto node = new FunctionCallNode($1); 
    node->addArgument($3);
    node->addArgument($5);
    $$ = node;
  }
  | IDENTIFIER OPEN_PARENTHESES expression COMMA expression COMMA expression CLOSE_PARENTHESES { 
    auto node = new FunctionCallNode($1); 
    node->addArgument($3);
    node->addArgument($5);
    node->addArgument($7);
    $$ = node;
  }
  ;

%%

extern FILE * yyin;
int main(int argc, char **argv) {
   // Set up source capture for error reporting
   std::string source_content;
   
   if (argc > 1) {
      set_current_file(argv[1]);
      yyin = fopen(argv[1], "r");
      if (yyin == NULL) {
         printf("syntax: %s filename\n", argv[0]);
         return 1;
      }
      
      // Load the source file for error reporting
      load_source_file(argv[1]);
   } else {
      // Reading from stdin, need to capture the input
      set_current_file("<stdin>");
      
      // Read complete input from stdin for error reporting
      std::string line;
      while (std::getline(std::cin, source_content)) {
         source_buffer += source_content + "\n";
      }
      
      // Reset stdin to start reading again
      std::istringstream input_stream(source_buffer);
      yyin = tmpfile();
      if (yyin == NULL) {
         printf("Error: Could not create temporary file for input\n");
         return 1;
      }
      
      fputs(source_buffer.c_str(), yyin);
      rewind(yyin);
      
      // Load the source content for error reporting
      load_source_from_string(source_buffer);
   }
   
   if (yyparse() == 0 && program) {
      std::cout << "Parsing completed successfully. AST:" << std::endl;
      program->print();
      std::cout << "\nOutput:" << std::endl;
      program->evaluate();
   }
   
   return 0;
}
