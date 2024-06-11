#ifndef MIPS_H
#define MIPS_H

void localVarRecord();
void mips(char* filename);
void printData();
void printGlobl();
void printText(char* filename);
void pushLocalVarToStack(char* name);
int calOffset(char* var);
void loadToReg(char* var,char* reg);

#endif