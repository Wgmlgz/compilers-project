%{
#include "../src/interpreter.hpp"
#include "../src/error.hpp"
#include "parser.tab.hpp"
#include <iostream>
#include <string>
#define DEBUG_LEXER 1

// Tracking line and column position
int line_num = 1;
int column_num = 1;
std::string current_file = "<stdin>";

// Update the position information
void update_position() {
    updateLocation(line_num, column_num, current_file);
    column_num += yyleng;
}

// Reset position for a new line
void new_line() {
    line_num++;
    column_num = 1;
}

// Set the current file being processed
void set_current_file(const char* filename) {
    current_file = filename;
    line_num = 1;
    column_num = 1;
}
%}

%%
           
[0-9]+           { 
    update_position();
    if (DEBUG_LEXER) std::cout << "NUMBER: " << yytext << std::endl;
    yylval.num = atoi(yytext); 
    return NUMBER; 
}
"-"?[0-9]+       { 
    update_position();
    if (DEBUG_LEXER) std::cout << "NEGATIVE NUMBER: " << yytext << std::endl;
    yylval.num = atoi(yytext); 
    return NUMBER; 
}
"("              { update_position(); if (DEBUG_LEXER) std::cout << "OPEN_PARENTHESES" << std::endl; return OPEN_PARENTHESES; }
")"              { update_position(); if (DEBUG_LEXER) std::cout << "CLOSE_PARENTHESES" << std::endl; return CLOSE_PARENTHESES; }
"{"              { update_position(); if (DEBUG_LEXER) std::cout << "OPEN_BRACKET" << std::endl; return OPEN_BRACKET; }
"}"              { update_position(); if (DEBUG_LEXER) std::cout << "CLOSE_BRACKET" << std::endl; return CLOSE_BRACKET; }
"=="             { update_position(); if (DEBUG_LEXER) std::cout << "EQ" << std::endl; return EQ; }
"<"              { update_position(); if (DEBUG_LEXER) std::cout << "LT" << std::endl; return LT; }
">"              { update_position(); if (DEBUG_LEXER) std::cout << "GT" << std::endl; return GT; }
"<="             { update_position(); if (DEBUG_LEXER) std::cout << "LEQ" << std::endl; return LEQ; }
">="             { update_position(); if (DEBUG_LEXER) std::cout << "GEQ" << std::endl; return GEQ; }
"!="             { update_position(); if (DEBUG_LEXER) std::cout << "NEQ" << std::endl; return NEQ; }
"&&"             { update_position(); if (DEBUG_LEXER) std::cout << "AND" << std::endl; return AND; }
"+"              { update_position(); if (DEBUG_LEXER) std::cout << "PLUS" << std::endl; return PLUS; }
"-"              { update_position(); if (DEBUG_LEXER) std::cout << "MINUS" << std::endl; return MINUS; }
"*"              { update_position(); if (DEBUG_LEXER) std::cout << "STAR" << std::endl; return STAR; }
"/"              { update_position(); if (DEBUG_LEXER) std::cout << "SLASH" << std::endl; return SLASH; }
"%"              { update_position(); if (DEBUG_LEXER) std::cout << "MODULO" << std::endl; return MODULO; }
"||"             { update_position(); if (DEBUG_LEXER) std::cout << "OR" << std::endl; return OR; }
"!"              { update_position(); if (DEBUG_LEXER) std::cout << "NOT" << std::endl; return NOT; }
";"              { update_position(); if (DEBUG_LEXER) std::cout << "SEMICOLON" << std::endl; return SEMICOLON; }
":"              { update_position(); if (DEBUG_LEXER) std::cout << "COLON" << std::endl; return COLON; }
","              { update_position(); if (DEBUG_LEXER) std::cout << "COMMA" << std::endl; return COMMA; }
"->"             { update_position(); if (DEBUG_LEXER) std::cout << "ARROW" << std::endl; return ARROW; }
"="              { update_position(); if (DEBUG_LEXER) std::cout << "ASSIGN" << std::endl; return ASSIGN; }
"+="             { update_position(); if (DEBUG_LEXER) std::cout << "PLUS_ASSIGN" << std::endl; return PLUS_ASSIGN; }
"-="             { update_position(); if (DEBUG_LEXER) std::cout << "MINUS_ASSIGN" << std::endl; return MINUS_ASSIGN; }
"*="             { update_position(); if (DEBUG_LEXER) std::cout << "STAR_ASSIGN" << std::endl; return STAR_ASSIGN; }
"/="             { update_position(); if (DEBUG_LEXER) std::cout << "SLASH_ASSIGN" << std::endl; return SLASH_ASSIGN; }
"%="             { update_position(); if (DEBUG_LEXER) std::cout << "MODULO_ASSIGN" << std::endl; return MODULO_ASSIGN; }
"print!"         { update_position(); if (DEBUG_LEXER) std::cout << "PRINT" << std::endl; return PRINT; }
"if"             { update_position(); if (DEBUG_LEXER) std::cout << "IF" << std::endl; return IF; }
"else"           { update_position(); if (DEBUG_LEXER) std::cout << "ELSE" << std::endl; return ELSE; }
"while"          { update_position(); if (DEBUG_LEXER) std::cout << "WHILE" << std::endl; return WHILE; }
"let"            { update_position(); if (DEBUG_LEXER) std::cout << "LET" << std::endl; return LET; }
"mut"            { update_position(); if (DEBUG_LEXER) std::cout << "MUT" << std::endl; return MUT; }
"fn"             { update_position(); if (DEBUG_LEXER) std::cout << "FN" << std::endl; return FN; }
"return"         { update_position(); if (DEBUG_LEXER) std::cout << "RETURN" << std::endl; return RETURN; }
"true"           { update_position(); if (DEBUG_LEXER) std::cout << "TRUE" << std::endl; return TRUE; }
"false"          { update_position(); if (DEBUG_LEXER) std::cout << "FALSE" << std::endl; return FALSE; }
"i32"            { update_position(); if (DEBUG_LEXER) std::cout << "I32_TYPE" << std::endl; return I32_TYPE; }
"str"            { update_position(); if (DEBUG_LEXER) std::cout << "STR_TYPE" << std::endl; return STR_TYPE; }
"bool"           { update_position(); if (DEBUG_LEXER) std::cout << "BOOL_TYPE" << std::endl; return BOOL_TYPE; }
"()"             { update_position(); if (DEBUG_LEXER) std::cout << "UNIT_TYPE" << std::endl; return UNIT_TYPE; }

\n               { column_num = 1; line_num++; /* track new lines */ }
([ \t])          { column_num++; /* track whitespace */ }
"//".*           { 
    update_position(); 
    /* ignore line comments */ 
}
"/*"([^*]|"*"[^/])*"*/" { 
    // For multi-line comments, count the newlines
    for(int i = 0; i < yyleng; i++) {
        if(yytext[i] == '\n') {
            new_line();
        } else {
            column_num++;
        }
    }
    /* ignore block comments */ 
}

[a-zA-Z_][a-zA-Z_0-9]* { 
    update_position();
    if (DEBUG_LEXER) std::cout << "IDENTIFIER: " << yytext << std::endl;
    yylval.str = strdup(yytext); 
    return IDENTIFIER;
}
\"([^\"\n]|\\[nt])*\"  { 
    update_position();
    if (DEBUG_LEXER) std::cout << "STRING: " << yytext << std::endl;
    yylval.str = strdup(yytext + 1);
    yylval.str[strlen(yylval.str) - 1] = '\0';
    return STRING;
}
.   { 
    update_position();
    if (DEBUG_LEXER) std::cout << "UNKNOWN: " << yytext << std::endl;
    syntaxError(std::string("unexpected character: ") + yytext);
    return 0; 
}

%%

int yywrap() {
    return 1;
}
