#include "ast.h"

int StructTableUni=0;
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
struct symbol{
    int type; // 0->var, 1->func
    union{
        Type var;
        struct Func* func;
    } val;
    struct symbol* next;
    char name[40];
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

struct TypeTable{
    char name[40];
    FieldList t;
    struct TypeTable*next;
};

void initSymbol();

/*type: 0->var, 1->func*/
struct symbol* findSymbolWithName(char* name, int type);

struct TypeTable* findStructWithName(char*name);

int Compare2Type(Type t1,Type t2);

Type genArrayType(struct Tree*node,char*type_name);

void do_semantics(struct Tree* node);