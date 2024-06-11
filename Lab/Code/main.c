#include<stdio.h>
#include<string.h>
#include"semantics.h"
#include"ir.h"
#include"mips.h"

int error=0;
FILE* output, *mips_out;
int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "rb");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if(argc==2) return 0;
    if(error==1) return 0;
    semantics();
    if(error==1) return 0;
    if(argv[2][strlen(argv[2])-1]=='s'){
        output=fopen("out.ir","w");
        translate_Program();
        fclose(output);
        if(error==1){
            remove("out.ir");
            return 0;
        }
    }
    else{
        output=fopen(argv[2],"w");
        translate_Program();
        fclose(output);
        if(error==1){
            remove(argv[2]);
            return 0;
        }
        return 0;
    }
    mips_out=fopen(argv[2],"w");
    mips(argv[2]);
    fclose(mips_out);
    remove("out.ir");
    return 0;
}

void set_error(){
    error=1;
}
