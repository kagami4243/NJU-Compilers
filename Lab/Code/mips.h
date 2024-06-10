#ifndef MIPS_H
#define MIPS_H
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include"semantics.h"
void mips(char* filename);
void printData(char* filename);
void printGlobl();
void printText(char* filename);
int findStackOffset(char* name);   

#endif