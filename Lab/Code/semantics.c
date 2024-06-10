#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantics.h"

struct symbol* symbolTable;
struct TypeTable* structTable;
int StructTableUni = 0;

static int error_pos=-1; // avoid duplicate error messages

void PrintErrorNum(int num, int pos){
    set_error();
    if(error_pos!=pos){
        printf("Error type %d at Line %d: \n",num, pos);
        error_pos=pos;
    }
}

Type genBasicType(char* basic){
    Type t=(Type)malloc(sizeof(struct Type_));
    t->kind=BASIC;
    strcpy(t->u.basic,basic);
    return t;
}

void initSymbol(){
    symbolTable=(struct symbol*)malloc(sizeof(struct symbol));
    symbolTable->next=NULL;
}

void initStruct(){
    structTable=(struct TypeTable*)malloc(sizeof(struct TypeTable));
    structTable->next=NULL;
}

void addVar2Symbol(char*name, int type,char* type_name){
    struct symbol* s=(struct symbol*)malloc(sizeof(struct symbol));
    s->next=symbolTable->next;
    symbolTable->next=s;
    s->type=type;
    strcpy(s->name,name);
    Type var=(Type)malloc(sizeof(struct Type_));
    var->kind=BASIC;
    strcpy(var->u.basic,type_name);
    s->val.var=var;
    s->param=0;
    return;
}

char* addArray2Symbol(struct Tree*node,int type,char*type_name){
    Type head,tail;
    head=tail=NULL;
    while(node->children->next!=NULL){
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=NULL;
        t->u.array.size=node->children->next->next->V.v_int;
        if(head==NULL) head=tail=t;
        else{
            tail->u.array.elem=t;
            tail=t;
        }
        node=node->children;
    }
    {
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=BASIC;
        strcpy(t->u.basic,type_name);
        tail->u.array.elem=t;
        tail=t;
    }
    struct symbol* s=(struct symbol*)malloc(sizeof(struct symbol));
    s->next=symbolTable->next;
    symbolTable->next=s;
    s->type=type;
    strcpy(s->name,node->children->V.v_string);
    s->val.var=head;
    char* name=(char*)malloc(sizeof(char)*40);
    strcpy(name,s->name);
    s->param=0;
    return name;
}

Type genArrayType(struct Tree*node,char*type_name){
    Type head,tail;
    head=tail=NULL;
    while(node->children->next!=NULL){
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=NULL;
        t->u.array.size=node->children->next->next->V.v_int;
        if(head==NULL) head=tail=t;
        else{
            tail->u.array.elem=t;
            tail=t;
        }
        node=node->children;
    }
    {
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=BASIC;
        strcpy(t->u.basic,type_name);
        tail->u.array.elem=t;
        tail=t;
    }
    return head;
}

void addFunc2Symbol(char*name, int type,Type return_type,int param_num,Type* param_type){
    struct symbol* s=(struct symbol*)malloc(sizeof(struct symbol));
    s->next=symbolTable->next;
    symbolTable->next=s;
    s->type=type;
    strcpy(s->name,name);
    struct Func* func=(struct Func*)malloc(sizeof(struct Func));
    func->return_type=return_type;
    func->param_num=param_num;
    func->param_type=(Type*)malloc(sizeof(Type)*param_num);
    for(int i=0;i<param_num;++i) func->param_type[i]=param_type[i];
    s->val.func=func;
    return;
}

struct symbol* findSymbolWithName(char* name, int type){
    struct symbol* cur=symbolTable->next;
    while(cur!=NULL){
        if(strcmp(cur->name,name)==0){
            if(cur->type==type)
                break;
        }
        cur=cur->next;
    }
    return cur;
}

int Compare2Type(Type t1,Type t2){
    if(t1==NULL || t2==NULL) return 0;
    if(t1->kind!=t2->kind) return 0;
    if(t1->kind==BASIC){
        if(strcmp(t1->u.basic,"int")==0 || strcmp(t1->u.basic,"float")==0
            || strcmp(t2->u.basic,"int")==0 || strcmp(t2->u.basic,"float")==0){
            if(strcmp(t1->u.basic,t2->u.basic)!=0){
                return 0;
            } 
        }
        else if(strcmp(t1->u.basic,t2->u.basic)!=0){
            struct TypeTable*tt1=findStructWithName(t1->u.basic),*tt2=findStructWithName(t2->u.basic);
            FieldList f1=tt1->t,f2=tt2->t;
            while(f1!=NULL && f2!=NULL){
                if(!Compare2Type(f1->type,f2->type))
                    return 0;
                f1=f1->tail,f2=f2->tail;
            }
            if((f1!=NULL && f2==NULL) || (f1==NULL && f2!=NULL)) return 0;
        }
    }   
    else if(t1->kind==ARRAY){
        t1=t1->u.array.elem;
        t2=t2->u.array.elem;
        return Compare2Type(t1,t2);
    }
    return 1;
}

char* add2StructTable(struct Tree*structspecifier){
    // StructSpecifier->STRUCT OptTag LC DefList RC
    struct Tree*tag=structspecifier->children->next;
    char* Tag;
    Tag=(char*)malloc(sizeof(char)*40);
    if(tag->type!=6){
        strcpy(Tag,tag->children->V.v_string);
    }
    else{
        sprintf(Tag,"%d",StructTableUni++);
    }
    if((findSymbolWithName(Tag,0)!=NULL) || (findStructWithName(Tag)!=NULL)){
        PrintErrorNum(16,tag->pos);
        return Tag;
    }
    int size=0;
    struct Tree* deflist=tag->next->next;
    FieldList t=(FieldList)malloc(sizeof(struct FieldList_));
    t->tail=NULL;
    FieldList tail=t;
    while(deflist->type!=6){
        struct Tree* def=deflist->children, *specifier=def->children, *declist=specifier->next;
        do_semantics(specifier);
        while(declist->children->next!=NULL){
            struct Tree*vardec=declist->children->children;
            FieldList field=(FieldList)malloc(sizeof(struct FieldList_));
            field->tail=NULL;
            Type unit=(Type)malloc(sizeof(struct Type_));
            char name[40];
            if(vardec->children->next==NULL){
                unit->kind=BASIC;
                strcpy(unit->u.basic,specifier->specifier);
                strcpy(name,vardec->children->V.v_string);
                if(strcmp(specifier->specifier,"int")==0 || strcmp(specifier->specifier,"float")==0){
                    size+=4;
                }
                else{
                    size+=findStructWithName(specifier->specifier)->size;
                }
                if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL
                    || findStructWithName(vardec->children->V.v_string)!=NULL){
                    PrintErrorNum(15,vardec->children->pos);
                }
                else 
                    addVar2Symbol(vardec->children->V.v_string,0,specifier->specifier);
            }
            else{
                unit->kind=ARRAY;
                unit->u.array=genArrayType(vardec,specifier->specifier)->u.array;
                size+=calculateVarSize(unit);
                struct Tree* v=vardec;
                while(v->children->next!=NULL) v=v->children;
                if(findSymbolWithName(v->children->V.v_string,0)!=NULL
                    || findStructWithName(v->children->V.v_string)!=NULL){
                    PrintErrorNum(15,v->children->pos);
                }
                else 
                    strcpy(name,addArray2Symbol(vardec,0,specifier->specifier));
            }
            field->type=unit;
            strcpy(field->name,name);
            tail->tail=field;
            tail=field;
            vardec->exp_type=unit;
            if(vardec->next!=NULL){
                PrintErrorNum(15,vardec->pos);
            }
            declist=declist->children->next->next;
        }
        {   struct Tree*vardec=declist->children->children;
            FieldList field=(FieldList)malloc(sizeof(struct FieldList_));
            field->tail=NULL;
            Type unit=(Type)malloc(sizeof(struct Type_));
            char name[40];
            if(vardec->children->next==NULL){
                unit->kind=BASIC;
                strcpy(unit->u.basic,specifier->specifier);
                strcpy(name,vardec->children->V.v_string);
                if(strcmp(specifier->specifier,"int")==0 || strcmp(specifier->specifier,"float")==0){
                    size+=4;
                }
                else{
                    size+=findStructWithName(specifier->specifier)->size;
                }
                if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL
                    || findStructWithName(vardec->children->V.v_string)!=NULL){
                    PrintErrorNum(15,vardec->children->pos);
                }
                else 
                    addVar2Symbol(vardec->children->V.v_string,0,specifier->specifier);
            }
            else{
                unit->kind=ARRAY;
                unit->u.array=genArrayType(vardec,specifier->specifier)->u.array;
                size+=calculateVarSize(unit);
                struct Tree* v=vardec;
                while(v->children->next!=NULL) v=v->children;
                if(findSymbolWithName(v->children->V.v_string,0)!=NULL
                    || findStructWithName(v->children->V.v_string)!=NULL){
                    PrintErrorNum(15,v->children->pos);
                }
                else 
                    strcpy(name,addArray2Symbol(vardec,0,specifier->specifier));
            }
            field->type=unit;
            strcpy(field->name,name);
            tail->tail=field;
            tail=field;
            vardec->exp_type=unit;
            if(vardec->next!=NULL){
                PrintErrorNum(15,vardec->pos);
            }
        }
        deflist=def->next;
    }
    struct TypeTable* tt=(struct TypeTable*)malloc(sizeof(struct TypeTable));
    strcpy(tt->name,Tag);
    tt->t=t->tail;
    tt->next=structTable->next;
    tt->size=size;
    structTable->next=tt;
    return Tag;
}

struct TypeTable* findStructWithName(char*name){
    struct TypeTable* cur=structTable->next;
    while(cur!=NULL){
        if(strcmp(cur->name,name)==0){
            break;
        }
        cur=cur->next;
    }
    return cur;
}

void do_semantics(struct Tree* node){
    if(node==NULL || node->type==6) return;
    // printf("%d\n",node->type);
    // if(node->type==0 || node->type==3 || node->type==5) printf("%s\n",node->V.v_string);
    // if(node->type==1) printf("INT: %d\n",node->V.v_int);
    // if(node->type==2) printf("FLOAT: %f\n",node->V.v_float);
    // if(node->type==4) printf("ID: %s\n",node->V.v_string);
    switch (node->type)
    {
    case 0:{
        if(strcmp(node->V.v_string,"Exp")==0){
            if(node->children->next==NULL){
                switch(node->children->type){
                    case 1:{
                        // Exp->INT
                        node->exp_type=genBasicType("int"); 
                        break;
                    }
                    case 2:{
                        // Exp->FLOAT
                        node->exp_type=genBasicType("float"); 
                        break;
                    }
                    case 4:{
                        // Exp->ID
                        struct Tree* id=node->children;
                        struct symbol* s=findSymbolWithName(id->V.v_string,0);
                        if(s==NULL){
                            PrintErrorNum(1,node->pos);
                            node->exp_type=genBasicType("int"); 
                            break;
                        } 
                        node->exp_type=s->val.var;
                        break;
                    }
                    default: break;
                }
            }
            else if(node->children->next->next==NULL){
                struct Tree* exp1=node->children->next;
                if(strcmp(node->children->V.v_string,"MINUS")==0){
                    // Exp->MINUS Exp
                    do_semantics(exp1);
                    if(exp1->exp_type->kind!=BASIC ||
                     (strcmp(exp1->exp_type->u.basic,"int")!=0 && strcmp(exp1->exp_type->u.basic,"float")!=0)){
                        PrintErrorNum(1,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    node->exp_type=exp1->exp_type;
                }
                else{
                    // Exp->NOT Exp
                    do_semantics(exp1);
                    if(exp1->exp_type->kind!=BASIC || strcmp(exp1->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(1,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    node->exp_type=exp1->exp_type;
                }
            }
            else if(node->children->next->next->next==NULL){
                char* op=node->children->next->V.v_string;
                if(strcmp(op,"AND")==0 || strcmp(op,"OR")==0){
                    // Exp->Exp AND Exp
                    // Exp->Exp OR Exp
                    struct Tree* exp1=node->children;
                    struct Tree* exp2=node->children->next->next;
                    do_semantics(exp1);
                    do_semantics(exp2);
                    node->exp_type=genBasicType("int");
                    if(exp1->exp_type->kind!=BASIC || exp2->exp_type->kind!=BASIC ||
                    strcmp(exp1->exp_type->u.basic,"int")!=0 || strcmp(exp2->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,node->pos);
                        break;
                    }
                }
                else if(strcmp(op,"ASSIGNOP")==0){
                    // Exp->Exp ASSIGNOP Exp
                    struct Tree* exp1=node->children;
                    struct Tree* exp2=node->children->next->next;
                    do_semantics(exp1);
                    do_semantics(exp2);
                    node->exp_type=exp1->exp_type;
                    if(!Compare2Type(exp1->exp_type,exp2->exp_type)){
                        PrintErrorNum(5,node->pos);
                        break;
                    }
                    if(exp1->leftValue==0){
                        PrintErrorNum(6,node->pos);
                        break;
                    }
                }
                else if(strcmp(op,"PLUS")==0 || strcmp(op,"MINUS")==0 || node->children->next->type==7
                        || strcmp(op,"STAR")==0 || strcmp(op,"DIV")==0){
                    // Exp->Exp RELOP Exp
                    // Exp->Exp PLUS Exp
                    // Exp->Exp MINUS Exp
                    // Exp->Exp STAR Exp
                    // Exp->Exp DIV Exp
                    struct Tree* exp1=node->children;
                    struct Tree* exp2=node->children->next->next;
                    do_semantics(exp1);
                    do_semantics(exp2);
                    if(exp1->exp_type->kind!=BASIC || exp2->exp_type->kind!=BASIC
                        || strcmp(exp1->exp_type->u.basic,exp2->exp_type->u.basic)!=0){
                        PrintErrorNum(7,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    if(strcmp(exp1->exp_type->u.basic,"int")!=0 && strcmp(exp1->exp_type->u.basic,"float")!=0){
                        PrintErrorNum(7,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    if(node->children->next->type==7){
                        node->exp_type=genBasicType("int");
                    }
                    else 
                        node->exp_type=exp1->exp_type;
                }
                else if(strcmp(op,"Exp")==0){
                    // Exp->LP Exp RP
                    struct Tree* exp1=node->children->next;
                    do_semantics(exp1);
                    node->exp_type=exp1->exp_type;
                }
                else if(strcmp(op,"LP")==0){
                    // Exp->ID LP RP
                    struct Tree* id=node->children;
                    struct symbol* s=findSymbolWithName(id->V.v_string,1);
                    if(findSymbolWithName(id->V.v_string,0)!=NULL){
                        PrintErrorNum(11,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    if(s==NULL){
                        PrintErrorNum(7,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    if(s->val.func->param_num!=0){
                        PrintErrorNum(9,node->pos);
                    }
                    node->exp_type=s->val.func->return_type;
                }
                else{
                    // Exp->Exp DOT ID
                    struct Tree* exp1=node->children,*id=node->children->next->next;
                    do_semantics(exp1);
                    struct TypeTable* s=findStructWithName(exp1->exp_type->u.basic);
                    if(s==NULL){
                        PrintErrorNum(13,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    FieldList f=s->t;
                    while(f!=NULL){
                        if(strcmp(f->name,id->V.v_string)==0)
                            break;
                        f=f->tail;
                    }
                    if(f==NULL){
                        PrintErrorNum(14,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    node->exp_type=f->type;
                }
            }
            else if(node->children->next->next->next->next==NULL){
                if(strcmp(node->children->next->V.v_string,"LP")==0){
                    // Exp->ID LP Args RP
                    struct Tree* id=node->children,*args=node->children->next->next;
                    struct symbol*s=findSymbolWithName(id->V.v_string,1);
                    if(findSymbolWithName(id->V.v_string,0)!=NULL){
                        PrintErrorNum(11,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    if(s==NULL){
                        PrintErrorNum(2,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    node->exp_type=s->val.func->return_type;
                    int index=0;
                    int error=0;
                    while(args->children->next!=NULL){
                        // Args->Exp COMMA Args
                        struct Tree* exp=args->children;
                        do_semantics(exp);
                        if(!Compare2Type(exp->exp_type,s->val.func->param_type[index])){
                            PrintErrorNum(9,node->pos);
                            error=1;
                            break;
                        }
                        args=args->children->next->next;
                        ++index;
                    }
                    // Args->Exp
                    if(error!=1){
                        struct Tree* exp=args->children;
                        do_semantics(exp);
                        if(!Compare2Type(exp->exp_type,s->val.func->param_type[index])
                            || index!=s->val.func->param_num-1){
                            PrintErrorNum(9,node->pos);
                            break;
                        }
                    }
                }
                else{
                    // Exp->Exp LB Exp RB
                    struct Tree* exp1=node->children,*exp2=node->children->next->next;
                    do_semantics(exp1);
                    do_semantics(exp2);
                    if(exp1->exp_type->kind!=ARRAY){
                        PrintErrorNum(10,node->pos);
                        node->exp_type=genBasicType("int");
                        break;
                    }
                    node->exp_type=exp1->exp_type->u.array.elem;
                    if(exp2->exp_type->kind!=BASIC || 
                        strcmp(exp2->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(12,node->pos);
                        break;
                    }
                }
            }
        }
        else if(strcmp(node->V.v_string,"ExtDefList")==0){
            // ExtDefList->ExtDef ExtDefList
            // ExtDefList->empty
            struct Tree* extdeflist=node;
            while(extdeflist->type!=6){
                do_semantics(extdeflist->children);
                extdeflist=extdeflist->children->next;
            }
        }
        else if(strcmp(node->V.v_string,"ExtDef")==0){
            if(strcmp(node->children->next->V.v_string,"SEMI")==0){
                // ExtDef->Specifier SEMI
                struct Tree* specifier=node->children;
                do_semantics(specifier);
            }
            else if(strcmp(node->children->next->V.v_string,"ExtDecList")==0){
                // ExtDef->Specifier ExtDecList SEMI
                struct Tree* specifier=node->children;
                do_semantics(specifier);
                struct Tree* extdeclist=node->children->next;
                while(extdeclist->children->next!=NULL){
                    // ExtDecList->VarDec COMMA ExtDecList
                    struct Tree* vardec=extdeclist->children;
                    strcpy(vardec->specifier,specifier->specifier);
                    do_semantics(vardec);
                    extdeclist=extdeclist->children->next->next;
                }
                {// ExtDecList->VarDec
                struct Tree* vardec=extdeclist->children;
                strcpy(vardec->specifier,specifier->specifier);
                do_semantics(vardec);
                }
            }
            else{
                // ExtDef->Specifier FunDec CompSt
                struct Tree* specifier=node->children, *fundec=node->children->next, *id=fundec->children;
                do_semantics(specifier);
                Type ret_type=(Type)malloc(sizeof(struct Type_));
                ret_type->kind=BASIC;
                strcpy(ret_type->u.basic,specifier->specifier);
                char name[40];
                strcpy(name,id->V.v_string);
                if(strcmp(id->next->next->V.v_string,"RP")==0){
                    // FunDec->ID LP RP
                    if(findSymbolWithName(id->V.v_string,1)!=NULL){
                        PrintErrorNum(4,fundec->pos);
                        break;
                    }
                    else 
                        addFunc2Symbol(name,1,ret_type
                        ,0,NULL);
                }
                else{
                    // FunDec->ID LP VarList RP
                    struct Tree* varlist=fundec->children->next->next;
                    int p_num=1;
                    struct Tree* v=varlist;
                    while(v->children->next!=NULL){
                        ++p_num;
                        v=v->children->next->next;
                    }
                    Type* param=(Type*)malloc(sizeof(Type)*p_num);
                    int index=0;
                    while(varlist->children->next!=NULL){
                        // VarList->ParamDec COMMA VarList
                        struct Tree* paramdec=varlist->children;
                        do_semantics(paramdec);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t=paramdec->ParamDecType;
                        param[index]=t;
                        varlist=varlist->children->next->next;
                        ++index;
                    }
                    {
                        struct Tree* paramdec=varlist->children;
                        do_semantics(paramdec);;
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t=paramdec->ParamDecType;
                        param[index]=t;
                    }
                    if(findSymbolWithName(name,1)!=NULL){
                        PrintErrorNum(4,node->pos);
                        break;
                    }
                    else 
                        addFunc2Symbol(name,1,ret_type
                        ,p_num,param);
                }
                struct Tree* compst=node->children->next->next;
                compst->ReturnType=ret_type;
                do_semantics(compst);
            }
        }
        else if(strcmp(node->V.v_string,"Specifier")==0){
            if(strcmp(node->children->V.v_string,"StructSpecifier")==0){
                //Specifier->Structspecifier
                do_semantics(node->children);
                strcpy(node->specifier,node->children->specifier);
            }
            else{
                //Specifier->TYPE
                strcpy(node->specifier,node->children->V.v_string);
            }
        }
        else if(strcmp(node->V.v_string,"StructSpecifier")==0){
            if(strcmp(node->children->next->V.v_string,"Tag")==0){
                //StructSpecifier->STRUCT Tag
                struct TypeTable* t=findStructWithName(node->children->next->children->V.v_string);
                if(t==NULL){
                    PrintErrorNum(17,node->pos);
                    strcpy(node->specifier,"int");
                    break;
                }
                else
                    strcpy(node->specifier,t->name);
            }
            else{
                //StructSpecifier->STRUCT OptTag LC DefList RC
                char*tag=add2StructTable(node);
                strcpy(node->specifier,tag);
            }
        }
        else if(strcmp(node->V.v_string,"VarDec")==0){
            struct Tree* vardec=node, *id=node->children;
            if(id->next==NULL){
                // VarDec->ID
                struct symbol* s=findSymbolWithName(id->V.v_string,0);
                struct TypeTable* tt=findStructWithName(id->V.v_string);
                if(s!=NULL){
                    PrintErrorNum(3,vardec->pos);
                    vardec->exp_type=s->val.var;
                    break;
                }
                else if(tt!=NULL){
                    PrintErrorNum(3,vardec->pos);
                    vardec->exp_type=genBasicType(tt->name);
                    break;
                }
                else
                    addVar2Symbol(id->V.v_string,0,vardec->specifier);
            }
            else{
                // VarDec->ID LB INT RB
                struct symbol* s=findSymbolWithName(id->V.v_string,0);
                struct TypeTable* tt=findStructWithName(id->V.v_string);
                if(s!=NULL){
                    PrintErrorNum(3,node->pos);
                    break;
                }
                else if(tt!=NULL){
                    PrintErrorNum(3,node->pos);
                    break;
                }
                else
                    addArray2Symbol(vardec,0,vardec->specifier);
            }
        }
        else if(strcmp(node->V.v_string,"ParamDec")==0){
            //ParamDec->Specifier VarDec
            struct Tree* specifier=node->children;
            do_semantics(specifier);
            struct Tree* vardec=node->children->next, *id=vardec->children;
            if(id->next==NULL){
                // VarDec->ID
                Type t=(Type)malloc(sizeof(struct Type_));
                t->kind=BASIC;
                strcpy(t->u.basic,specifier->specifier);
                node->ParamDecType=t;
                if(findSymbolWithName(id->V.v_string,0)!=NULL){
                    PrintErrorNum(3,vardec->pos);
                    break;
                }
                else if(findStructWithName(id->V.v_string)!=NULL){
                    PrintErrorNum(3,vardec->pos);
                    break;
                }
                else{
                    addVar2Symbol(id->V.v_string,0,specifier->specifier);
                    setParam(id->V.v_string);
                }
            }
            else{
                node->ParamDecType=genArrayType(vardec,specifier->specifier);
                if(findSymbolWithName(id->V.v_string,0)!=NULL){
                    PrintErrorNum(3,vardec->pos);
                    break;
                }
                else if(findStructWithName(id->V.v_string)!=NULL){
                    PrintErrorNum(3,vardec->pos);
                    break;
                }
                else
                    addArray2Symbol(vardec,0,specifier->specifier);
            }
        }
        else if(strcmp(node->V.v_string,"CompSt")==0){
            //CompSt->LC DefList StmtList RC
            struct Tree* deflist=node->children->next, *stmtlist=deflist->next;
            do_semantics(deflist);
            stmtlist->ReturnType=node->ReturnType;
            do_semantics(stmtlist);
        }
        else if(strcmp(node->V.v_string,"DefList")==0){
            //DefList->Def DefList
            //DefList->empty
            struct Tree* deflist=node;
            while(deflist->type!=6){
                // Def->Specifier DecList SEMI
                struct Tree* def=deflist->children, *specifier=def->children, *declist=def->children->next;
                do_semantics(specifier);
                while(declist->children->next!=NULL){
                    // DecList->Dec COMMA DecList
                    struct Tree*vardec=declist->children->children;
                    // Dec->VarDec
                    // Dec->VarDec ASSIGNOP Exp
                    strcpy(vardec->specifier,specifier->specifier);
                    do_semantics(vardec);
                    if(vardec->next!=NULL){
                        struct Tree* exp=vardec->next->next, *v=vardec;
                        while(v->children->next!=NULL) v=v->children;
                        struct Tree* id=v->children;
                        do_semantics(exp);
                        Type t;
                        struct symbol* s=findSymbolWithName(id->V.v_string,0);
                        if(s!=NULL) t=s->val.var;
                        else{
                            struct TypeTable* tt=findStructWithName(id->V.v_string);
                            t=genBasicType(tt->name);
                        }
                        if(!Compare2Type(exp->exp_type,t)){
                            PrintErrorNum(5,vardec->pos);
                        }
                    }
                    declist=declist->children->next->next;
                }
                {
                    // DecList->Dec
                    struct Tree*vardec=declist->children->children;
                    // Dec->VarDec
                    // Dec->VarDec ASSIGNOP Exp
                    strcpy(vardec->specifier,specifier->specifier);
                    do_semantics(vardec);
                    if(vardec->next!=NULL){
                        struct Tree* exp=vardec->next->next, *v=vardec;
                        while(v->children->next!=NULL) v=v->children;
                        struct Tree* id=v->children;
                        do_semantics(exp);
                        Type t;
                        struct symbol* s=findSymbolWithName(id->V.v_string,0);
                        if(s!=NULL) t=s->val.var;
                        else{
                            struct TypeTable* tt=findStructWithName(id->V.v_string);
                            t=genBasicType(tt->name);
                        }
                        if(!Compare2Type(exp->exp_type,t)){
                            PrintErrorNum(5,vardec->pos);
                        }
                    }
                }
                deflist=def->next;;
            }
        }
        else if(strcmp(node->V.v_string,"StmtList")==0){
            //StmtList->Stmt StmtList
            //StmtList->empty
            struct Tree* stmtlist=node;
            while(stmtlist->type!=6){
                struct Tree* stmt=stmtlist->children;
                stmt->ReturnType=stmtlist->ReturnType;
                do_semantics(stmt);
                stmtlist=stmt->next;
                stmtlist->ReturnType=stmt->ReturnType;
            }
        }
        else if(strcmp(node->V.v_string,"Stmt")==0){
            if(node->children->next==NULL){
                //Stmt->Compst
                struct Tree* compst=node->children;
                compst->ReturnType=node->ReturnType;
                do_semantics(compst);
            }
            else if(node->children->next->next==NULL){
                //Stmt->Exp SEMI
                struct Tree* exp=node->children;
                do_semantics(exp);
            }
            else if(node->children->next->next->next==NULL){
                //Stmt->RETURN Exp SEMI
                struct Tree* exp=node->children->next;
                do_semantics(exp);
                if(!Compare2Type(node->ReturnType,exp->exp_type)){
                    PrintErrorNum(8,exp->pos);
                }
            }
            else if(strcmp(node->children->V.v_string,"WHILE")==0){
                //Stmt->WHILE LP Exp RP Stmt
                struct Tree* exp=node->children->next->next, *stmt1=node->children->next->next->next->next;
                do_semantics(exp);
                if(exp->exp_type->kind!=BASIC||
                    strcmp(exp->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,exp->pos);
                }
                stmt1->ReturnType=node->ReturnType;
                do_semantics(stmt1);
            }
            else if(node->children->next->next->next->next->next==NULL){
                //stmt->IF LP Exp RP Stmt
                struct Tree* exp=node->children->next->next, *stmt1=node->children->next->next->next->next;
                do_semantics(exp);
                if(exp->exp_type->kind!=BASIC||
                    strcmp(exp->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,exp->pos);
                }
                stmt1->ReturnType=node->ReturnType;
                do_semantics(stmt1);
            }
            else{
                //stmt->IF LP Exp RP Stmt ELSE Stmt
                struct Tree* exp=node->children->next->next, *stmt1=node->children->next->next->next->next, *stmt2=stmt1->next->next;
                do_semantics(exp);
                if(exp->exp_type->kind!=BASIC||
                    strcmp(exp->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,exp->pos);
                }
                stmt1->ReturnType=node->ReturnType;
                do_semantics(stmt1);
                stmt2->ReturnType=node->ReturnType;
                do_semantics(stmt2);
            }
        }
        else{
            do_semantics(node->children);
            do_semantics(node->next);
        }
        break;
    }
    default:{
        do_semantics(node->children);
        do_semantics(node->next);
        break;
    }  
    }
}

void semantics(){
    initSymbol();
    initStruct();
    {
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=BASIC;
        strcpy(t->u.basic,"int");
        addFunc2Symbol("read",1,t,0,NULL);
        Type* p=(Type*)malloc(sizeof(Type));
        p[0]=t;
        addFunc2Symbol("write",1,t,1,p);
    }
    do_semantics(root);
}

int calculateVarSize(Type var){
    if(var->kind==ARRAY){
        return calculateVarSize(var->u.array.elem)*var->u.array.size;
    }
    else{
        if(strcmp(var->u.basic,"int")==0 || strcmp(var->u.basic,"float")==0){
            return 4;
        }
        return findStructWithName(var->u.basic)->size;
    }
}

void setParam(char* name){
    struct symbol* s=findSymbolWithName(name,0);
    s->param=1;
}