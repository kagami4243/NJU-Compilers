#include"mips.h"

extern FILE* mips_out;
static int argnum=0;
static char args[100][100];
struct{
    int size;
    struct {
        char funcName[100];
        char localName[10000][100];
        int localNum;
        int localSize[10000];
    }LV[1000];
}localVar;
static struct symbol* f;
static int pa_index;

void localVarRecord(){
    localVar.size=-1;
    FILE *file;
    char line[100]; 
    file = fopen("out.ir", "r");
    if (file == NULL) {
        printf("cannot open file: out.ir\n");
        return 1;
    }
    char f[100],pa[100][100];
    int pa_num,finish;
    while (fgets(line, sizeof(line), file)){
        if(strstr(line,"FUNCTION")!=NULL){
            int f_len=strlen(line)-12;
            strncpy(f,line+9,f_len);f[f_len]='\0';
            strcpy(localVar.LV[++localVar.size].funcName,f);
            localVar.LV[localVar.size].localNum=-1;
            pa_num=0;
            finish=0;
        }
        else{
            if(strstr(line,"PARAM")!=NULL){
                char x[100];
                int x_len=strlen(line)-7;
                strncpy(x,line+6,x_len);x[x_len]='\0';
                strcpy(pa[pa_num++],x);
                ++localVar.LV[localVar.size].localNum;
            }
            else
            {
                if(!finish){
                    for(int i=pa_num-1;i>=0;--i){
                        strcpy(localVar.LV[localVar.size].localName[pa_num-i-1],pa[i]);
                        localVar.LV[localVar.size].localSize[pa_num-i-1]=4;
                    }
                    finish=1;
                }
                if(strstr(line,":=")!=NULL){
                    char x[100];
                    int x_len=strstr(line,":=")-line-1;
                    if(line[0]=='*'){
                        strncpy(x,line+1,x_len-1);x[x_len-1]='\0';
                    }
                    else{
                        strncpy(x,line,x_len);x[x_len]='\0';
                    }
                    int Num=localVar.LV[localVar.size].localNum,i;
                    for(i=0;i<Num;++i){
                        if(strcmp(localVar.LV[localVar.size].localName[i],x)==0) break;
                    }
                    if(Num<0 || Num==i){
                        strcpy(localVar.LV[localVar.size].localName[++localVar.LV[localVar.size].localNum],x);
                        localVar.LV[localVar.size].localSize[localVar.LV[localVar.size].localNum]=4;
                    } 
                }
                else if(strstr(line,"READ")!=NULL){
                    char x[100];
                    int x_len=strlen(line)-6;
                    strncpy(x,line+5,x_len);x[x_len]='\0';
                    strcpy(localVar.LV[localVar.size].localName[++localVar.LV[localVar.size].localNum],x);
                    localVar.LV[localVar.size].localSize[localVar.LV[localVar.size].localNum]=4;
                }
                else if(strstr(line,"DEC")!=NULL){
                    char x[100],k[100];
                    int x_len=0;
                    while(line[4+x_len]!=' ') ++x_len;
                    int k_len=strlen(line)-x_len-6;
                    strncpy(x,line+4,x_len);x[x_len]='\0';
                    strncpy(k,line+5+x_len,k_len);k[k_len]='\0';
                    strcpy(localVar.LV[localVar.size].localName[++localVar.LV[localVar.size].localNum],x);
                    localVar.LV[localVar.size].localSize[localVar.LV[localVar.size].localNum]=atoi(k);
                }
            } 
        }
    }
}

void mips(char* filename){
    localVarRecord();
    printData(filename);
    printGlobl();
    printText(filename);
}

void printData(char* filename){
    fprintf(mips_out,".data\n");
    fprintf(mips_out,"_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(mips_out,"_ret: .asciiz \"\\n\"\n");
}

void printGlobl(){
    fprintf(mips_out,".globl main\n");
}

void printText(char* filename){
    fprintf(mips_out,".text\n");
    fprintf(mips_out,"read:\nli $v0, 4\nla $a0, _prompt\nsyscall\nli $v0, 5\nsyscall\njr $ra\nwrite:\nli $v0, 1\nsyscall\nli $v0, 4\nla $a0, _ret\nsyscall\nmove $v0, $0\njr $ra\n");
    FILE *file;
    char line[100]; 
    file = fopen("out.ir", "r");
    if (file == NULL) {
        printf("cannot open file: out.ir\n");
        return 1;
    }
    while (fgets(line, sizeof(line), file)) {
        if(strstr(line,"ARG")!=NULL){
            // ARG x
            char x[100];
            int x_len=strlen(line)-5;
            strncpy(x,line+4,x_len);x[x_len]='\0';
            strcpy(args[argnum++],x);
        }
        else if(strstr(line,"CALL")!=NULL){
            // x = CALL f
            int an=argnum;
            char a[100][100];
            memcpy(a,args,sizeof args);
            argnum=0;
            char x[100],f[100];
            int x_len=strstr(line,":=")-line-1,f_len=strlen(line)-x_len-10;
            strncpy(x,line,x_len);x[x_len]='\0';
            strncpy(f,line+x_len+9,f_len);f[f_len]='\0';
            fprintf(mips_out,"addi $sp, $sp, -4\n");
            fprintf(mips_out,"sw $ra, 0($sp)\n");
            fprintf(mips_out,"addi $sp, $sp, -4\n");
            fprintf(mips_out,"sw $fp, 0($sp)\n");
            for(int i=0;i<an;++i){
                loadToReg(a[i],"$t0");
                fprintf(mips_out,"addi $sp, $sp, -4\n");
                fprintf(mips_out,"sw $t0, 0($sp)\n");
            }
            fprintf(mips_out,"addi $fp, $sp, %d\n",4*an);
            if(strcmp(f,"main")==0) fprintf(mips_out,"jal main\n");
            else fprintf(mips_out,"jal _%s\n",f);
            fprintf(mips_out,"move $sp, $fp\n");
            fprintf(mips_out,"lw $fp, 0($sp)\n");
            fprintf(mips_out,"addi $sp, $sp, 4\n");
            fprintf(mips_out,"lw $ra, 0($sp)\n");
            fprintf(mips_out,"addi $sp, $sp, 4\n");
            fprintf(mips_out,"move $t0, $v0\n");
            fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
        }
        else if(strstr(line,"FUNCTION")!=NULL){
            // FUNCTION f:
            char t[100];
            int t_len=strlen(line)-12;
            strncpy(t,line+9,t_len);t[t_len]='\0';
            f=findSymbolWithName(t,1);
            pa_index=0;
            if(strcmp(t,"main")!=0) fprintf(mips_out,"_");
            fprintf(mips_out,"%s:\n",t);
            if(strcmp(t,"main")==0) 
                fprintf(mips_out,"move $fp, $sp\n");
            if(f->val.func->param_num==0){
                pushLocalVarToStack(t);
            }
        }
        else if(strstr(line,"IF")!=NULL){
            // IF x relop y GOTO z
            if(strstr(line,"==")!=NULL){
                char x[100],y[100];
                int x_len=strstr(line,"==")-line-4,y_len=strstr(line,"GOTO")-line-x_len-8;
                strncpy(x,line+3,x_len);
                strncpy(y,line+7+x_len,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(x,"$t0");
                loadToReg(y,"$t1");
                fprintf(mips_out,"beq $t0, $t1, %s",line+13+x_len+y_len); 
            }
            else if(strstr(line,"!=")!=NULL){
                char x[100],y[100];
                int x_len=strstr(line,"!=")-line-4,y_len=strstr(line,"GOTO")-line-x_len-8;
                strncpy(x,line+3,x_len);
                strncpy(y,line+7+x_len,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(x,"$t0");
                loadToReg(y,"$t1");
                fprintf(mips_out,"bne $t0, $t1, %s",line+13+x_len+y_len); 
            }
            else if(strstr(line,">=")!=NULL){
                char x[100],y[100];
                int x_len=strstr(line,">=")-line-4,y_len=strstr(line,"GOTO")-line-x_len-8;
                strncpy(x,line+3,x_len);
                strncpy(y,line+7+x_len,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(x,"$t0");
                loadToReg(y,"$t1");
                fprintf(mips_out,"bge $t0, $t1, %s",line+13+x_len+y_len); 
            }
            else if(strstr(line,"<=")!=NULL){
                char x[100],y[100];
                int x_len=strstr(line,"<=")-line-4,y_len=strstr(line,"GOTO")-line-x_len-8;
                strncpy(x,line+3,x_len);
                strncpy(y,line+7+x_len,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(x,"$t0");
                loadToReg(y,"$t1");
                fprintf(mips_out,"ble $t0, $t1, %s",line+13+x_len+y_len); 
            }
            else if(strstr(line,">")!=NULL){
                char x[100],y[100];
                int x_len=strstr(line,">")-line-4,y_len=strstr(line,"GOTO")-line-x_len-7;
                strncpy(x,line+3,x_len);
                strncpy(y,line+6+x_len,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(x,"$t0");
                loadToReg(y,"$t1");
                fprintf(mips_out,"bgt $t0, $t1, %s",line+12+x_len+y_len); 
            }
            else if(strstr(line,"<")!=NULL){
                char x[100],y[100];
                int x_len=strstr(line,"<")-line-4,y_len=strstr(line,"GOTO")-line-x_len-7;
                strncpy(x,line+3,x_len);
                strncpy(y,line+6+x_len,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(x,"$t0");
                loadToReg(y,"$t1");
                fprintf(mips_out,"blt $t0, $t1, %s",line+12+x_len+y_len); 
            }
        }
        else if(strstr(line,"GOTO")!=NULL){
            // GOTO x
            char t[100];
            strcpy(t,line+5);
            fprintf(mips_out,"j %s",t);
        }
        else if(strstr(line,"LABEL")!=NULL){
            // LABEL x:
            char t[100];
            strcpy(t,line+6);
            fprintf(mips_out,"%s",t);
        }
        else if(strstr(line,"PARAM")!=NULL){
            // PARAM x
            ++pa_index;
            if(pa_index==f->val.func->param_num){
                pushLocalVarToStack(f->name);
            }
        }
        else if(strstr(line,"READ")!=NULL){
            // READ x
            char x[100];
            int x_len=strlen(line)-6;
            strncpy(x,line+5,x_len);
            x[x_len]='\0';
            fprintf(mips_out,"addi $sp, $sp, -4\n");
            fprintf(mips_out,"sw $ra, 0($sp)\n");
            fprintf(mips_out,"jal read\n");
            fprintf(mips_out,"lw $ra, 0($sp)\n");
            fprintf(mips_out,"addi $sp, $sp, 4\n");
            fprintf(mips_out,"move $t0, $v0\n");
            fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
        }
        else if(strstr(line,"RETURN")!=NULL){
            // RETURN x
            char t[100];
            int t_len=strlen(line)-8;
            strncpy(t,line+7,t_len);
            t[t_len]='\0';
            loadToReg(t,"$t0");
            fprintf(mips_out,"move $v0, $t0\n");
            fprintf(mips_out,"jr $ra\n");
        }       
        else if(strstr(line,"WRITE")!=NULL){
            // WRITE x
            char x[100];
            int x_len=strlen(line)-7;
            strncpy(x,line+6,x_len);
            x[x_len]='\0';
            fprintf(mips_out,"addi $sp, $sp, -4\n");
            fprintf(mips_out,"sw $ra, 0($sp)\n");
            loadToReg(x,"$a0");
            fprintf(mips_out,"jal write\n");
            fprintf(mips_out,"lw $ra, 0($sp)\n");
            fprintf(mips_out,"addi $sp, $sp, 4\n");
        }
        else if(strstr(line,"+")!=NULL){
            // x = y + z
            char x[100],y[100],z[100];
            int x_len=strstr(line,":=")-line-1,y_len=strstr(line,"+")-line-5-x_len,z_len=strlen(line)-x_len-y_len-8;
            strncpy(x,line,x_len);
            strncpy(y,line+x_len+4,y_len);
            strncpy(z,line+x_len+y_len+7,z_len);
            x[x_len]='\0';
            y[y_len]='\0';
            z[z_len]='\0';
            loadToReg(y,"$t0");
            loadToReg(z,"$t1");
            fprintf(mips_out,"add $t0, $t0, $t1\n");
            fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
        }
        else if(strstr(line,"-")!=NULL && line[strstr(line,"-")-line+1]==' '){
            // x = y - z
            char x[100],y[100],z[100];
            int x_len=strstr(line,":=")-line-1,y_len=strstr(line,"-")-line-5-x_len,z_len=strlen(line)-x_len-y_len-8;
            strncpy(x,line,x_len);
            strncpy(y,line+x_len+4,y_len);
            strncpy(z,line+x_len+y_len+7,z_len);
            x[x_len]='\0';
            y[y_len]='\0';
            z[z_len]='\0';
            loadToReg(y,"$t0");
            loadToReg(z,"$t1");
            fprintf(mips_out,"sub $t0, $t0, $t1\n");
            fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
        }
        else if(strstr(line,"/")!=NULL){
            // x = y / z
            char x[100],y[100],z[100];
            int x_len=strstr(line,":=")-line-1,y_len=strstr(line,"/")-line-5-x_len,z_len=strlen(line)-x_len-y_len-8;
            strncpy(x,line,x_len);
            strncpy(y,line+x_len+4,y_len);
            strncpy(z,line+x_len+y_len+7,z_len);
            x[x_len]='\0';
            y[y_len]='\0';
            z[z_len]='\0';
            loadToReg(y,"$t0");
            loadToReg(z,"$t1");
            loadToReg(x,"$t2");
            fprintf(mips_out,"div $t0, $t1\n");
            fprintf(mips_out,"mflo $t2\n");
            fprintf(mips_out,"sw $t2, -%d($fp)\n",calOffset(x));
        }
        else if(strstr(line,"&")!=NULL){
            // X = &y
            char x[100],y[100];
            int x_len=strstr(line,":=")-line-1,y_len=strlen(line)-x_len-6;
            strncpy(x,line,x_len);
            strncpy(y,line+x_len+5,y_len);
            x[x_len]='\0';
            y[y_len]='\0';
            fprintf(mips_out,"la $t0, -%d($fp)\n",calOffset(y));
            fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
        }
        else if(strstr(line,"*")!=NULL){
            int index=strstr(line,"*")-line,index2=-1;
            if(strstr(line+index+1,"*")!=NULL) index2=strstr(line+index+1,"*")-line;
            if(index==0){
                // *x = y
                char x[100],y[100];
                int x_len=strstr(line,":=")-line-2,y_len=strlen(line)-x_len-6;
                strncpy(x,line+1,x_len);
                strncpy(y,line+x_len+5,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                fprintf(mips_out,"lw $t0, -%d($fp)\n",calOffset(x));
                loadToReg(y,"$t1");
                fprintf(mips_out,"sw $t1, 0($t0)\n");
            }
            else if(line[index-2]=='=' && index2==-1){
                // x = *y
                char x[100],y[100];
                int x_len=strstr(line,":=")-line-1,y_len=strlen(line)-x_len-6;
                strncpy(x,line,x_len);
                strncpy(y,line+x_len+5,y_len);
                x[x_len]='\0';
                y[y_len]='\0';
                loadToReg(y,"$t1");
                fprintf(mips_out,"lw $t0, 0($t1)\n");
                fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
            }
            else{
                // x = y * z
                char x[100],y[100],z[100];
                int x_len=strstr(line,":=")-line-1,y_len=strstr(line+x_len+5,"*")-line-5-x_len,z_len=strlen(line)-x_len-y_len-8;
                strncpy(x,line,x_len);
                strncpy(y,line+x_len+4,y_len);
                strncpy(z,line+x_len+y_len+7,z_len);
                x[x_len]='\0';
                y[y_len]='\0';
                z[z_len]='\0';
                loadToReg(y,"$t0");
                loadToReg(z,"$t1");
                fprintf(mips_out,"mul $t0, $t0, $t1\n");
                fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
            }
        }
        else if(strstr(line,":=")!=NULL){
            // x = y
            char x[100],y[100];
            int x_len=strstr(line,":=")-line-1,y_len=strlen(line)-x_len-5;
            strncpy(x,line,x_len);x[x_len]='\0';
            strncpy(y,line+x_len+4,y_len);y[y_len]='\0';
            loadToReg(y,"$t0");
            fprintf(mips_out,"sw $t0, -%d($fp)\n",calOffset(x));
        }
    }
    fclose(file);
}

void pushLocalVarToStack(char* name){
    int pushNum=pa_index;
    int index;
    for(int i=0;i<=localVar.size;++i){
        if(strcmp(name,localVar.LV[i].funcName)==0){
            index=i;
            break;
        }
    }
    int Num=localVar.LV[index].localNum;
    for(int j=pa_index;j<=Num;++j){
        fprintf(mips_out,"addi $sp, $sp, -%d\n",localVar.LV[index].localSize[j]);
    }
}

int calOffset(char* var){
    int index;
    for(int i=0;i<=localVar.size;++i){
        if(strcmp(localVar.LV[i].funcName,f->name)==0) index=i;
    }
    int sum=0;
    for(int i=0;i<=localVar.LV[index].localNum;++i){
        sum+=localVar.LV[index].localSize[i];
        if(strcmp(var,localVar.LV[index].localName[i])==0) return sum;
    }
}

void loadToReg(char* var,char* reg){
    if(var[0]=='*'){
        fprintf(mips_out,"lw %s, -%d($fp)\n",reg,calOffset(var+1));
        fprintf(mips_out,"lw %s, 0(%s)\n",reg,reg);
    }
    else if(var[0]=='#'){
        fprintf(mips_out,"li %s, %d\n",reg,atoi(var+1));
    }
    else if(var[0]=='&'){
        fprintf(mips_out,"la %s, -%d($fp)\n",reg,calOffset(var+1));
    }
    else{
        fprintf(mips_out,"lw %s, -%d($fp)\n",reg,calOffset(var));
    }
}