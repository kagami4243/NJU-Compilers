#ifndef AST_H
#define AST_H

union Value{
    int v_int;
    float v_float;
    char* v_string;
};
struct Tree
{   
    int type; // 0->no token, 1->int, 2->float, 3->token, 4->id, 5->type, 6->empty, 7->relop
    union Value V;
    int pos;
    struct Tree* children;
    struct Tree* next;
    int leftValue; // 0->false, 1->true
    struct Type_* exp_type; 
    char specifier[40]; 
    struct Type_* ParamDecType;
    struct Type_* ReturnType;
};

struct Tree* root;
struct Tree* newTreeNode(struct Tree* next, struct Tree* children, int pos, int type, char* value);
void set_error();

#endif