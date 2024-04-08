#include<stdio.h>
int error=0;

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "rb");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if(error==1) return 0;
    semantics();
    return 0;
}

void set_error(){
    error=1;
}
