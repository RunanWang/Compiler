#pragma once
#define MAXVARLISTNUM 128
#define MAXTOKEN 64
#define MAXFUNNUM 32

typedef struct VarInfo {
	char name[MAXTOKEN];
	int cla;
	int type;
	int num;
	char ch;
	int mempos;
	int memposbegin;
	int paranum;
}VarInfo;

VarInfo VarList[MAXVARLISTNUM];
VarInfo VarListPart[MAXFUNNUM][MAXVARLISTNUM];
extern int VarListPos;
extern int VarListPosPart;
extern int VarListPartNum;

int SearchPartTable(char*name);
int SearchTable(char*name);
int InsertTable(int cla, int type, int num, char ch, char* name, int flag, int mempos);
int printTable();