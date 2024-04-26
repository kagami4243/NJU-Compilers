#ifndef IR_H
#define IR_H

typedef struct args* arg_list;
struct args{
    char name[40];
    struct args* next;
};

char* newTemp();
char* newLabel();
int findOffset(Type t,char* name);

#endif