%{
#include "../src/interpreter.hpp"
#include "parser.tab.hpp"
#include <iostream>
#define DEBUG_LEXER 1
%}

%%
           
[0-9]+           { 
    if (DEBUG_LEXER) std::cout << "NUMBER: " << yytext << std::endl;
    yylval.num = atoi(yytext); 
    return NUMBER; 
}
"-"?[0-9]+       { 
    if (DEBUG_LEXER) std::cout << "NEGATIVE NUMBER: " << yytext << std::endl;
    yylval.num = atoi(yytext); 
    return NUMBER; 
}
"("              { if (DEBUG_LEXER) std::cout << "OPEN_PARENTHESES" << std::endl; return OPEN_PARENTHESES; }
")"              { if (DEBUG_LEXER) std::cout << "CLOSE_PARENTHESES" << std::endl; return CLOSE_PARENTHESES; }
"{"              { if (DEBUG_LEXER) std::cout << "OPEN_BRACKET" << std::endl; return OPEN_BRACKET; }
"}"              { if (DEBUG_LEXER) std::cout << "CLOSE_BRACKET" << std::endl; return CLOSE_BRACKET; }
"=="             { if (DEBUG_LEXER) std::cout << "EQ" << std::endl; return EQ; }
"<"              { if (DEBUG_LEXER) std::cout << "LT" << std::endl; return LT; }
">"              { if (DEBUG_LEXER) std::cout << "GT" << std::endl; return GT; }
"<="             { if (DEBUG_LEXER) std::cout << "LEQ" << std::endl; return LEQ; }
">="             { if (DEBUG_LEXER) std::cout << "GEQ" << std::endl; return GEQ; }
"!="             { if (DEBUG_LEXER) std::cout << "NEQ" << std::endl; return NEQ; }
"&&"             { if (DEBUG_LEXER) std::cout << "AND" << std::endl; return AND; }
"+"              { if (DEBUG_LEXER) std::cout << "PLUS" << std::endl; return PLUS; }
"-"              { if (DEBUG_LEXER) std::cout << "MINUS" << std::endl; return MINUS; }
"*"              { if (DEBUG_LEXER) std::cout << "STAR" << std::endl; return STAR; }
"/"              { if (DEBUG_LEXER) std::cout << "SLASH" << std::endl; return SLASH; }
"%"              { if (DEBUG_LEXER) std::cout << "MODULO" << std::endl; return MODULO; }
"||"             { if (DEBUG_LEXER) std::cout << "OR" << std::endl; return OR; }
"!"              { if (DEBUG_LEXER) std::cout << "NOT" << std::endl; return NOT; }
";"              { if (DEBUG_LEXER) std::cout << "SEMICOLON" << std::endl; return SEMICOLON; }
":"              { if (DEBUG_LEXER) std::cout << "COLON" << std::endl; return COLON; }
","              { if (DEBUG_LEXER) std::cout << "COMMA" << std::endl; return COMMA; }
"->"             { if (DEBUG_LEXER) std::cout << "ARROW" << std::endl; return ARROW; }
"="              { if (DEBUG_LEXER) std::cout << "ASSIGN" << std::endl; return ASSIGN; }
"+="             { if (DEBUG_LEXER) std::cout << "PLUS_ASSIGN" << std::endl; return PLUS_ASSIGN; }
"-="             { if (DEBUG_LEXER) std::cout << "MINUS_ASSIGN" << std::endl; return MINUS_ASSIGN; }
"*="             { if (DEBUG_LEXER) std::cout << "STAR_ASSIGN" << std::endl; return STAR_ASSIGN; }
"/="             { if (DEBUG_LEXER) std::cout << "SLASH_ASSIGN" << std::endl; return SLASH_ASSIGN; }
"%="             { if (DEBUG_LEXER) std::cout << "MODULO_ASSIGN" << std::endl; return MODULO_ASSIGN; }
"print!"         { if (DEBUG_LEXER) std::cout << "PRINT" << std::endl; return PRINT; }
"if"             { if (DEBUG_LEXER) std::cout << "IF" << std::endl; return IF; }
"else"           { if (DEBUG_LEXER) std::cout << "ELSE" << std::endl; return ELSE; }
"while"          { if (DEBUG_LEXER) std::cout << "WHILE" << std::endl; return WHILE; }
"let"            { if (DEBUG_LEXER) std::cout << "LET" << std::endl; return LET; }
"mut"            { if (DEBUG_LEXER) std::cout << "MUT" << std::endl; return MUT; }
"fn"             { if (DEBUG_LEXER) std::cout << "FN" << std::endl; return FN; }
"return"         { if (DEBUG_LEXER) std::cout << "RETURN" << std::endl; return RETURN; }
"true"           { if (DEBUG_LEXER) std::cout << "TRUE" << std::endl; return TRUE; }
"false"          { if (DEBUG_LEXER) std::cout << "FALSE" << std::endl; return FALSE; }
"i32"            { if (DEBUG_LEXER) std::cout << "I32_TYPE" << std::endl; return I32_TYPE; }
"str"            { if (DEBUG_LEXER) std::cout << "STR_TYPE" << std::endl; return STR_TYPE; }
"bool"           { if (DEBUG_LEXER) std::cout << "BOOL_TYPE" << std::endl; return BOOL_TYPE; }
"()"             { if (DEBUG_LEXER) std::cout << "UNIT_TYPE" << std::endl; return UNIT_TYPE; }

\n               { /* ignore newlines */ }
([ \t])          { /* ignore whitespace */ }
"//".*           { /* ignore comments */ }
"/*"([^*]|"*"[^/])*"*/" { /* ignore comments */ }

[a-zA-Z_][a-zA-Z_0-9]* { 
    if (DEBUG_LEXER) std::cout << "IDENTIFIER: " << yytext << std::endl;
    yylval.str = strdup(yytext); 
    return IDENTIFIER;
}
\"([^\"\n]|\\[nt])*\"  { 
    if (DEBUG_LEXER) std::cout << "STRING: " << yytext << std::endl;
    yylval.str = strdup(yytext + 1);
    yylval.str[strlen(yylval.str) - 1] = '\0';
    return STRING;
}
.   { 
    if (DEBUG_LEXER) std::cout << "UNKNOWN: " << yytext << std::endl;
    return 0; 
}

%%

int yywrap() {
    return 1;
}
