%option caseless

%{
#include "Tree.h"
#include "parser.tab.hpp"
#include <cstdio>
#include <cstring>

char *get_text(const char *s)
{
    int length = strlen(s);
    char *buffer = new char[length + 1];
    memcpy(buffer, s, (length + 1) * sizeof(char));
    return buffer;
}

void yyerror(const char *);

%}



WHITESPACE      [ \t\n]
IDENTIFIER      [A-Za-z][_0-9A-Za-z]*
VALUE_INT       [0-9]+
VALUE_STRING    '[^']*'
VALUE_FLOAT     [0-9]+\.[0-9]*
VALUE_DATE      '[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}'


%x SQL_COMMENT

%%


QUIT|quit              { /*printf("QUIT");*/return QUIT;       }
CREATE|create          { /*printf("CREATE "); */ return CREATE; }
DROP|drop              { /*printf("DROP "); */ return DROP; }
USE|use                { /*printf("USE "); */ return USE; }
SHOW|show              { /*printf("SHOW "); */ return SHOW; }
DATABASES|databases    { /*printf("DATABASES "); */ return DATABASES; }
DATABASE|database      { /*printf("DATABASE "); */ return DATABASE; }
TABLE|table            { /*printf("TABLE "); */ return TABLE; }
TABLES|tables          { /*printf("TABLES "); */ return TABLES; }
SELECT|select          { /*printf("SELECT "); */ return SELECT; }
DELETE|delete          { /*printf("DELETE "); */ return DELETE; }
INSERT|insert          { /*printf("INSERT "); */ return INSERT; }
UPDATE|update          { /*printf("UPDATE "); */ return UPDATE; }
VALUES|values          { /*printf("VALUES "); */ return VALUES; }
REFERENCES|references  { /*printf("REFERENCES "); */ return REFERENCES; }
SET|set                { /*printf("SET "); */ return SET; }
FROM|from              { /*printf("FROM "); */ return FROM; }
INTO|into              { /*printf("INTO "); */ return INTO; }
WHERE|where            { /*printf("WHERE "); */ return WHERE; }
"NOT NULL"             { /*printf("NOT_NULL "); */ return NOTNULL; }
"PRIMARY KEY"          { /*printf("PRIMARY_KEY "); */ return PRIMARY; }
"FOREIGN KEY"          { /*printf("FOREIGN_KEY "); */ return FOREIGN; }
DESC|desc              { /*printf("DESC "); */ return DESC; }
INDEX|index            { /*printf("INDEX "); */ return INDEX; }
IN|in                  { /*printf("IN "); */ return IN; }
IS|is                  { /*printf("IS "); */ return IS; }
NULL|null              { /*printf("NULL "); */ return T_NULL; }

INT|int         { /*printf("int "); */ return INT; }
FLOAT|float     { /*printf("float "); */ return FLOAT; }
CHAR|char       { /*printf("char "); */ return CHAR; }
VARCHAR|varchar { /*printf("varchar "); */ return VARCHAR; }
DATE|date       { /*printf("date "); */ return DATE;  }
{VALUE_FLOAT}   { /*printf("FLOAT "); */ sscanf(yytext, "%f", &yylval.fvalue); return VALUE_FLOAT; }
{VALUE_INT}     { /*printf("INTEGER "); */ sscanf(yytext, "%d", &yylval.ivalue); return VALUE_INT; }
{VALUE_DATE}    { yylval.string = get_text(yytext, true); return VALUE_DATE; }
{VALUE_STRING}  { yylval.string = get_text(yytext, false); /*printf("STRING(%s) ", yylval.string);*/ return VALUE_STRING; }
{IDENTIFIER}    { yylval.string = get_text(yytext, false); /*printf("IDENTIFIER(%s) ", yylval.string);*/ return IDENTIFIER; }

AND|and         { /*printf("AND "); */ return AND; }
OR|or           { /*printf("OR ");  */ return OR;  }
"="             { /*printf("= ");*/ return EQ; }
">"             { /*printf("> ");*/ return GT; }
"<"             { /*printf("< ");*/ return LT; }
">="            { /*printf(">= ");*/ return GE; }
"<="            { /*printf("<= ");*/ return LE; }
"<>"            { /*printf("<> ");*/ return NE; }
{WHITESPACE}    {}
.               { yyerror((std::string("invalid char ") + yytext).c_str()); }
","             { /*printf(", "); */ return yytext[0]; }
"("             { /*printf("( "); */ return yytext[0]; }
")"             { /*printf(") "); */ return yytext[0]; }
"."             { /*printf(". "); */ return yytext[0]; }
";"             { /*printf("; "); */ return yytext[0]; }
"+"             { /*printf("+ "); */ return yytext[0]; }
"-"             { /*printf("- "); */ return yytext[0]; }
"/"             { /*printf("/ "); */ return yytext[0]; }
"*"             { /*printf("* "); */ return yytext[0]; }

%%

int yywrap() {
    return 1;
}