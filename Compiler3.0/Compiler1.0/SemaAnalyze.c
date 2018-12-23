#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SemaAnalyze.h"
#include "WordAnalyze.h"
#include "VarList.h"
#include "GrammarAnalyze.h"

int tempvar_i = 0;
int strposi = 0;
int labelnum = 0;
int midresultpos = 0;
int funcnum = 0;

MidFour midResult[MAXMID];

///////////////////////////////////////////////////////////////////////////
int SA_alloc(int Type, int size, char* name, int funcflag) {
	int r = 0;
	char tt[10] = { 0 };
	if (Type == INTSY) {
		strcpy(tt, "int");
	}
	if (Type == CHARSY) {
		strcpy(tt, "char");
	}
	if (funcflag == 1) {
		if (size > 1) {
			SA_printMid(SAALLOCOP, tt, "ARRAY", name, 0, 0, 0);
		}
		else { 
			SA_printMid(SAALLOCOP, tt, NULL, name, 0, 0, 0); 
		}
		SA_printMIPS(SAALLOCOP, 0, 0, size, name);
	}
	else {
		if (size > 1) {
			SA_printMid(SAALLOCOP, tt, "ARRAY", name, 0, 0, 0);
		}
		else {
			SA_printMid(SAALLOCOP, tt, NULL, name, 0, 0, 0);
		}
		SA_printMIPS(SADATAOP, 0, 0, size, name);
		return 0;
	}
}

int SA_para(int Type, int size, char* name, int funcflag) {
	int r = 0;
	char tt[10] = { 0 };
	if (Type == INTSY) {
		strcpy(tt, "int");
	}
	if (Type == CHARSY) {
		strcpy(tt, "char");
	}
	if (funcflag == 1) {
		SA_printMid(SAPARAOP, tt, NULL, name, 0, 0, 0);
		SA_printMIPS(SAALLOCOP, 0, 0, size, name);
	}
	else {
		SA_printMid(SAPARAOP, tt, NULL, name, 0, 0, 0);
		SA_printMIPS(SADATAOP, 0, 0, size, name);
		return 0;
	}
}

int SA_funcdef(int Type, char* name) {
	char tt[10] = { 0 };
	
	int functablenum = 0;
	if (Type == INTSY) {
		strcpy(tt,"int" );
	}
	if (Type == VOIDSY) {
		strcpy(tt, "void");
	}
	if (Type == MAINSY) {
		strcpy(tt, "void");
	}
	if (Type == CHARSY) {
		strcpy(tt, "char");
	}
	SA_printMid(SAFUNCDEFOP, tt, NULL, name, 0, 0, 0);
	SA_printMIPS(SAFUNCDEFOP, 0, 0, 0, name);
	funcnum = SearchTable(name);
	functablenum = VarList[funcnum].num;
	midResult[midresultpos].op = SAFUNCDEFOP;
	midResult[midresultpos].num1 = functablenum;
	midresultpos++;
}

int SA_funcover(){
	midResult[midresultpos].op = SAFUNCOVEROP;
	midresultpos++;
}
///////////////////////////////////////////////////////////////////////////
int SA_factorCh(char ch) {
	int r = 0;
	char c[2] = { 0 };
	r = SA_tempvar();
	c[0] = ch;
	SA_printMid(SAFACCHOP, NULL, NULL, c, r, 0, 0);
	midResult[midresultpos].op = SAFACCHOP;
	midResult[midresultpos].num1 = (int)ch;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}

int SA_factorNum(int num) {
	int r = SA_tempvar();
	SA_printMid(SAFACNUMOP, NULL, NULL, NULL, num, r, 0);
	midResult[midresultpos].op = SAFACNUMOP;
	midResult[midresultpos].num1 = num;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}

int SA_factorExp(int t) {
	int temp = 0;
	temp = SA_tempvar();
	SA_printMid(SAFACEXPOP, NULL, NULL, NULL, t, temp, 0);
	midResult[midresultpos].op = SAFACEXPOP;
	midResult[midresultpos].num1 = t;
	midResult[midresultpos].result = temp;
	midresultpos++;
	return temp;
}

int SA_factorAR(char* name, int t) {
	int r = SA_tempvar();
	int varno = 0;
	SA_printMid(SAFACAROP, NULL, NULL, name, t, r, 0);
	varno = SearchPartTable(name);
	if (varno == -1) {
		varno = SearchTable(name);
		midResult[midresultpos].part = 1;
	}
	midResult[midresultpos].op = SAFACAROP;
	midResult[midresultpos].num1 = varno;
	midResult[midresultpos].num2 = t;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}

int SA_factorVar(char* name, int varnum) {
	int r = SA_tempvar();
	int varno = 0;
	SA_printMid(SAFACVAROP, NULL, NULL, name, r, 0, 0);
	midResult[midresultpos].op = SAFACVAROP;
	varno = SearchPartTable(name);
	if (varno == -1) {
		varno = SearchTable(name);
		midResult[midresultpos].part = 1;
		if (VarList[varno].cla == CONSTANTEX) {
			if (VarList[varno].type == CHARSY) {
				midResult[midresultpos].op = SAFACCHOP;
				midResult[midresultpos].num1 = VarList[varno].ch;
				midResult[midresultpos].result = r;
				midResult[midresultpos].part = 1;
				midresultpos++;
				return r;
			}
			else if (VarList[varno].type == INTSY) {
				midResult[midresultpos].op = SAFACNUMOP;
				midResult[midresultpos].num1 = VarList[varno].num;
				midResult[midresultpos].result = r;
				midResult[midresultpos].part = 1;
				midresultpos++;
				return r;
			}
		}
	}
	midResult[midresultpos].num1 = varno;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}
////////////////////////////////////////////////////////////////////////////
int SA_termm(int f1, int f2) {
	int r = SA_tempvar();
	SA_printMid(SATERMMOP, NULL, NULL, NULL, f1, f2, r);
	midResult[midresultpos].op = SATERMMOP;
	midResult[midresultpos].num1 = f1;
	midResult[midresultpos].num2 = f2;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}
int SA_termd(int f1, int f2) {
	int r = SA_tempvar();
	SA_printMid(SATERMDOP, NULL, NULL, NULL, f1, f2, r);
	midResult[midresultpos].op = SATERMDOP;
	midResult[midresultpos].num1 = f1;
	midResult[midresultpos].num2 = f2;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}
////////////////////////////////////////////////////////////////////////////
int SA_expopp(int t) {
	int r = SA_tempvar();
	SA_printMid(SAEXPOPPOP, NULL, NULL, NULL, t, 0, r);
	midResult[midresultpos].op = SAEXPOPPOP;
	midResult[midresultpos].num1 = t;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}
int SA_expadd(int t1, int t2) {
	int r = SA_tempvar();
	SA_printMid(SAEXPADDOP, NULL, NULL, NULL, t1, t2, r);
	midResult[midresultpos].op = SAEXPADDOP;
	midResult[midresultpos].num1 = t1;
	midResult[midresultpos].num2 = t2;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}
int SA_expsub(int t1, int t2) {
	int r = SA_tempvar();
	SA_printMid(SAEXPSUBOP, NULL, NULL, NULL, t1, t2, r);
	midResult[midresultpos].op = SAEXPSUBOP;
	midResult[midresultpos].num1 = t1;
	midResult[midresultpos].num2 = t2;
	midResult[midresultpos].result = r;
	midresultpos++;
	return r;
}
///////////////////////////////////////////////////////////////////////////
int SA_condition(int e1, int type, int e2) {
	SA_printMid(SACONDOP, NULL, NULL, NULL, e1, e2, type);
	midResult[midresultpos].op = SACONDOP;
	midResult[midresultpos].num1 = e1;
	midResult[midresultpos].num2 = e2;
	midResult[midresultpos].result = type;
	midresultpos++;
}

int SA_bz() {
	int re = labelnum;
	labelnum++;
	SA_printMid(SABZOP, NULL, NULL, NULL, re, 0, 0);
	midResult[midresultpos].num1 = re;
	midResult[midresultpos].op = SABZOP;
	midresultpos++;
	return re;
}

int SA_case(int re1, int re2) {
	int re = labelnum;
	labelnum++;
	SA_printMid(SACASEOP, NULL, NULL, NULL,  re1, re2, re);
	midResult[midresultpos].num1 = re1;
	midResult[midresultpos].num2 = re2;
	midResult[midresultpos].result = re;
	midResult[midresultpos].op = SACASEOP;
	midresultpos++;
	return re;
}

int SA_bnz(int l) {
	SA_printMid(SABNZOP, NULL, NULL, NULL, l, 0, 0);
	midResult[midresultpos].num1 = l;
	midResult[midresultpos].op = SABNZOP;
	midresultpos++;
}

int SA_goto() {
	int re = labelnum;
	labelnum++;
	SA_printMid(SAGOTOOP, NULL, NULL, NULL, re, 0, 0);
	midResult[midresultpos].num1 = re;
	midResult[midresultpos].op = SAGOTOOP;
	midresultpos++;
	return re;
}

int SA_gotoswend(int l) {
	SA_printMid(SAGOTOOP, NULL, NULL, NULL, l, 0, 0);
	midResult[midresultpos].num1 = l;
	midResult[midresultpos].op = SAGOTOOP;
	midresultpos++;
}

int SA_do() {
	int re = labelnum;
	labelnum++;
	SA_printMid(SADOOP, NULL, NULL, NULL, re, 0, 0);
	midResult[midresultpos].num1 = re;
	midResult[midresultpos].op = SADOOP;
	midresultpos++;
	return re;
}

int SA_label(int l) {
	SA_printMid(SALABELOP, NULL, NULL, NULL, l, 0, 0);
	midResult[midresultpos].num1 = l;
	midResult[midresultpos].op = SALABELOP;
	midresultpos++;
}

int SA_assignarr(char* name, int re1, int re2) {
	int varno = 0;
	SA_printMid(SAASSIGNARROP, NULL, NULL, name, re1, re2, 0);
	midResult[midresultpos].num1 = re1;
	midResult[midresultpos].num2 = re2;
	varno = SearchPartTable(name);
	if (varno == -1) {
		varno = SearchTable(name);
		midResult[midresultpos].part = 1;
	}
	midResult[midresultpos].result = varno;
	midResult[midresultpos].op = SAASSIGNARROP;
	midresultpos++;
}

int SA_assignvar(char* name, int re2) {
	int varno = 0;
	SA_printMid(SAASSIGNVAROP, NULL, NULL, name, re2, 0, 0);
	midResult[midresultpos].num1 = re2;
	varno = SearchPartTable(name);
	if (varno == -1) {
		varno = SearchTable(name);
		if (varno == -1) {
			printf("Grammar Analyze Error: Assign var Error: The ID %s in assign is not exist!\n", name);
		}
		midResult[midresultpos].part = 1;
	}
	midResult[midresultpos].result = varno;
	midResult[midresultpos].op = SAASSIGNVAROP;
	midresultpos++;
}
////////////////////////////////////////////////////////////////////////////
int SA_printfstr() {
	SA_printMid(SAPRINTFSTROP, NULL, NULL, string[strposi], 0, 0, 0);
	midResult[midresultpos].num1 = strposi;
	midResult[midresultpos].op = SAPRINTFSTROP;
	midresultpos++;
	strposi++;
}

int SA_printfreg(int re, int flag) {
	SA_printMid(SAPRINTFREGOP, NULL, NULL, NULL, re, 0, 0);
	midResult[midresultpos].num1 = re;
	midResult[midresultpos].op = SAPRINTFREGOP;
	midResult[midresultpos].num2 = flag;
	midresultpos++;
}

int SA_printfid(char* GrIden) {
	int varno = 0;
	SA_printMid(SAPRINTFIDOP, NULL, NULL, GrIden, 0, 0, 0);
	varno = SearchPartTable(GrIden);
	if (varno == -1) {
		varno = SearchTable(GrIden);
		if (varno == -1) {
			printf("Grammar Analyze Error: Printf Error: The ID %s in printf is not exist!\n", GrIden);
		}
		midResult[midresultpos].part = 1;
		midResult[midresultpos].num2 = VarList[varno].type;
	}
	else {
		midResult[midresultpos].num2 = VarListPart[funcnum][varno].type;
	}
	midResult[midresultpos].num1 = varno;
	midResult[midresultpos].op = SAPRINTFIDOP;

	midresultpos++;
}

int SA_scanf(char* GrIden) {
	int varno = 0;
	SA_printMid(SASCANFOP, NULL, NULL, GrIden, 0, 0, 0);
	varno = SearchPartTable(GrIden);
	if (varno == -1) {
		varno = SearchTable(GrIden);
		if (varno == -1) {
			printf("Grammar Analyze Error: Scanf Error: The ID %s in scanf is not exist!\n",GrIden);
		}
		midResult[midresultpos].part = 1;
	}
	midResult[midresultpos].num1 = varno;
	midResult[midresultpos].op = SASCANFOP;
	midresultpos++;
}
////////////////////////////////////////////////////////////////////////////
int SA_call(char* name) {
	int funno = 0;
	int r = SA_tempvar();
	SA_printMid(SACALLOP, NULL, NULL, name, r, 0, 0);
	funno = SearchTable(name);
	funno = VarList[funno].num;
	midResult[midresultpos].num1 = funno;
	midResult[midresultpos].result = r;
	midResult[midresultpos].op = SACALLOP;
	midresultpos++;
	return r;
}

int SA_noretcall(char* name) {
	int funno = 0;
	SA_printMid(SANORETCALLOP, NULL, NULL, name, 0, 0, 0);
	funno = SearchTable(name);
	funno = VarList[funno].num;
	midResult[midresultpos].num1 = funno;
	midResult[midresultpos].op = SANORETCALLOP;
	midresultpos++;
}

int SA_pushpara(int t) {
	SA_printMid(SAPUSHOP, NULL, NULL, NULL, t, 0, 0);
	midResult[midresultpos].num1 = t;
	midResult[midresultpos].op = SAPUSHOP;
	midresultpos++;
}
int SA_pushover() {
	midResult[midresultpos].op = SAPUSHOVEROP;
	midresultpos++;
}
///////////////////////////////////////////////////////////////////////////////
int SA_return(int re) {
	SA_printMid(SARETOP, NULL, NULL, NULL, re, 0, 0);
	midResult[midresultpos].num1 = re;
	midResult[midresultpos].op = SARETOP;
	midresultpos++;
}

int SA_noreturn() {
	SA_printMid(SANORETOP, NULL, NULL, NULL, 0, 0, 0);
	midResult[midresultpos].op = SANORETOP;
	midresultpos++;
}
///////////////////////////////////////////////////////////////////////////////
int SA_printMid(int opnum, char* left, char* right, char* answer, int num1, int num2, int num3) {
	FILE*fp = NULL;
	fp = fopen("Result3_Mid.txt", "a");
	if (opnum == SAALLOCOP|| opnum == SADATAOP) {
		if (right != NULL) {
			fprintf(fp, "var %s %s %s\n", left, right, answer);
		}
		else { fprintf(fp, "var %s %s\n", left, answer); }
	}
	if (opnum == SAPARAOP) {
		fprintf(fp, "para %s %s\n", left, answer);
	}
	if (opnum == SAFUNCDEFOP) {
		fprintf(fp, "\n%s %s()\n",left, answer);
	}

	if (opnum == SAFACCHOP) {
		fprintf(fp, "t%d=\'%s\'\n",num1, answer);
	}
	if (opnum == SAFACNUMOP) {
		fprintf(fp, "t%d=%d\n", num2, num1);
	}
	if (opnum == SAFACEXPOP) {
		fprintf(fp, "t%d=t%d\n", num2, num1);
	}
	if (opnum == SAFACAROP) {
		fprintf(fp, "t%d=%s[t%d]\n", num2, answer, num1);
	}
	if (opnum == SAFACVAROP) {
		fprintf(fp, "t%d=%s\n", num1, answer);
	}

	if (opnum == SATERMMOP) {
		fprintf(fp, "t%d=t%d*t%d\n", num3, num1, num2);
	}
	if (opnum == SATERMDOP) {
		fprintf(fp, "t%d=t%d/t%d\n", num3, num1, num2);
	}

	if (opnum == SAEXPOPPOP) {
		fprintf(fp, "t%d=-t%d\n", num3, num1);
	}
	if (opnum == SAEXPADDOP) {
		fprintf(fp, "t%d=t%d+t%d\n", num3, num1, num2);
	}
	if (opnum == SAEXPSUBOP) {
		fprintf(fp, "t%d=t%d-t%d\n", num3, num1, num2);
	}

	if (opnum == SACALLOP) {
		fprintf(fp, "call %s\n", answer);
		fprintf(fp, "t%d=RET\n", num1);
	}
	if (opnum == SANORETCALLOP) {
		fprintf(fp, "call %s\n", answer);
	}
	if (opnum == SAPUSHOP) {
		fprintf(fp, "push t%d\n", num1);
	}

	if (opnum == SACONDOP) {
		if (num3 == NEQSY) {
			fprintf(fp, "t%d!=t%d\n", num1, num2);
		}
		if (num3 == LESSSY) {
			fprintf(fp, "t%d<t%d\n", num1, num2);
		}
		if (num3 == LESSEQSY) {
			fprintf(fp, "t%d<=t%d\n", num1, num2);
		}
		if (num3 == EQSY) {
			fprintf(fp, "t%d==t%d\n", num1, num2);
		}
		if (num3 == MORESY) {
			fprintf(fp, "t%d>t%d\n", num1, num2);
		}
		if (num3 == MOREEQSY) {
			fprintf(fp, "t%d>=t%d\n", num1, num2);
		}
	}
	if (opnum == SABZOP) {
		fprintf(fp, "BZ LABEL_%d\n", num1);
	}
	if (opnum == SABNZOP) {
		fprintf(fp, "BNZ LABEL_%d\n", num1);
	}
	if (opnum == SAGOTOOP) {
		fprintf(fp, "GOTO LABEL_%d\n", num1);
	}
	if (opnum == SALABELOP) {
		fprintf(fp, "LABEL_%d:\n", num1);
	}
	if (opnum == SADOOP) {
		fprintf(fp, "LABEL_%d:\n", num1);
	}
	if (opnum == SACASEOP) {
		fprintf(fp, "t%d==%d\n", num1, num2);
		fprintf(fp, "BZ LABEL_%d\n", num3);
	}

	if (opnum == SAASSIGNARROP) {
		fprintf(fp, "%s[t%d]=t%d\n", answer, num1, num2);
	}
	if (opnum == SAASSIGNVAROP) {
		fprintf(fp, "%s=t%d\n", answer, num1);
	}

	if (opnum == SAPRINTFSTROP) {
		fprintf(fp, "printf \"%s\"\n", answer);
	}
	if (opnum == SAPRINTFREGOP) {
		fprintf(fp, "printf \"t%d\"\n", num1);
	}
	if (opnum == SAPRINTFIDOP) {
		fprintf(fp, "printf \"t%s\"\n", answer);
	}
	if (opnum == SASCANFOP) {
		fprintf(fp, "scanf \"%s\"\n", answer);
	}

	if (opnum == SARETOP) {
		fprintf(fp, "ret t%d\n", num1);
	}
	if (opnum == SANORETOP) {
		fprintf(fp, "ret\n");
	}
	fclose(fp);
}

int SA_printMIPS(int opnum, int left, int right, int answer, char* label) {
	FILE*fp = NULL;
	FILE*fp2 = NULL;
	fp = fopen("Result4_MIPS.txt", "a");
	fp2 = fopen("Result5_MIPSop.txt", "a");
	if (opnum == SADATAOP) {
		fprintf(fp, "%s:.space %d\n", label, answer*4);
		fprintf(fp2, "%s:.space %d\n", label, answer * 4);
	}
	fclose(fp);
	fclose(fp2);
}

int SA_printInit() {
	FILE*fp = NULL;
	fp = fopen("Result3_Mid.txt", "w");
	fprintf(fp,"未经优化的中间代码如下:\n");
	fclose(fp);
	fp = fopen("Result4_MIPS.txt", "w");
	fprintf(fp, ".data\n");
	fclose(fp);
	fp = fopen("Result5_MIPSop.txt", "w");
	fprintf(fp, ".data\n");
	fclose(fp);
}

int SA_printString() {
	int i = 0;
	FILE*fp = NULL;
	fp = fopen("Result4_MIPS.txt", "a");
	for (i = 0; i < stringnum; i++) {
		fprintf(fp, "str%d:.asciiz\"%s\"\n", i, string[i]);
	}
	fprintf(fp, ".text\n");
	fprintf(fp, "addi $fp, $sp, 0\n");
	fprintf(fp, "jal funcmain\n");
	fprintf(fp, "li $v0 10\nsyscall\n");
	fclose(fp);
	fp = fopen("Result5_MIPSop.txt", "a");
	for (i = 0; i < stringnum; i++) {
		fprintf(fp, "str%d:.asciiz\"%s\"\n", i, string[i]);
	}
	fprintf(fp, ".text\n");
	fprintf(fp, "addi $fp, $sp, 0\n");
	fprintf(fp, "jal funcmain\n");
	fprintf(fp, "li $v0 10\nsyscall\n");
	fclose(fp);
}

int SA_tempvar() {
	tempvar_i++;
	return tempvar_i;
}

int SA_initmid() {
	int i = 0;
	for (i = 0; i < MAXMID; i++) {
		midResult[i].num1 = 0;
		midResult[i].num2 = 0;
		midResult[i].op = 0;
		midResult[i].result = 0;
		midResult[i].part = 0;
	}
}

int SA_printMIPSend() {
	FILE*fp = NULL;
	fp = fopen("Result4_MIPS.txt", "a");
	fprintf(fp, "li $v0 10\nsyscall\n");
	fclose(fp);
}