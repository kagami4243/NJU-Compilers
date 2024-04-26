
%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "ast.h"
    #include "syntax.tab.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
    void Print_A_Error();
    int isErrorAB = 0, isPrintAB = 1;
%}
%option yylineno


INT         [0-9]+
FLOAT       {INT}\.{INT}
ID          [a-zA-Z_][a-zA-Z0-9_]*
SEMI        ";"
COMMA       ","
ASSIGNOP    "="
RELOP       ">"|"<"|">="|"<="|"=="|"!="
PLUS        "+"
MINUS       "-"
STAR        "*"
DIV         "/"
AND         "&&"
OR          "||"
DOT         "."
NOT         "!"
TYPE        "int"|"float"
LP          "("
RP          ")"
LB          "["
RB          "]"
LC          "{"
RC          "}"
STRUCT      "struct"
RETURN      "return"
IF          "if"
ELSE        "else"
WHILE       "while"

%%
[ \t\r]+    { ; }
\n          { yycolumn = 1; isErrorAB=0; }
"//".*      { ; }
"/*"        {   
                int c1 = 0, c2 = input();
                for(;;) {
                    if(c2 == EOF || c2 == 0){
                        Print_B_Error();
                        break;
                    }
                    if(c1 == '*' && c2 == '/')
                        break;
                    c1 = c2;
                    c2 = input();
                }
            }

{TYPE}      { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,5,yytext); return TYPE; }
{STRUCT}    { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"STRUCT"); return STRUCT; }
{RETURN}    { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"RETURN"); return RETURN; }
{IF}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"IF"); return IF; }
{ELSE}      { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"ELSE"); return ELSE; }
{WHILE}     { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"WHILE"); return WHILE; }
{INT}       { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,1,yytext); return INT; }
{FLOAT}     { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,2,yytext); return FLOAT; }
{ID}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,4,yytext); return ID; }
{SEMI}      { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"SEMI"); return SEMI; }
{COMMA}     { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"COMMA"); return COMMA; }
{ASSIGNOP}  { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"ASSIGNOP"); return ASSIGNOP; }
{RELOP}     { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,7,yytext); return RELOP; }
{PLUS}      { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"PLUS"); return PLUS; }
{MINUS}     { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"MINUS"); return MINUS; }
{STAR}      { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"STAR"); return STAR; }
{DIV}       { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"DIV"); return DIV; }
{AND}       { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"AND"); return AND; }
{OR}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"OR"); return OR; }
{DOT}       { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"DOT"); return DOT; }
{NOT}       { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"NOT"); return NOT; }
{LP}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"LP"); return LP; }
{RP}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"RP"); return RP; }
{LB}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"LB"); return LB; }
{RB}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"RB"); return RB; }
{LC}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"LC"); return LC; }
{RC}        { yylval.type_Tree=newTreeNode(NULL,NULL,yylineno,3,"RC"); return RC; } 
.           { Print_A_Error(); }

%%
struct Tree* newTreeNode(struct Tree* next, struct Tree* children, int pos, int type, char* value){
    struct Tree* t=(struct Tree*)malloc(sizeof(struct Tree));
    t->next=next; t->children=children;
    t->pos=pos;
    t->type=type;
    t->leftValue=0;
    switch(type){
        case 0: case 3: case 4: case 5: case 6: case 7:
                t->V.v_string=strdup(value); break;
        case 1: t->V.v_int=atoi(value); break;
        case 2: t->V.v_float=atof(value); break;
        default: break;
    }
    return t;
}  

void Print_A_Error(){
    set_error();
    if(!isErrorAB) 
        printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); 
    isErrorAB = 1; isPrintAB = 0;
}