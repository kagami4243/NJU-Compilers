#include<stdio.h>
int error=0;
FILE* output;
int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "rb");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    output=fopen(argv[2],"w");
    yyrestart(f);
    yyparse();
    if(error==1) return 0;
    semantics();
    if(error==1) return 0;
    translate_Program();
    fclose(output);
    if(error==1){
        output=fopen(argv[2],"w");
        fclose(output);
        return 0;
    } 
    return 0;
}

void set_error(){
    error=1;
}
