#include"semantics.h"
#include"ast.h"
#include"ir.h"
#include<string.h>
#include<stdio.h>
extern FILE* output;
static int tmp_num=1,label_num=1;
void translate_Program(){
    translate_ExtDefList(root->children);
}

void translate_ExtDefList(struct Tree* ExtDefList){
    while(ExtDefList->type!=6){
        translate_ExtDef(ExtDefList->children);
        ExtDefList=ExtDefList->children->next;
    }
}

void translate_ExtDef(struct Tree* ExtDef){
    if(strcmp(ExtDef->children->next->V.v_string,"ExtDecList")==0){
        translate_ExtDecList(ExtDef->children->next,ExtDef->children->specifier);
    }
    else if(strcmp(ExtDef->children->next->V.v_string,"FunDec")==0){
        translate_FunDec(ExtDef->children->next);
        translate_CompSt(ExtDef->children->next->next);
    }
}

void translate_ExtDecList(struct Tree* extdeclist,char* specifier){
    while(extdeclist->children->next!=NULL){
        translate_VarDec(extdeclist->children,specifier);
        extdeclist=extdeclist->children->next->next;
    }
    translate_VarDec(extdeclist->children,specifier);
}

void translate_VarDec(struct Tree* vardec,char* specifier){
    if(vardec->children->next!=NULL){
        while(vardec->children->next!=NULL) vardec=vardec->children;
        Type t=findSymbolWithName(vardec->children->V.v_string,0)->val.var;
        int size=calculateVarSize(t);
        fprintf(output,"DEC %s %d\n",vardec->children->V.v_string,size);
    }
    else{
        Type t=findSymbolWithName(vardec->children->V.v_string,0)->val.var;
        if(strcmp(t->u.basic,"int")!=0){
            int size=calculateVarSize(t);
            fprintf(output,"DEC %s %d\n",vardec->children->V.v_string,size);
        }
    }
}

void translate_FunDec(struct Tree* fundec){
    fprintf(output,"FUNCTION %s :\n",fundec->children->V.v_string);
    if(strcmp(fundec->children->next->next->V.v_string,"VarList")==0){
        translate_VarList(fundec->children->next->next);
    }
}

void translate_VarList(struct Tree* varlist){
    while(varlist->children->next!=NULL){
        translate_ParamDec(varlist->children);
        varlist=varlist->children->next->next;
    }
    translate_ParamDec(varlist->children);
}

void translate_ParamDec(struct Tree* paramdec){
    struct Tree* vardec=paramdec->children->next;
    while(vardec->children->next!=NULL) vardec=vardec->children;
    fprintf(output,"PARAM %s\n",vardec->children->V.v_string);
}

void translate_CompSt(struct Tree* compst){
    translate_DefList(compst->children->next);
    translate_StmtList(compst->children->next->next);
}

void translate_StmtList(struct Tree* stmtlist){
    while(stmtlist->type!=6){
        translate_Stmt(stmtlist->children);
        stmtlist=stmtlist->children->next;
    }
}

void translate_Stmt(struct Tree* stmt){
    if(strcmp(stmt->children->V.v_string,"Exp")==0){
        translate_Exp(stmt->children,NULL);
    }
    else if(strcmp(stmt->children->V.v_string,"CompSt")==0){
        translate_CompSt(stmt->children);
    }
    else if(strcmp(stmt->children->V.v_string,"RETURN")==0){
        char* t=newTemp();
        translate_Exp(stmt->children->next,t);
        fprintf(output,"RETURN %s\n",t);
    }
    else if(strcmp(stmt->children->V.v_string,"IF")==0){
        if(stmt->children->next->next->next->next->next==NULL){
            char* l1=newLabel(),*l2=newLabel();
            translate_Cond(stmt->children->next->next,l1,l2);
            fprintf(output,"LABEL %s :\n",l1);
            translate_Stmt(stmt->children->next->next->next->next);
            fprintf(output,"LABEL %s :\n",l2);
        }
        else{
            char*l1=newLabel(),*l2=newLabel(),*l3=newLabel();
            translate_Cond(stmt->children->next->next,l1,l2);
            fprintf(output,"LABEL %s :\n",l1);
            translate_Stmt(stmt->children->next->next->next->next);
            fprintf(output,"GOTO %s\n",l3);
            fprintf(output,"LABEL %s :\n",l2);
            translate_Stmt(stmt->children->next->next->next->next->next->next);
            fprintf(output,"LABEL %s :\n",l3);
        }
    }
    else{
        // WHILE
        char*l1=newLabel(),*l2=newLabel(),*l3=newLabel();
        fprintf(output,"LABEL %s :\n",l1);
        translate_Cond(stmt->children->next->next,l2,l3);
        fprintf(output,"LABEL %s :\n",l2);
        translate_Stmt(stmt->children->next->next->next->next);
        fprintf(output,"GOTO %s\n",l1);
        fprintf(output,"LABEL %s :\n",l3);
    }
}

void translate_DefList(struct Tree* deflist){
    while(deflist->type!=6){
        translate_Def(deflist->children);
        deflist=deflist->children->next;
    }
}

void translate_Def(struct Tree* def){
    translate_DecList(def->children->next,def->children->specifier);
}

void translate_DecList(struct Tree* declist, char* specifier){
    while(declist->children->next!=NULL){
        translate_Dec(declist->children,specifier);
        declist=declist->children->next->next;
    }
    translate_Dec(declist->children,specifier);
}

void translate_Dec(struct Tree* dec, char* specifier){
    translate_VarDec(dec->children,specifier);
    if(dec->children->next!=NULL){
        char* t1=newTemp();
        translate_Exp(dec->children->next->next,t1);
        struct Tree* vardec=dec->children;
        while(vardec->children->next!=NULL) vardec=vardec->children;
        fprintf(output,"%s := %s\n",vardec->children->V.v_string,t1);
    }
}

void translate_Exp(struct Tree* exp, char* place){
    if(exp->children->type==1){
        if(place!=NULL) fprintf(output,"%s := #%d\n",place,exp->children->V.v_int);
    }
    else if(exp->children->type==4 && exp->children->next==NULL){
        if(place!=NULL){
            Type t=exp->exp_type;
            if(strcmp("int",t->u.basic)==0)
                fprintf(output,"%s := %s\n",place,exp->children->V.v_string);
            else{
                if(findSymbolWithName(exp->children->V.v_string,0)->param){
                    fprintf(output,"%s := %s\n",place,exp->children->V.v_string);
                }
                else{
                    fprintf(output,"%s := &%s\n",place,exp->children->V.v_string);
                }
            }  
        }
    }
    else if(strcmp(exp->children->next->V.v_string,"ASSIGNOP")==0){
        if(exp->children->children->type==4){
            char v[40];
            strcpy(v,exp->children->children->V.v_string);
            Type t=exp->children->exp_type;
            if(t->kind==BASIC){
                char* t1=newTemp();
                translate_Exp(exp->children->next->next,t1);
                fprintf(output,"%s := %s\n",v,t1);
                if(place!=NULL) fprintf(output,"%s := %s\n",place,v);
            }
            else if(t->kind==ARRAY){
                struct Tree*exp1=exp->children,*exp2=exp->children->next->next;
                Type t2=exp2->exp_type;
                int size=calculateVarSize(t2)/t2->u.array.size;
                for(int i=0;i<t2->u.array.size;++i){
                    char* nt1=newTemp(),*nt2=newTemp();
                    if(findSymbolWithName(exp1->children->V.v_string,0)->param)
                        fprintf(output,"%s := %s",nt1,exp1->children->V.v_string);
                    else fprintf(output,"%s := &%s",nt1,exp1->children->V.v_string);
                    if(i) fprintf(output," + #%d",size*i);
                    fprintf(output,"\n");
                    if(findSymbolWithName(exp2->children->V.v_string,0)->param)
                        fprintf(output,"%s := %s",nt1,exp1->children->V.v_string);
                    else fprintf(output,"%s := &%s",nt2,exp2->children->V.v_string);
                    if(i) fprintf(output," + #%d",size*i);
                    fprintf(output,"\n");
                    fprintf(output,"*%s := *%s\n",nt1,nt2);
                }
            }
        }
        else if(strcmp(exp->children->children->next->V.v_string,"LB")==0){
            if(exp->children->exp_type->kind==BASIC ||
                exp->children->exp_type->u.array.elem->kind!=ARRAY){
                char* t1=newTemp(),*t2=newTemp();
                translate_Exp(exp->children,t1);
                translate_Exp(exp->children->next->next,t2);
                fprintf(output,"%s := %s\n",t1,t2);
                if(place!=NULL) fprintf(output,"%s := %s\n",place,t1);
            }
            else{
                printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
                set_error();
                return;
            }
        } 
        else{
            // Exp DOT ID ASSIGNOP Exp
            char* t1=newTemp(),*t2=newTemp();
            translate_Exp(exp->children,t1);
            translate_Exp(exp->children->next->next,t2);
            fprintf(output,"%s := %s\n",t1,t2);
            if(place!=NULL) fprintf(output,"%s := %s\n",place,t1);
        }
    }
    else if(strcmp(exp->children->next->V.v_string,"PLUS")==0){
        char* t1=newTemp(),*t2=newTemp();
        translate_Exp(exp->children,t1);
        translate_Exp(exp->children->next->next,t2);
        if(place!=NULL) fprintf(output,"%s := %s + %s\n",place,t1,t2);
    }
    else if(strcmp(exp->children->next->V.v_string,"MINUS")==0){
        char* t1=newTemp(),*t2=newTemp();
        translate_Exp(exp->children,t1);
        translate_Exp(exp->children->next->next,t2);
        if(place!=NULL) fprintf(output,"%s := %s - %s\n",place,t1,t2);
    }
    else if(strcmp(exp->children->next->V.v_string,"STAR")==0){
        char* t1=newTemp(),*t2=newTemp();
        translate_Exp(exp->children,t1);
        translate_Exp(exp->children->next->next,t2);
        if(place!=NULL) fprintf(output,"%s := %s * %s\n",place,t1,t2);
    }
    else if(strcmp(exp->children->next->V.v_string,"DIV")==0){
        char* t1=newTemp(),*t2=newTemp();
        translate_Exp(exp->children,t1);
        translate_Exp(exp->children->next->next,t2);
        if(place!=NULL) fprintf(output,"%s := %s / %s\n",place,t1,t2);
    }
    else if(strcmp(exp->children->V.v_string,"MINUS")==0){
        char* t1=newTemp();
        translate_Exp(exp->children->next,t1);
        if(place!=NULL) fprintf(output,"%s := #0 - %s\n",place,t1);
    }
    else if(exp->children->next->type==7 || strcmp(exp->children->V.v_string,"NOT")==0
        || strcmp(exp->children->next->V.v_string,"AND")==0 || strcmp(exp->children->next->V.v_string,"OR")==0){
            char* l1=newLabel(),*l2=newLabel();
            if(place!=NULL) fprintf(output,"%s := #0\n",place);
            translate_Cond(exp,l1,l2);
            fprintf(output,"LABEL %s :\n",l1);
            if(place!=NULL) fprintf(output,"%s := #1\n",place);
            fprintf(output,"LABEL %s :\n",l2);
        }
    else if(strcmp(exp->children->next->V.v_string,"Exp")==0){
        translate_Exp(exp->children->next,place);
    }
    else if(strcmp(exp->children->next->V.v_string,"LB")==0){
        if(exp->children->children->next!=NULL && 
            exp->children->children->next->next!=NULL &&
            strcmp(exp->children->children->next->next->V.v_string,"LB")==0){
                printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
                set_error();
                return;
        }
        if(place!=NULL){
            struct Tree* exp1=exp->children;
            char*t1=newTemp(),*t2=newTemp();
            translate_Exp(exp->children->next->next,t1);
            Type v=exp->children->exp_type;
            int size=calculateVarSize(v)/v->u.array.size;
            fprintf(output,"%s := %s * #%d\n",place,t1,size); 
            translate_Exp(exp1,t2);          
            if(t2[0]=='*') fprintf(output,"%s := %s + %s\n",place,place,t2+1);
            else fprintf(output,"%s := %s + %s\n",place,place,t2);
            char t[40];
            strcpy(t,place);
            sprintf(place,"*%s",t);
        }
    }
    else if(strcmp(exp->children->next->V.v_string,"DOT")==0){
        if(place!=NULL){
            char*t1=newTemp();
            translate_Exp(exp->children,t1);
            char t[40];
            strcpy(t,t1);
            if(t[0]=='*') sprintf(t1,"%s",t+1);
            struct symbol* s=findSymbolWithName(exp->children->V.v_string,0);
            if(s!=NULL && !s->param) sprintf(t1,"&%s",t+1);
            fprintf(output,"%s := %s + #%d\n",place,t1,findOffset(exp->children->exp_type,exp->children->next->next->V.v_string));
            strcpy(t,place);
            sprintf(place,"*%s",t);
        }
    }
    else if(exp->children->type==4){
        if(strcmp(exp->children->next->next->V.v_string,"Args")!=0){
            char* function=(char*)malloc(sizeof(char)*40);
            strcpy(function,exp->children->V.v_string);
            if(strcmp(function,"read")==0){
                if(place!=NULL) fprintf(output,"READ %s\n",place);
            }
            else{
                fprintf(output,"%s := CALL %s\n",place,function);
            }
        }
        else{
            char* function=(char*)malloc(sizeof(char)*40);
            strcpy(function,exp->children->V.v_string);
            arg_list al=(arg_list)malloc(sizeof(struct args));
            al->next=NULL;
            translate_Args(exp->children->next->next,al);
            al=al->next;
            if(strcmp(function,"write")==0){
                fprintf(output,"WRITE %s\n",al->name);
                if(place!=NULL) fprintf(output,"%s := #0\n",place);
                return;
            }
            while(al!=NULL){
                fprintf(output,"ARG %s\n",al->name);
                al=al->next;
            }
            fprintf(output,"%s := CALL %s\n",place,function);
        }
    }
}

void translate_Cond(struct Tree* exp, char* label_true, char* label_false){
    if(exp->children->next==NULL){
        char* t1=newTemp();
        translate_Exp(exp,t1);
        fprintf(output,"IF %s != #0 GOTO %s\n",t1,label_true);
        fprintf(output,"GOTO %s\n",label_false);
    }
    else if(exp->children->next->type==7){
        char* t1=newTemp(),*t2=newTemp();
        translate_Exp(exp->children,t1);
        translate_Exp(exp->children->next->next,t2);
        fprintf(output,"IF %s %s %s GOTO %s\n",t1,exp->children->next->V.v_string,t2,label_true);
        fprintf(output,"GOTO %s\n",label_false);
    }
    else if(strcmp(exp->children->V.v_string,"NOT")==0){
        translate_Cond(exp->children->next,label_false,label_true);
    }
    else if(strcmp(exp->children->next->V.v_string,"AND")==0){
        char* label1=newLabel();
        translate_Cond(exp->children,label1,label_false);
        fprintf(output,"LABEL %s :\n",label1);
        translate_Cond(exp->children->next->next,label_true,label_false);
    }
    else if(strcmp(exp->children->next->V.v_string,"OR")==0){
        char* label1=newLabel();
        translate_Cond(exp->children,label_true,label1);
        fprintf(output,"LABEL %s :\n",label1);
        translate_Cond(exp->children->next->next,label_true,label_false);
    }
    else{
        char* t1=newTemp();
        translate_Exp(exp,t1);
        fprintf(output,"IF %s != #0 GOTO %s\n",t1,label_true);
        fprintf(output,"GOTO %s\n",label_false);
    }
}

void translate_Args(struct Tree* args,arg_list al){
    char* t1=newTemp();
    translate_Exp(args->children,t1);
    arg_list a=(arg_list)malloc(sizeof(struct args));
    if(strcmp(args->children->exp_type->u.basic,"int")==0){
        strcpy(a->name,t1);
    }
    else{
        if(t1[0]=='*') sprintf(a->name,"%s",t1+1);
        else strcpy(a->name,t1);
    } 
    a->next=al->next;
    al->next=a;
    if(args->children->next!=NULL){       
        translate_Args(args->children->next->next,al);
    }
}

char* newTemp(){
    char* t=(char*)malloc(sizeof(char)*40);
    sprintf(t,"t%d",tmp_num++);
    return t;
}

char* newLabel(){
    char* l=(char*)malloc(sizeof(char)*40);
    sprintf(l,"label%d",label_num++);
    return l;
}

int findOffset(Type t,char* name){
    int offset=0;
    FieldList f=findStructWithName(t->u.basic)->t;
    while(f!=NULL){
        if(strcmp(f->name,name)==0){
            return offset;
        }
        offset+=calculateVarSize(f->type);
        f=f->tail;
    }
    printf("can't find struct!\n");
}