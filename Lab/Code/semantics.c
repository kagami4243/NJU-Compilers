#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantics.h"

struct symbol* symbolTable;
struct TypeTable* structTable;

static int error_pos=-1;

void PrintErrorNum(int num, int pos){
    if(error_pos!=pos){
        printf("Error type %d at Line %d: \n",num, pos);
        error_pos=pos;
    }
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
    return;
}

char* addArray2Symbol(struct Tree*node,int type,char*type_name){
    Type head,tail;
    head=tail=NULL;
    while(node->children->next!=NULL){
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=NULL;
        if(head==NULL) head=tail=t;
        else{
            tail->u.array.elem=t;
            tail=t;
        }
        node=node->children;
    }
    Type t=(Type)malloc(sizeof(struct Type_));
    t->kind=BASIC;
    strcpy(t->u.basic,type_name);
    tail->u.array.elem=t;
    tail=t;
    struct symbol* s=(struct symbol*)malloc(sizeof(struct symbol));
    s->next=symbolTable->next;
    symbolTable->next=s;
    s->type=type;
    strcpy(s->name,node->children->V.v_string);
    s->val.var=head;
    char* name=(char*)malloc(sizeof(char)*40);
    strcpy(name,s->name);
    return name;
}

Type genArrayType(struct Tree*node,char*type_name){
    Type head,tail;
    head=tail=NULL;
    while(node->children->next!=NULL){
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=NULL;
        if(head==NULL) head=tail=t;
        else{
            tail->u.array.elem=t;
            tail=t;
        }
        node=node->children;
    }
    Type t=(Type)malloc(sizeof(struct Type_));
    t->kind=BASIC;
    strcpy(t->u.basic,type_name);
    tail->u.array.elem=t;
    tail=t;
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
    if(t1==NULL && t2==NULL) return 1;
    if(t1==NULL || t2==NULL) return 0;
    //printf("%d %d\n",t1->kind,t2->kind);
    if(t1->kind!=t2->kind) return 0;
    if(t1->kind==BASIC){
        //printf("BASIC: %s %s\n",t1->u.basic,t2->u.basic);
        if(strcmp(t1->u.basic,"int")==0 || strcmp(t1->u.basic,"float")==0
            || strcmp(t2->u.basic,"int")==0 || strcmp(t2->u.basic,"float")==0){
            if(strcmp(t1->u.basic,t2->u.basic)!=0){
                return 0;
            } 
        }
        else if(strcmp(t1->u.basic,t2->u.basic)!=0){
            struct TypeTable*tt1=findStructWithName(t1->u.basic),*tt2=findStructWithName(t2->u.basic);
            FieldList f1=tt1->t->tail,f2=tt2->t->tail;
            while(f1!=NULL && f2!=NULL){
                if(!Compare2Type(f1->type,f2->type))
                    return 0;
                f1=f1->tail,f2=f2->tail;
            }
            if((f1!=NULL && f2==NULL) || (f1==NULL && f2!=NULL)) return 0;
        }
    }   
    else if(t1->kind==ARRAY){
        //printf("ARRAY: \n");
        t1=t1->u.array.elem;
        t2=t2->u.array.elem;
        return Compare2Type(t1,t2);
    }
    return 1;
}

char* add2StructTable(struct Tree*node){
    struct Tree*tag=node->children->next;
    char* Tag;
    Tag=(char*)malloc(sizeof(char)*40);
    if(tag->type!=6){
        strcpy(Tag,node->children->next->children->V.v_string);
    }
    else{
        sprintf(Tag,"%d",StructTableUni++);
    }
    if((findSymbolWithName(Tag,0)!=NULL) || (findStructWithName(Tag)!=NULL)){
        PrintErrorNum(16,node->pos);
    }
    node=node->children->next->next->next; // node->DefList
    FieldList t=(FieldList)malloc(sizeof(struct FieldList_));
    t->tail=NULL;
    FieldList tail=t;
    while(node->type!=6){
        node=node->children; // node->Def
        do_semantics(node->children);
        struct Tree*declist=node->children->next;
        while(declist->children->next!=NULL){
            struct Tree*vardec=declist->children->children;
            FieldList field=(FieldList)malloc(sizeof(struct FieldList_));
            field->tail=NULL;
            Type unit=(Type)malloc(sizeof(struct Type_));
            char name[40];
            if(vardec->children->next==NULL){
                unit->kind=BASIC;
                strcpy(unit->u.basic,node->children->specifier);
                strcpy(name,vardec->children->V.v_string);
                if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL){
                    PrintErrorNum(15,vardec->children->pos);
                }
                else 
                    addVar2Symbol(vardec->children->V.v_string,0,node->children->specifier);
            }
            else{
                unit->kind=ARRAY;
                unit->u.array=genArrayType(vardec,node->children->specifier)->u.array;
                struct Tree* r=vardec;
                while(r->children->next!=NULL) r=r->children;
                if(findSymbolWithName(r->children->V.v_string,0)!=NULL){
                    PrintErrorNum(15,vardec->children->pos);
                }
                else 
                    strcpy(name,addArray2Symbol(vardec,0,node->children->specifier));
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
        {struct Tree*vardec=declist->children->children;
        FieldList field=(FieldList)malloc(sizeof(struct FieldList_));
        field->tail=NULL;
        Type unit=(Type)malloc(sizeof(struct Type_));
        char name[40];
        if(vardec->children->next==NULL){
            unit->kind=BASIC;
            strcpy(unit->u.basic,node->children->specifier);
            strcpy(name,vardec->children->V.v_string);
            if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL){
                PrintErrorNum(15,vardec->children->pos);
            }
            else 
                addVar2Symbol(vardec->children->V.v_string,0,node->children->specifier);
        }
        else{
            unit->kind=ARRAY;
            unit->u.array=genArrayType(vardec,node->children->specifier)->u.array;
            struct Tree* r=vardec;
            while(r->children->next!=NULL) r=r->children;
            if(findSymbolWithName(r->children->V.v_string,0)!=NULL){
                PrintErrorNum(15,vardec->children->pos);
            }
            else 
                strcpy(name,addArray2Symbol(vardec,0,node->children->specifier));
        }
        field->type=unit;
        strcpy(field->name,name);
        tail->tail=field;
        tail=field;
        vardec->exp_type=unit;
        if(vardec->next!=NULL){
            PrintErrorNum(15,node->pos);
        }
        }
        node=node->next;
    }
    struct TypeTable* tt=(struct TypeTable*)malloc(sizeof(struct TypeTable));
    strcpy(tt->name,Tag);
    tt->t=t;
    tt->next=structTable->next;
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
    // if(node->type==1) printf("%d\n",node->V.v_int);
    switch (node->type)
    {
    case 0:{
        if(strcmp(node->V.v_string,"Exp")==0){
            if(node->children->next==NULL){
                switch(node->children->type){
                    case 1:{
                        // Exp->INT
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"int");
                        node->exp_type=t; 
                        node->leftValue=0;
                        break;
                    }
                    case 2:{
                        // Exp->FLOAT
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"float");
                        node->exp_type=t;
                        node->leftValue=0;
                        break;
                    }
                    case 4:{
                        // Exp->ID
                        struct symbol* s=findSymbolWithName(node->children->V.v_string,0);
                        if(s==NULL){
                            PrintErrorNum(1,node->pos);
                            Type t=(Type)malloc(sizeof(struct Type_));
                            t->kind=BASIC;
                            strcpy(t->u.basic,"int");
                            node->exp_type=t; 
                            node->leftValue=1;
                            break;
                        } 
                        node->exp_type=s->val.var;
                        node->leftValue=1;
                        break;
                    }
                    default: break;
                }
            }
            else if(node->children->next->next==NULL){
                struct Tree* t=node->children->next;
                if(strcmp(node->children->V.v_string,"MINUS")==0){
                    // Exp->MINUS Exp
                    do_semantics(t);
                    node->leftValue=0;
                    node->exp_type=t->exp_type;
                    if(t->exp_type->kind!=BASIC ||
                     (strcmp(t->exp_type->u.basic,"int")!=0 && strcmp(t->exp_type->u.basic,"float")!=0)){
                        PrintErrorNum(1,node->pos);
                        break;
                    }
                }
                else{
                    // Exp->NOT Exp
                    do_semantics(t);
                    node->leftValue=0;
                    node->exp_type=t->exp_type;
                    if(t->exp_type->kind!=BASIC || strcmp(t->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(1,node->pos);
                        break;
                    }
                }
            }
            else if(node->children->next->next->next==NULL){
                char* s=node->children->next->V.v_string;
                if(strcmp(s,"AND")==0 || strcmp(s,"OR")==0){
                    // Exp->Exp AND Exp
                    // Exp->Exp OR Exp
                    do_semantics(node->children);
                    do_semantics(node->children->next->next);
                    node->leftValue=0;
                    Type t=(Type)malloc(sizeof(struct Type_));
                    t->kind=BASIC;
                    strcpy(t->u.basic,"int");
                    node->exp_type=t;
                    if(node->children->exp_type->kind!=BASIC || node->children->next->next->exp_type->kind!=BASIC){
                        PrintErrorNum(7,node->pos);
                        break;
                    }
                    if(strcmp(node->children->exp_type->u.basic,"int")!=0 || strcmp(node->children->next->next->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,node->pos);
                        break;
                    }
                }
                else if(strcmp(s,"ASSIGNOP")==0){
                    // Exp->Exp ASSIGNOP Exp
                    do_semantics(node->children);
                    do_semantics(node->children->next->next);
                    node->leftValue=0;
                    node->exp_type=node->children->exp_type;
                    if(!Compare2Type(node->children->exp_type,node->children->next->next->exp_type)){
                        PrintErrorNum(5,node->pos);
                        break;
                    }
                    if(node->children->leftValue==0){
                        PrintErrorNum(6,node->pos);
                        break;
                    }
                }
                else if(strcmp(s,"PLUS")==0 || strcmp(s,"MINUS")==0 || strcmp(s,"RELOP")==0
                        || strcmp(s,"STAR")==0 || strcmp(s,"DIV")==0){
                    // Exp->Exp RELOP Exp
                    // Exp->Exp PLUS Exp
                    // Exp->Exp MINUS Exp
                    // Exp->Exp STAR Exp
                    // Exp->Exp DIV Exp
                    do_semantics(node->children);
                    do_semantics(node->children->next->next);
                    node->leftValue=0;
                    Type t=(Type)malloc(sizeof(struct Type_));
                    t->kind=BASIC;
                    strcpy(t->u.basic,"int");
                    if(node->children->exp_type->kind!=BASIC || node->children->next->next->exp_type->kind!=BASIC){
                        PrintErrorNum(7,node->pos);
                        node->exp_type=t;
                        break;
                    }
                    if(strcmp(node->children->exp_type->u.basic,node->children->next->next->exp_type->u.basic)!=0){
                        PrintErrorNum(7,node->pos);
                        node->exp_type=t;
                        break;
                    }
                    if(strcmp(node->children->exp_type->u.basic,"int")!=0 && strcmp(node->children->exp_type->u.basic,"float")!=0){
                        PrintErrorNum(7,node->pos);
                        node->exp_type=t;
                        break;
                    }
                    if(strcmp(s,"RELOP")==0){
                        node->exp_type=t;
                    }
                    else 
                        node->exp_type=node->children->exp_type;
                }
                else if(strcmp(s,"Exp")==0){
                    // Exp->LP Exp RP
                    do_semantics(node->children->next);
                    node->leftValue=0;
                    node->exp_type=node->children->next->exp_type;
                }
                else if(strcmp(s,"LP")==0){
                    // Exp->ID LP RP
                    struct symbol* s=findSymbolWithName(node->children->V.v_string,1);
                    if(findSymbolWithName(node->children->V.v_string,0)!=NULL){
                        PrintErrorNum(11,node->pos);
                        break;
                    }
                    if(s==NULL){
                        PrintErrorNum(7,node->pos);
                        break;
                    }
                    if(s->val.func->param_num!=0){
                        PrintErrorNum(7,node->pos);
                        break;
                    }
                    node->exp_type=s->val.func->return_type;
                }
                else{
                    // Exp->Exp DOT ID
                    do_semantics(node->children);
                    node->leftValue=1;
                    struct TypeTable* s=findStructWithName(node->children->exp_type->u.basic);
                    if(s==NULL){
                        PrintErrorNum(13,node->pos);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"int");
                        node->exp_type=t;
                        break;
                    }
                    FieldList f=s->t->tail;
                    while(f!=NULL){
                        if(strcmp(f->name,node->children->next->next->V.v_string)==0)
                            break;
                        f=f->tail;
                    }
                    if(f==NULL){
                        PrintErrorNum(14,node->pos);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"int");
                        node->exp_type=t;
                        break;
                    }
                    node->exp_type=f->type;
                }
            }
            else if(node->children->next->next->next->next==NULL){
                if(strcmp(node->children->next->V.v_string,"LP")==0){
                    // Exp->ID LP Args RP
                    struct symbol*s=findSymbolWithName(node->children->V.v_string,1);
                    if(findSymbolWithName(node->children->V.v_string,0)!=NULL){
                        PrintErrorNum(11,node->pos);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"int");
                        node->exp_type=t;
                        break;
                    }
                    if(s==NULL){
                        PrintErrorNum(2,node->pos);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"int");
                        node->exp_type=t;
                        break;
                    }
                    node->exp_type=s->val.func->return_type;
                    struct Tree* cur=node->children->next->next;//cur->Args
                    int index=0;
                    int error=0;
                    while(cur->children->next!=NULL){
                        do_semantics(cur->children);
                        if(!Compare2Type(cur->children->exp_type,s->val.func->param_type[index])){
                            PrintErrorNum(9,node->pos);
                            error=1;
                            break;
                        }
                        cur=cur->children->next->next;
                        ++index;
                    }
                    if(error!=1){
                        do_semantics(cur->children);
                        if(!Compare2Type(cur->children->exp_type,s->val.func->param_type[index])){
                            PrintErrorNum(9,node->pos);
                            break;
                        }
                        ++index;
                        if(index!=s->val.func->param_num){
                            PrintErrorNum(9,node->pos);
                            break;
                        }
                    }
                }
                else{
                    // Exp->Exp LB Exp RB
                    do_semantics(node->children);
                    do_semantics(node->children->next->next);
                    node->leftValue=1;
                    if(node->children->exp_type->kind!=ARRAY){
                        PrintErrorNum(10,node->pos);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t->kind=BASIC;
                        strcpy(t->u.basic,"int");
                        node->exp_type=t;
                        break;
                    }
                    node->exp_type=node->children->exp_type->u.array.elem;
                    if(node->children->next->next->exp_type->kind!=BASIC){
                        PrintErrorNum(12,node->pos);
                        break;
                    }
                    if(strcmp(node->children->next->next->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(12,node->pos);
                        break;
                    }
                }
            }
        }
        else if(strcmp(node->V.v_string,"ExtDefList")==0){
            // ExtDefList->ExtDef ExtDefList
            // ExtDefList->empty
            while(node->type!=6){
                do_semantics(node->children);
                node=node->children->next;
            }
        }
        else if(strcmp(node->V.v_string,"ExtDef")==0){
            if(strcmp(node->children->next->V.v_string,"SEMI")==0){
                //ExtDef->Specifier SEMI
                do_semantics(node->children);
            }
            else if(strcmp(node->children->next->V.v_string,"ExtDecList")==0){
                //ExtDef->Specifier ExtDecList SEMI
                do_semantics(node->children);
                char specifier[40];
                strcpy(specifier,node->children->specifier);
                struct Tree* cur=node->children->next; // cur->ExtDecList
                while(cur->children->next!=NULL){
                    if(cur->children->children->next==NULL){
                        if(findSymbolWithName(cur->children->children->V.v_string,0)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else if(findStructWithName(cur->children->children->V.v_string)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else
                            addVar2Symbol(cur->children->children->V.v_string,0,specifier);
                    }
                    else{
                        if(findSymbolWithName(cur->children->children->V.v_string,0)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else if(findStructWithName(cur->children->children->V.v_string)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else
                            addArray2Symbol(cur->children,0,specifier);
                    }
                    cur=cur->children->next->next;
                }
                if(cur->children->children->next==NULL){
                    if(findSymbolWithName(cur->children->children->V.v_string,0)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else if(findStructWithName(cur->children->children->V.v_string)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else
                        addVar2Symbol(cur->children->children->V.v_string,0,specifier);
                }
                else{
                    if(findSymbolWithName(cur->children->children->V.v_string,0)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else if(findStructWithName(cur->children->children->V.v_string)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else
                        addArray2Symbol(cur->children,0,specifier);
                }
            }
            else{
                //ExtDef->Specifier FunDec CompSt
                do_semantics(node->children);
                struct Tree* cur=node->children->next; // cur->FunDec
                Type ret_type=(Type)malloc(sizeof(struct Type_));
                ret_type->kind=BASIC;
                strcpy(ret_type->u.basic,node->children->specifier);
                char name[40];
                strcpy(name,cur->children->V.v_string);
                if(strcmp(cur->children->next->next->V.v_string,"RP")==0){
                    if(findSymbolWithName(cur->children->V.v_string,1)!=NULL){
                        PrintErrorNum(4,node->pos);
                        break;
                    }
                    else 
                        addFunc2Symbol(name,1,ret_type
                        ,0,NULL);
                }
                else{
                    cur=cur->children->next->next; // cur->VarList
                    int p_num=1;
                    struct Tree* c=cur;
                    while(c->children->next!=NULL){
                        ++p_num;
                        c=c->children->next->next;
                    }
                    Type* param=(Type*)malloc(sizeof(Type)*p_num);
                    int index=0;
                    while(cur->children->next!=NULL){
                        do_semantics(cur->children);
                        Type t=(Type)malloc(sizeof(struct Type_));
                        t=cur->children->ParamDecType;
                        param[index]=t;
                        cur=cur->children->next->next;
                        ++index;
                    }
                    {do_semantics(cur->children);
                    Type t=(Type)malloc(sizeof(struct Type_));
                    t=cur->children->ParamDecType;
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
                node->children->next->next->ReturnType=ret_type;
                do_semantics(node->children->next->next);
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
        else if(strcmp(node->V.v_string,"ParamDec")==0){
            //ParamDec->Specifier VarDec
            do_semantics(node->children);
            if(node->children->next->children->next==NULL){
                Type t=(Type)malloc(sizeof(struct Type_));
                t->kind=BASIC;
                strcpy(t->u.basic,node->children->specifier);
                if(findSymbolWithName(node->children->next->children->V.v_string,0)!=NULL){
                    PrintErrorNum(3,node->pos);
                    break;
                }
                else if(findStructWithName(node->children->next->children->V.v_string)!=NULL){
                    PrintErrorNum(3,node->pos);
                    break;
                }
                else
                    addVar2Symbol(node->children->next->children->V.v_string,0,node->children->specifier);
                node->ParamDecType=t;
            }
            else{
                node->ParamDecType=genArrayType(node->children->next,node->children->specifier);
                if(findSymbolWithName(node->children->next->children->V.v_string,0)!=NULL){
                    PrintErrorNum(3,node->pos);
                    break;
                }
                else if(findStructWithName(node->children->next->children->V.v_string)!=NULL){
                    PrintErrorNum(3,node->pos);
                    break;
                }
                else
                    addArray2Symbol(node->children->next,0,node->children->specifier);
            }
        }
        else if(strcmp(node->V.v_string,"CompSt")==0){
            //CompSt->LC DefList StmtList RC
            do_semantics(node->children->next);
            node->children->next->next->ReturnType=node->ReturnType;
            do_semantics(node->children->next->next);
        }
        else if(strcmp(node->V.v_string,"DefList")==0){
            //DefList->Def DefList
            //DefList->empty
            while(node->type!=6){
                node=node->children; // node->Def
                do_semantics(node->children);
                struct Tree*declist=node->children->next;
                while(declist->children->next!=NULL){
                    struct Tree*vardec=declist->children->children;
                    if(vardec->children->next==NULL){
                        if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else if(findStructWithName(vardec->children->V.v_string)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else 
                            addVar2Symbol(vardec->children->V.v_string,0,node->children->specifier);
                    }
                    else{
                        if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else if(findStructWithName(vardec->children->V.v_string)!=NULL){
                            PrintErrorNum(3,node->pos);
                            break;
                        }
                        else 
                            addArray2Symbol(vardec,0,node->children->specifier);
                    }
                    if(vardec->next!=NULL){
                        do_semantics(vardec->next->next);
                        if(!Compare2Type(vardec->next->next->exp_type,findSymbolWithName(vardec->children->V.v_string,0)->val.var)){
                            PrintErrorNum(5,node->pos);
                        }
                    }
                    declist=declist->children->next->next;
                }
                {struct Tree*vardec=declist->children->children;
                if(vardec->children->next==NULL){
                    if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else if(findStructWithName(vardec->children->V.v_string)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else
                        addVar2Symbol(vardec->children->V.v_string,0,node->children->specifier);
                }
                else{
                    if(findSymbolWithName(vardec->children->V.v_string,0)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else if(findStructWithName(vardec->children->V.v_string)!=NULL){
                        PrintErrorNum(3,node->pos);
                        break;
                    }
                    else
                        addArray2Symbol(vardec,0,node->children->specifier);
                }
                if(vardec->next!=NULL){
                    do_semantics(vardec->next->next);
                    if(!Compare2Type(vardec->next->next->exp_type,findSymbolWithName(vardec->children->V.v_string,0)->val.var)){
                        PrintErrorNum(5,node->pos);
                    }
                }
                }
                node=node->next;
            }
        }
        else if(strcmp(node->V.v_string,"StmtList")==0){
            //StmtList->Stmt StmtList
            //StmtList->empty
            while(node->type!=6){
                node->children->ReturnType=node->ReturnType;
                node=node->children; // node->Stmt
                do_semantics(node);
                node->next->ReturnType=node->ReturnType;
                node=node->next;
            }
        }
        else if(strcmp(node->V.v_string,"Stmt")==0){
            if(node->children->next==NULL){
                //Stmt->Compst
                node->children->ReturnType=node->ReturnType;
                do_semantics(node->children);
            }
            else if(node->children->next->next==NULL){
                //Stmt->Exp SEMI
                do_semantics(node->children);
            }
            else if(node->children->next->next->next==NULL){
                //Stmt->RETURN Exp SEMI
                do_semantics(node->children->next);
                if(!Compare2Type(node->ReturnType,node->children->next->exp_type)){
                    PrintErrorNum(8,node->pos);
                }
            }
            else if(strcmp(node->children->V.v_string,"WHILE")==0){
                //Stmt->WHILE LP Exp RP Stmt
                do_semantics(node->children->next->next);
                if(node->children->next->next->exp_type->kind!=BASIC||
                    strcmp(node->children->next->next->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,node->pos);
                }
                node->children->next->next->next->next->ReturnType=node->ReturnType;
                do_semantics(node->children->next->next->next->next);
            }
            else if(node->children->next->next->next->next->next==NULL){
                //stmt->IF LP Exp RP Stmt
                do_semantics(node->children->next->next);
                if(node->children->next->next->exp_type->kind!=BASIC||
                    strcmp(node->children->next->next->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,node->pos);
                }
                node->children->next->next->next->next->ReturnType=node->ReturnType;
                do_semantics(node->children->next->next->next->next);
            }
            else{
                //stmt->IF LP Exp RP Stmt ELSE Stmt
                do_semantics(node->children->next->next);
                if(node->children->next->next->exp_type->kind!=BASIC||
                    strcmp(node->children->next->next->exp_type->u.basic,"int")!=0){
                        PrintErrorNum(7,node->pos);
                }
                node->children->next->next->next->next->ReturnType=node->ReturnType;
                do_semantics(node->children->next->next->next->next);
                node->children->next->next->next->next->next->next->ReturnType=node->ReturnType;
                do_semantics(node->children->next->next->next->next->next->next);
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
    do_semantics(root);
}