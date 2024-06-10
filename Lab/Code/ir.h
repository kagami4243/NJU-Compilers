#ifndef IR_H
#define IR_H

typedef struct args* arg_list;
struct args{
    char name[40];
    struct args* next;
};

void translate_Program();

void translate_ExtDefList(struct Tree* ExtDefList);

void translate_ExtDef(struct Tree* ExtDef);

void translate_ExtDecList(struct Tree* extdeclist,char* specifier);

void translate_VarDec(struct Tree* vardec,char* specifier);

void translate_FunDec(struct Tree* fundec);

void translate_VarList(struct Tree* varlist);

void translate_ParamDec(struct Tree* paramdec);

void translate_CompSt(struct Tree* compst);

void translate_StmtList(struct Tree* stmtlist);

void translate_Stmt(struct Tree* stmt);

void translate_DefList(struct Tree* deflist);

void translate_Def(struct Tree* def);

void translate_DecList(struct Tree* declist, char* specifier);

void translate_Dec(struct Tree* dec, char* specifier);

void translate_Exp(struct Tree* exp, char* place);

void translate_Cond(struct Tree* exp, char* label_true, char* label_false);

void translate_Args(struct Tree* args,arg_list al);

char* newTemp();

char* newLabel();

int findOffset(Type t,char* name);

#endif