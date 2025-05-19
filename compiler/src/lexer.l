%{
#include "../src/compiler.hpp"
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

void advance(const std::string& str, const std::string& more = "") {
    update_position();
    if (DEBUG_LEXER) std::cerr << str;
    if (more.size());
    std::cerr << more;
    std::cerr << std::endl;
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
    advance("NUMBER: ", yytext);
    yylval.num = atoi(yytext); 
    return NUMBER; 
}
"-"?[0-9]+       { 
    advance("NEGATIVE NUMBER: ", yytext);
    yylval.num = atoi(yytext); 
    return NUMBER; 
}
"("              { advance("OPEN_PARENTHESES"); return OPEN_PARENTHESES; }
")"              { advance("CLOSE_PARENTHESES"); return CLOSE_PARENTHESES; }
"{"              { advance("OPEN_BRACKET"); return OPEN_BRACKET; }
"}"              { advance("CLOSE_BRACKET"); return CLOSE_BRACKET; }
"["              { advance("OPEN_SUBSCRIPT"); return OPEN_SUBSCRIPT; }
"]"              { advance("CLOSE_SUBSCRIPT"); return CLOSE_SUBSCRIPT; }
"=="             { advance("EQ"); return EQ; }
"<"              { advance("LT"); return LT; }
">"              { advance("GT"); return GT; }
"<="             { advance("LEQ"); return LEQ; }
">="             { advance("GEQ"); return GEQ; }
"!="             { advance("NEQ"); return NEQ; }
"&&"             { advance("AND"); return AND; }
"+"              { advance("PLUS"); return PLUS; }
"-"              { advance("MINUS"); return MINUS; }
"*"              { advance("STAR"); return STAR; }
"/"              { advance("SLASH"); return SLASH; }
"%"              { advance("MODULO"); return MODULO; }
"||"             { advance("OR"); return OR; }
"!"              { advance("NOT"); return NOT; }
";"              { advance("SEMICOLON"); return SEMICOLON; }
":"              { advance("COLON"); return COLON; }
"="              { advance("ASSIGN"); return ASSIGN; }
"+="             { advance("PLUS_ASSIGN"); return PLUS_ASSIGN; }
"-="             { advance("MINUS_ASSIGN"); return MINUS_ASSIGN; }
"*="             { advance("STAR_ASSIGN"); return STAR_ASSIGN; }
"/="             { advance("SLASH_ASSIGN"); return SLASH_ASSIGN; }
"%="             { advance("MODULO_ASSIGN"); return MODULO_ASSIGN; }
"if"             { advance("IF"); return IF; }
"else"           { advance("ELSE"); return ELSE; }
"for"            { advance("FOR"); return FOR; }
"loop"           { advance("LOOP"); return LOOP; }
"break"          { advance("BREAK"); return BREAK; }
"continue"       { advance("CONTINUE"); return CONTINUE; }
"while"          { advance("WHILE"); return WHILE; }
"let"            { advance("LET"); return LET; }
"i32"            { advance("I32_TYPE"); return I32_TYPE; }
"str"            { advance("STR_TYPE"); return STR_TYPE; }

\n               { column_num = 1; line_num++; /* track new lines */ }
([ \t\r])          { column_num++; /* track whitespace */ }
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
    advance("IDENTIFIER: ", yytext);
    yylval.str = strdup(yytext); 
    return IDENTIFIER;
}

[a-zA-Z_][a-zA-Z_0-9]*! { 
    update_position();
    if (DEBUG_LEXER) advance("MACRO_IDENTIFIER: ", yytext);
    yylval.str = strdup(yytext); 
    return MACRO_IDENTIFIER;
}

\"([^\"\n]|\\[nt])*\"  { 
    advance("STRING: ", yytext);
    yylval.str = strdup(yytext + 1);
    yylval.str[strlen(yylval.str) - 1] = '\0';
    return STRING;
}
.   { 
    advance("UNKNOWN: ", yytext);
    syntaxError(std::string("unexpected character: ") + yytext);
    return 0; 
}

%%

int yywrap() {
    return 1;
}
