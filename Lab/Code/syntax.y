%locations

%{
    #include <stdio.h>
    #include "lex.yy.c"
    extern int isError, isPrint;
    void PrintTree(struct Tree* t, int num);
    void SetRoot(struct Tree* t);
%}
%union{
    struct Tree* type_Tree;
}

%token <type_Tree> INT FLOAT ID RELOP SEMI COMMA ASSIGNOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE 

%type <type_Tree> Program ExtDefList ExtDef Specifier FunDec CompSt VarDec StructSpecifier OptTag Tag ParamDec VarList StmtList Stmt Dec DecList Def DefList ExtDecList Args Exp

%nonassoc LOWER_THAN_ALL

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEG
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDefList    { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Program"); root = $$;if(isPrint) PrintTree(root,0); }
    ;
ExtDefList : ExtDef ExtDefList  { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ExtDefList"); $1->next = $2; }
    | /* empty */               { $$ = newTreeNode(NULL, NULL, 0, 6, "ExtDefList"); }
    | error ExtDefList          { ; }
    ;
ExtDef : Specifier ExtDecList SEMI { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ExtDef"); $1->next = $2; $2->next = $3; }
    | Specifier SEMI               { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ExtDef"); $1->next = $2; }
    | Specifier FunDec CompSt      { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ExtDef"); $1->next = $2; $2->next = $3; }
    | error SEMI                   { ; }
    ;
ExtDecList : VarDec             { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ExtDecList"); }
    | VarDec COMMA ExtDecList   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ExtDecList"); $1->next = $2; $2->next = $3; }
    ;

Specifier : TYPE        { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Specifier"); }
    | StructSpecifier   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Specifier"); }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "StructSpecifier"); $1->next = $2; $2->next = $3; $3->next = $4; $4->next = $5; }
    | STRUCT Tag                                { $$ = newTreeNode(NULL, $1, $1->pos, 0, "StructSpecifier"); $1->next = $2; }
    | STRUCT error LC DefList RC    { ; }
    | STRUCT OptTag LC error RC     { ; }
    ;
OptTag : ID         { $$ = newTreeNode(NULL, $1, $1->pos, 0, "OptTag"); }
    | /* empty */   { $$ = newTreeNode(NULL, NULL, 0, 6, "OptTag"); }
    ;
Tag : ID    { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Tag"); }
    ;

VarDec : ID             { $$ = newTreeNode(NULL, $1, $1->pos, 0, "VarDec"); }
    | VarDec LB INT RB  { $$ = newTreeNode(NULL, $1, $1->pos, 0, "VarDec"); $1->next = $2; $2->next = $3; $3->next = $4; }
    ;
FunDec : ID LP VarList RP   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "FunDec"); $1->next = $2; $2->next = $3; $3->next = $4; }
    | ID LP RP              { $$ = newTreeNode(NULL, $1, $1->pos, 0, "FunDec"); $1->next = $2; $2->next = $3; }
    | ID LP error RP        { ; }
    | error LP RP           { ; }
    | error LP VarList RP   { ; }
    ;
VarList : ParamDec COMMA VarList    { $$ = newTreeNode(NULL, $1, $1->pos, 0, "VarList"); $1->next = $2; $2->next = $3; }
    | ParamDec                      { $$ = newTreeNode(NULL, $1, $1->pos, 0, "VarList"); }
    ;
ParamDec : Specifier VarDec     { $$ = newTreeNode(NULL, $1, $1->pos, 0, "ParamDec"); $1->next = $2; }
    ;

CompSt : LC DefList StmtList RC     { $$ = newTreeNode(NULL, $1, $1->pos, 0, "CompSt"); $1->next = $2; $2->next = $3; $3->next = $4; }
    | LC error RC          { ; }
    ;
StmtList : Stmt StmtList    { $$ = newTreeNode(NULL, $1, $1->pos, 0, "StmtList"); $1->next = $2; }
    | /* empty */           { $$ = newTreeNode(NULL, NULL, 0, 6, "StmtList"); }
    | error StmtList        { ; }
    ;
Stmt : Exp SEMI                                 { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Stmt"); $1->next = $2; }
    | CompSt                                    { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Stmt"); }
    | RETURN Exp SEMI                           { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Stmt"); $1->next = $2; $2->next = $3; }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Stmt"); $1->next = $2; $2->next = $3; $3->next = $4; $4->next = $5; }
    | IF LP Exp RP Stmt ELSE Stmt               { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Stmt"); $1->next = $2; $2->next = $3; $3->next = $4; $4->next = $5; $5->next = $6; $6->next = $7; }
    | WHILE LP Exp RP Stmt                      { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Stmt"); $1->next = $2; $2->next = $3; $3->next = $4; $4->next = $5; }
    | WHILE LP error RP Stmt                    { ; }
    | WHILE LP error Stmt                       { ; }
    | WHILE error RP Stmt                       { ; }
    ;

DefList : Def DefList   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "DefList"); $1->next = $2; }
    | /* empty */       { $$ = newTreeNode(NULL, NULL, 0, 6, "DefList"); }
    | error DefList     { ; }
    ;
Def : Specifier DecList SEMI    { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Def"); $1->next = $2; $2->next = $3; }
    ;
DecList : Dec           { $$ = newTreeNode(NULL, $1, $1->pos, 0, "DecList"); }
    | Dec COMMA DecList { $$ = newTreeNode(NULL, $1, $1->pos, 0, "DecList"); $1->next = $2; $2->next = $3; }
    ;
Dec : VarDec                { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Dec"); }
    | VarDec ASSIGNOP Exp   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Dec"); $1->next = $2; $2->next = $3; }
    ;

Exp : Exp ASSIGNOP Exp  { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp AND Exp       { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp OR Exp        { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp RELOP Exp     { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp PLUS Exp      { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp MINUS Exp     { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp STAR Exp      { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp DIV Exp       { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | LP Exp RP         { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | MINUS Exp %prec NEG   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; }
    | NOT Exp           { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; }
    | ID LP Args RP     { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; $3->next = $4; }
    | ID LP RP          { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | Exp LB Exp RB     { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; $3->next = $4; }
    | Exp DOT ID        { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); $1->next = $2; $2->next = $3; }
    | ID                { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); }
    | INT               { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); }
    | FLOAT             { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Exp"); }

    | error ASSIGNOP Exp { ; }
    | error AND Exp     { ; }
    | error OR Exp      { ; }
    | error RELOP Exp   { ; }
    | error PLUS Exp    { ; }
    | error MINUS Exp   { ; }
    | error STAR Exp    { ; }
    | error DIV Exp     { ; }
    | error DOT Exp     { ; }
    | LP error RP       { ; }
    | Exp LB error RB   { ; }
    | error %prec LOWER_THAN_ALL { ; }
    ;
Args : Exp COMMA Args   { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Args"); $1->next = $2; $2->next = $3; }
    | Exp               { $$ = newTreeNode(NULL, $1, $1->pos, 0, "Args"); }
    ;

%%
yyerror(char* msg){
    Print_B_Error();
}

void PrintTree(struct Tree* t, int num){
    
    if(t==NULL) return ;
    int cpy_num = num;
    if(t->type!=6)
        while(cpy_num--) printf(" "); 
    switch(t->type){
        case 0: printf("%s (%d)\n",t->V.v_string,t->pos); break;
        case 1: printf("INT: %d\n",t->V.v_int); break;
        case 2: printf("FLOAT: %f\n",t->V.v_float); break;
        case 3: printf("%s\n",t->V.v_string); break;
        case 4: printf("ID: %s\n",t->V.v_string); break;
        case 5: printf("TYPE: %s\n",t->V.v_string); break;
        case 6: break;
        default: break;
    };
    PrintTree(t->children,num+2);
    PrintTree(t->next,num);
}

void Print_B_Error(){
    if(!isError) printf("Error type B at Line %d: %s\n", yylineno, yytext);
    isError = 1; isPrint = 0;
}