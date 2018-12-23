#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "GrammarAnalyze.h"
#include "WordAnalyze.h"
#include "SemaAnalyze.h"
#include "VarList.h"
//����ļ��а���һЩ�Ա�����Ĳ���
int VarListPos = 0;					//ȫ�ֱ�ǰλ��
int VarListPosPart = 0;				//�ֲ���ǰλ��		
int VarListPartNum = -1;
//�������
int InsertTable(int cla, int type, int num, char ch, char* name, int flag, int mempos) {
	//cla������������������������
	//type����char,int
	//��char���ͳ���ʱ��洢ch
	//name�Ǳ���������
	if (flag == 0) {
		if (SearchTable(name) != -1) {
			printf("VarList error: This var has been declared!\n");
		}
		VarList[VarListPos].cla = cla;
		strcpy(VarList[VarListPos].name, name);
		VarList[VarListPos].ch = ch;
		VarList[VarListPos].type = type;
		VarList[VarListPos].num = num;
		VarList[VarListPos].paranum = mempos;
		if (VarListPos == 0) {
			VarList[VarListPos].mempos = 0; 
			VarList[VarListPos].memposbegin = 0;
		}
		else { 
			VarList[VarListPos].memposbegin = VarList[VarListPos -1].mempos;
			if (cla == ARRAYEX) {
				VarList[VarListPos].mempos = VarList[VarListPos - 1].mempos + 4 * num;
			}
			else if (cla == VAREX) { 
				VarList[VarListPos].mempos = VarList[VarListPos - 1].mempos + 4;
			}
			else {
				VarList[VarListPos].mempos = VarList[VarListPos - 1].mempos;
			}
		}
		VarListPos++;
	}
	else {
		if (SearchPartTable(name) != -1) {
			printf("VarList error: This var has been declared!\n");
		}
		VarListPart[VarListPartNum][VarListPosPart].cla = cla;
		strcpy(VarListPart[VarListPartNum][VarListPosPart].name, name);
		VarListPart[VarListPartNum][VarListPosPart].ch = ch;
		VarListPart[VarListPartNum][VarListPosPart].type = type;
		VarListPart[VarListPartNum][VarListPosPart].num = num;
		VarListPart[VarListPartNum][VarListPosPart].memposbegin = VarListPart[VarListPartNum][VarListPosPart].mempos;
		if (VarListPosPart == 0) { 
			VarListPart[VarListPartNum][VarListPosPart].mempos = 0;
			VarListPart[VarListPartNum][VarListPosPart].memposbegin = 0;
		}
		else {
			VarListPart[VarListPartNum][VarListPosPart].memposbegin = VarListPart[VarListPartNum][VarListPosPart - 1].mempos;
			if (cla == ARRAYEX) {
				VarListPart[VarListPartNum][VarListPosPart].mempos = VarListPart[VarListPartNum][VarListPosPart - 1].mempos + 4 * num;
			}
			else if (cla == VAREX||cla==PARAEX||cla==CONSTANTEX) {
				VarListPart[VarListPartNum][VarListPosPart].mempos = VarListPart[VarListPartNum][VarListPosPart - 1].mempos + 4;
			}
			else {
				VarListPart[VarListPartNum][VarListPosPart].mempos = VarListPart[VarListPartNum][VarListPosPart - 1].mempos;
			}
		}
		VarListPosPart++;
	}
	return 0;
}
//��ȫ�ֱ�
int SearchTable(char*name) {
	int i = 0;
	for (i = 0; i < VarListPos; i++) {
		if (!strcmp(name, VarList[i].name)) {
			return i;
		}
	}
	return -1;
}
//��ֲ���
int SearchPartTable(char*name) {
	int i = 0;
	for (i = 0; i < VarListPosPart; i++) {
		if (!strcmp(name, VarListPart[VarListPartNum][i].name)) {
			return i;
		}
	}
	return -1;
}
//��ӡ������
int printTable() {
	FILE* fp = NULL;
	int i, j = 0;
	fp = fopen("Result0_Table.txt", "w");
	fprintf(fp, "the main vars\n");
	for (i = 0; i < VarListPos; i++) {
		fprintf(fp, "%15s%10s\t%10d\t0x%x\t0x%x\n", VarList[i].name, reserch[VarList[i].type-1], VarList[i].ch,VarList[i].memposbegin,VarList[i].mempos);
	}
	for (i = 0; i <= VarListPartNum; i++) {
		fprintf(fp, "\n%d %s\n", i, VarListPart[i][0].name);
		j = 0;
		while (strcmp(VarListPart[i][j].name, "") != 0) {
			fprintf(fp, "%15s%10s\t%d\t0x%x\t0x%x\n", VarListPart[i][j].name, reserch[VarListPart[i][j].type-1], VarListPart[i][j].num, VarListPart[i][j].memposbegin,VarListPart[i][j].mempos);
			j++;
		}
	}
	fclose(fp);
	/*
	fp = fopen("MidFour.txt", "w");
	for (i = 0; i < midresultpos; i++) {
		fprintf(fp, "%d\t%d\t%d\t%d\t\n", midResult[i].op, midResult[i].num1, midResult[i].num2, midResult[i].result);
	}
	fclose(fp);*/
}
