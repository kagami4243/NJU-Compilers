#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"

extern int StructTableUni; // for anonymous struct
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

/*symbol table item*/
struct symbol{
    int type; // 0->var, 1->func
    union{
        Type var; // var's type
        struct Func* func; // store func info
    } val; // item value
    struct symbol* next;
    char name[40];
    int param; // parameter or not
};

struct Func{
    Type return_type;
    int param_num;
    Type* param_type;
};

struct Array{ 
    Type elem; 
    int size; 
};
struct Type_
{
    enum { BASIC, ARRAY, STRUCTURE } kind;
    union
    {
    // 基本类型
    char basic[40];
    // 数组类型信息包括元素类型与数组大小构成
    struct Array array;
    // 结构体类型信息是一个链表
    FieldList structure;
    } u;
};
struct FieldList_
{
    char name[40]; // 域的名字
    Type type; // 域的类型
    FieldList tail; // 下一个域
};

/*store struct info*/
struct TypeTable{
    char name[40]; 
    FieldList t; // struct content
    struct TypeTable*next;
    int size;
};

/*num: error type, pos: error pos*/
void PrintErrorNum(int num, int pos);

/*basic: type name*/
Type genBasicType(char* basic);

void initSymbol();

void initStruct();

/*name: var name, type:0->var, type_name: type name*/
void addVar2Symbol(char*name, int type,char* type_name);

/*node: vardec array node, type:0->var, type_name: type name*/
char* addArray2Symbol(struct Tree*node,int type,char*type_name);

/*node: vardec array node, type_name: type name*/
Type genArrayType(struct Tree*node,char*type_name);

/*name: func name, type: 1->func, return_type: return value type, param_type: params type*/
void addFunc2Symbol(char*name, int type,Type return_type,int param_num,Type* param_type);

/*name: var or func name, type: 0->var, 1->func*/
struct symbol* findSymbolWithName(char* name, int type);

/*compare 2 types, 1->true, 0->false*/
int Compare2Type(Type t1,Type t2);

/*structspecifier: StructSpecifier node, StructSpecifier->STRUCT OptTag LC DefList RC*/
char* add2StructTable(struct Tree*structspecifier);

/*name: struct name*/
struct TypeTable* findStructWithName(char*name);

/*main function for semantic analysis*/
void do_semantics(struct Tree* node);

int calculateVarSize(Type var);

void setParam(char* name);

#endif