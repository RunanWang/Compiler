#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SemaAnalyze.h"
#include "WordAnalyze.h"
#include "VarList.h"
#include "GrammarAnalyze.h"
#include "MidToMIPS.h"
#include "Optimize.h"

int Opregtemp[10] = { 0 };
int Opreplace = 0;
int Optempsp[512] = { 0 };
int Optorecover[10] = { 0 };
int Opblock[256] = { 0 };
int Opblockpos = 0;
typedef struct varnode {
	int varno;
	int part;
	int temp;
}varnode;

varnode Opvartotemp[MAXTEMPVAR];

int Optimize() {
	printf("____Optimizing...____\n");
	Op_MidProcess();
	Op_printMIPS();
	Op_MidPrint();
	printf("____Optimize Done!____\n\n");
	return 0;
}

int Op_printMIPS() {
	FILE*fp = NULL;
	int i = 0;
	int j = 0;
	int k = 1;
	int tr = 0;
	int tn1 = 0;
	int tn2 = 0;
	int paranum = 0;
	int paranumi = 0;
	int funnum = 0;
	int funsp = 0;
	char name[32] = { 0 };
	int functnum[32] = { 0 };
	fp = fopen("Result5_MIPSop.txt", "a");
	printf("    Allocating the regs...\n");
	for (i = 0; i < midresultpos; i++) {
		paranum = 0;
		paranumi = 0;
		if (midResult[i].op == SAFUNCDEFOP) {
			fprintf(fp, "\nfunc%s:\n", VarListPart[midResult[i].num1][0].name);
			funnum = midResult[i].num1;
			j = 1;
			//先在栈中开参数、局部变量
			while (strcmp(VarListPart[funnum][j].name, "")) {
				if (VarListPart[funnum][j].cla == PARAEX) {
					paranum++;
				}
				if (VarListPart[funnum][j].cla == VAREX) {
					fprintf(fp, "addi $sp, $sp, -4\n");
				}
				if (VarListPart[funnum][j].cla == CONSTANTEX) {
					if (VarListPart[funnum][j].type == INTSY) {
						fprintf(fp, "li $t1, %d\n", VarListPart[funnum][j].num);
					}
					else {
						fprintf(fp, "li $t1, %d\n", (int)VarListPart[funnum][j].ch);
					}
					//fprintf(fp, "sw $t1, 0($sp)\n");
					fprintf(fp, "sw $t1, -%d($fp)\n", VarListPart[funnum][j].memposbegin);
					fprintf(fp, "addi $sp, $sp, -4\n");
				}
				if (VarListPart[funnum][j].cla == ARRAYEX) {
					fprintf(fp, "addi $sp, $sp, -%d\n", 4 * VarListPart[funnum][j].num);
				}
				j++;
			}
			while (paranum != 0) {
				fprintf(fp, "lw $t1, %d($fp)\n", 4 * paranum + 8);
				fprintf(fp, "sw $t1, -%d($fp)\n", 4 * paranumi);
				paranum--;
				paranumi++;
			}
			j--;
			funsp = VarListPart[funnum][j].mempos;
			j = i + 1;
			//统计一共要开多大的临时变量区
			while (midResult[j].op != SAFUNCDEFOP && j<midresultpos) {
				if (midResult[j].op >= SAFACCHOP&&midResult[j].op <= SAEXPSUBOP) {
					functnum[funnum]++;
					Optempsp[midResult[j].result] = funsp;
					funsp = funsp + 4;
					//k++;
				}
				if (midResult[j].op == SACALLOP) {
					functnum[funnum]++;
					//Optempsp[k] = funsp;
					Optempsp[midResult[j].result] = funsp;
					funsp = funsp + 4;
					//k++;
				}
				j++;
			}
			fprintf(fp, "addi $sp, $sp, -%d\n", 4 * functnum[funnum]);
		}
		if (midResult[i].op == SAFACCHOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "li $t%d, %d\n", tr, midResult[i].num1);
		}
		if (midResult[i].op == SAFACNUMOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "li $t%d, %d\n", tr, midResult[i].num1);
		}
		if (midResult[i].op == SAFACEXPOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "addi $t%d, $t%d, 0\n", tr, tn1);
		}
		if (midResult[i].op == SAFACAROP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn2 = Op_regold(midResult[i].num2, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].num1].name);
				fprintf(fp, "sll $v1, $t%d, 2\n", tn2);
				fprintf(fp, "lw $t%d, %s($v1)\n", tr, name);
			}
			else {
				fprintf(fp, "sll $v1, $t%d, 2\n",  tn2);
				fprintf(fp, "addi $v1,$v1,%d\n",VarListPart[funnum][midResult[i].num1].memposbegin);
				fprintf(fp, "sub $v1, $fp, $v1\n");
				fprintf(fp, "lw $t%d, ($v1)\n",tr);
			}
		}
		if (midResult[i].op == SAFACVAROP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].num1].name);
				fprintf(fp, "lw $t%d, %s\n",tr, name);
			}
			else {
				fprintf(fp, "lw $t%d, -%d($fp)\n",tr, VarListPart[funnum][midResult[i].num1].memposbegin);
			}
		}
		if (midResult[i].op == SAFUNCOVEROP) {
			fprintf(fp, "jr $ra\n");
		}

		if (midResult[i].op == SATERMMOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			tn2 = Op_regold(midResult[i].num2, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "mult $t%d, $t%d\n", tn1, tn2);
			fprintf(fp, "mflo $t%d\n",tr);
		}
		if (midResult[i].op == SATERMDOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			tn2 = Op_regold(midResult[i].num2, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "div $t%d, $t%d\n",tn1,tn2);
			fprintf(fp, "mflo $t%d\n",tr);
		}

		if (midResult[i].op == SAEXPOPPOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "sub $t%d, $0 ,$t%d\n",tr, tn1);
		}
		if (midResult[i].op == SAEXPADDOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			tn2 = Op_regold(midResult[i].num2, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "add $t%d, $t%d, $t%d\n",tr, tn1, tn2);
		}
		if (midResult[i].op == SAEXPSUBOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			tn2 = Op_regold(midResult[i].num2, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "sub $t%d, $t%d, $t%d\n",tr, tn1, tn2);
		}
		if (midResult[i].op == SAADDIOP) {
			fclose(fp);
			tr = Op_regnew(midResult[i].result, i);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "addi $t%d, $t%d, %d\n", tr, tn1, midResult[i].num2);
		}

		if (midResult[i].op == SACONDOP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			if (midResult[i].num2 == 0) {
				if (midResult[i + 1].op == SABZOP) {
					if (midResult[i].result == MORESY) {
						fprintf(fp, "beq $t%d, $0, Label%d\n",tn1, midResult[i + 1].num1);
					}
				}
				if (midResult[i + 1].op == SABNZOP) {
					if (midResult[i].result == MORESY) {
						fprintf(fp, "bne $t%d, $0, Label%d\n",tn1, midResult[i + 1].num1);
					}
				}
			}
			else {
				fclose(fp);
				tn2 = Op_regold(midResult[i].num2, i);
				fp = fopen("Result5_MIPSop.txt", "a");
				if (midResult[i + 1].op == SABZOP) {
					if (midResult[i].result == EQSY) {
						fprintf(fp, "bne $t%d, $t%d, Label%d\n",tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == NEQSY) {
						fprintf(fp, "beq $t%d, $t%d, Label%d\n",tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == MORESY) {
						fprintf(fp, "ble $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == MOREEQSY) {
						fprintf(fp, "blt $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSSY) {
						fprintf(fp, "bge $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSEQSY) {
						fprintf(fp, "bgt $t%d, $t%d, Label%d\n",tn1, tn2, midResult[i + 1].num1);
					}
				}
				if (midResult[i + 1].op == SABNZOP) {
					if (midResult[i].result == EQSY) {
						fprintf(fp, "beq $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == NEQSY) {
						fprintf(fp, "bne $t%d, $t%d, Label%d\n",tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == MORESY) {
						fprintf(fp, "bgt $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == MOREEQSY) {
						fprintf(fp, "bge $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSSY) {
						fprintf(fp, "blt $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSEQSY) {
						fprintf(fp, "ble $t%d, $t%d, Label%d\n", tn1, tn2, midResult[i + 1].num1);
					}
				}
			}
		}
		if (midResult[i].op == SAGOTOOP) {
			fprintf(fp, "j Label%d\n", midResult[i].num1);
		}
		if (midResult[i].op == SACASEOP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "sw $t%d, -%d($fp)\n", tn1, Optempsp[-midResult[i].num1]);
			fprintf(fp, "bne $t%d, %d, Label%d\n", tn1, midResult[i].num2, midResult[i].result);
		}
		if (midResult[i].op == SADOOP) {
			fprintf(fp, "Label%d:\n", midResult[i].num1);
		}
		if (midResult[i].op == SALABELOP) {
			fprintf(fp, "Label%d:\n", midResult[i].num1);
		}

		if (midResult[i].op == SAASSIGNVAROP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].result].name);
				fprintf(fp, "sw $t%d, %s\n", tn1, name);
			}
			else {
				fprintf(fp, "sw $t%d, -%d($fp)\n", tn1, VarListPart[funnum][midResult[i].result].memposbegin);
			}
		}
		if (midResult[i].op == SAASSIGNARROP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			tn2 = Op_regold(midResult[i].num2, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].result].name);
				fprintf(fp, "sll $v1, $t%d, 2\n", tn1);
				fprintf(fp, "sw $t%d, %s($v1)\n", tn2, name);
			}
			else {
				fprintf(fp, "sll $v1, $t%d, 2\n", tn1);
				fprintf(fp, "addi $v1, $v1, %d\n", VarListPart[funnum][midResult[i].result].memposbegin);
				fprintf(fp, "sub $v1, $fp, $v1\n");
				fprintf(fp, "sw $t%d, ($v1)\n",tn2);
			}
		}

		if (midResult[i].op == SAPRINTFSTROP) {
			fprintf(fp, "li $v0, 4\n");
			fprintf(fp, "la $a0, str%d\n", midResult[i].num1);
			fprintf(fp, "syscall\n");
		}
		if (midResult[i].op == SAPRINTFREGOP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			if (midResult[i].num2 == 0) {
				fprintf(fp, "li $v0, 1\n");
			}
			else {
				fprintf(fp, "li $v0, 11\n");
			}
			fprintf(fp, "addi $a0, $t%d, 0\n", tn1);
			fprintf(fp, "syscall\n");
		}
		if (midResult[i].op == SAPRINTFIDOP) {
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].num1].name);
				if (VarList[midResult[i].num1].cla == CONSTANTEX) {
					fprintf(fp, "li $a0 %d\n", VarList[midResult[i].num1].num);
				}
				else {
					fprintf(fp, "lw $a0 %s\n", name);
				}
				if (VarList[midResult[i].num1].type == CHARSY) {
					fprintf(fp, "li $v0, 11\n");
				}
				else {
					fprintf(fp, "li $v0, 1\n");
				}
				fprintf(fp, "syscall\n");
			}
			else {
				fprintf(fp, "lw $a0 -%d($fp)\n", VarListPart[funnum][midResult[i].num1].memposbegin);
				if (VarListPart[funnum][midResult[i].num1].type == CHARSY) {

					fprintf(fp, "li $v0, 11\n");
				}
				else {
					fprintf(fp, "li $v0, 1\n");
				}
				fprintf(fp, "syscall\n");

			}

		}
		if (midResult[i].op == SASCANFOP) {
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].num1].name);
				if (VarList[midResult[i].num1].type == CHARSY) {
					fprintf(fp, "li $v0, 12\n");
				}
				else {
					fprintf(fp, "li $v0, 5\n");
				}
				fprintf(fp, "syscall\n");
				fprintf(fp, "sw $v0 %s\n", name);
			}
			else {
				if (VarListPart[funnum][midResult[i].num1].type == CHARSY) {
					fprintf(fp, "li $v0, 12\n");
				}
				else {
					fprintf(fp, "li $v0, 5\n");
				}
				fprintf(fp, "syscall\n");
				fprintf(fp, "sw $v0 -%d($fp)\n", VarListPart[funnum][midResult[i].num1].memposbegin);
			}

		}

		if (midResult[i].op == SACALLOP) {
			fclose(fp);
			Op_regsave(i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "sw $ra, ($sp)\n");
			fprintf(fp, "addi $sp $sp -4\n");
			fprintf(fp, "sw $fp, ($sp)\n");
			fprintf(fp, "addi $sp, $sp, -4\n");
			fprintf(fp, "move $fp, $sp\n");
			fprintf(fp, "jal func%s\n", VarListPart[midResult[i].num1][0].name);
			fprintf(fp, "lw $ra, 8($fp)\n");
			fprintf(fp, "move $sp, $fp\n");
			fprintf(fp, "lw $fp, 4($fp)\n");
			fprintf(fp, "addi $sp, $sp, 8\n");
			fclose(fp);
			Op_regrecover();
			tr = Op_regnew(midResult[i].result, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "addi $t%d, $a0, 0\n", tr);
		}
		if (midResult[i].op == SANORETCALLOP) {
			fclose(fp);
			Op_regsave(i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "sw $ra, ($sp)\n");
			fprintf(fp, "addi $sp $sp -4\n");
			fprintf(fp, "sw $fp, ($sp)\n");
			fprintf(fp, "addi $sp, $sp, -4\n");
			fprintf(fp, "move $fp, $sp\n");
			fprintf(fp, "jal func%s\n", VarListPart[midResult[i].num1][0].name);
			fprintf(fp, "lw $ra, 8($fp)\n");
			fprintf(fp, "move $sp, $fp\n");
			fprintf(fp, "lw $fp, 4($fp)\n");
			fprintf(fp, "addi $sp, $sp, 8\n");
			fclose(fp);
			Op_regrecover();
			fp = fopen("Result5_MIPSop.txt", "a");
		}
		if (midResult[i].op == SAPUSHOP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "sw $t%d, ($sp)\n", tn1);
			fprintf(fp, "addi $sp, $sp, -4\n");
		}

		if (midResult[i].op == SARETOP) {
			fclose(fp);
			tn1 = Op_regold(midResult[i].num1, i);
			fp = fopen("Result5_MIPSop.txt", "a");
			fprintf(fp, "addi $a0, $t%d, 0\n", tn1);
			fprintf(fp, "jr $ra\n");
		}
		if (midResult[i].op == SANORETOP) {
			fprintf(fp, "jr $ra\n");
		}
	}
	fclose(fp);
	printf("    Registors are allocated!\n");
	return 0;
}

int Op_findreg(int t) {
	int i = 0;
	for (i = 0; i < 10; i++) {
		if (Opregtemp[i] == t) {
			return i;
		}
		else{}
	}
	return -1;
}

int Op_regold(int t, int midpos) {
	int r = 0;
	int i = midpos;
	FILE*fp = NULL;
	r = Op_findreg(-t);
	if (r == -1) {
		Opreplace = (Opreplace + 1) % 10;
		if (Opregtemp[Opreplace] != 0) {
			while (midResult[i].op != SAFUNCOVEROP) {
				if (-midResult[i].num1 == Opregtemp[Opreplace]|| -midResult[i].num2 == Opregtemp[Opreplace]) {
					fp = fopen("Result5_MIPSop.txt", "a");
					fprintf(fp, "sw $t%d, -%d($fp)\n", Opreplace, Optempsp[Opregtemp[Opreplace]]);
					fprintf(fp, "lw $t%d, -%d($fp)\n", Opreplace, Optempsp[-t]);
					fclose(fp);
					Opregtemp[Opreplace] = -t;
					return Opreplace;
				}
				else {
					fp = fopen("Result5_MIPSop.txt", "a");
					fprintf(fp, "lw $t%d, -%d($fp)\n", Opreplace, Optempsp[-t]);
					fclose(fp);
					Opregtemp[Opreplace] = -t;
					return Opreplace;
				}
				i++;
			}
		}
		Opregtemp[Opreplace] = -t;
		return Opreplace;
	}
	else {
		return r;
	}
}

int Op_regnew(int t, int midpos) {
	int i = midpos;
	FILE*fp = NULL;
	Opreplace = (Opreplace + 1) % 10;
	if (Opregtemp[Opreplace] != 0) {
		while (midResult[i].op != SAFUNCOVEROP) {
			if (-midResult[i].num1 == Opregtemp[Opreplace]|| -midResult[i].num2 == Opregtemp[Opreplace]) {
				fp = fopen("Result5_MIPSop.txt", "a");
				fprintf(fp, "sw $t%d, -%d($fp)\n", Opreplace, Optempsp[Opregtemp[Opreplace]]);
				fclose(fp);
				Opregtemp[Opreplace] = t;
				return Opreplace;
			}
			i++;
		}
	}
	Opregtemp[Opreplace] = t;
	return Opreplace;
}

int Op_regsave(int midpos) {
	int i = midpos;
	int j = 0;
	FILE*fp = NULL;
	for (j = 0; j < 10; j++) {
		i = midpos;
		while (midResult[i].op != SAFUNCOVEROP) {
			if ((-midResult[i].num1 == Opregtemp[j]|| -midResult[i].num2 == Opregtemp[j])&&Opregtemp[j]!=0) {
				fp = fopen("Result5_MIPSop.txt", "a");
				fprintf(fp, "sw $t%d, -%d($fp)\n", j, Optempsp[Opregtemp[j]]);
				Optorecover[j] = Opregtemp[j];
				fclose(fp);
				break;
			}
			i++;
		}
	}
	return 0;
}

int Op_regrecover() {
	int j = 0;
	FILE*fp = NULL;
	for (j = 0; j < 10; j++) {
			if (Optorecover[j]!=0) {
				fp = fopen("Result5_MIPSop.txt", "a");
				fprintf(fp, "lw $t%d, -%d($fp)\n", j, Optempsp[Optorecover[j]]);
				Opregtemp[j]=Optorecover[j]  ;
				Optorecover[j] = 0;
				fclose(fp);
			}
		}
	return 0;
}

int Op_MidProcess() {
	int i = 0;
	midResult[midresultpos].op = SAFUNCOVEROP;
	midresultpos++;
	
	printf("    Algebraic Optimizing...\n");
	for (i = 0; i < midresultpos; i++) {
		if (midResult[i].op == SAFACVAROP&&midResult[i + 1].op == SAFACNUMOP && (midResult[i + 2].op == SAEXPADDOP)) {
			midResult[i + 1].op = SANOPOP;
			midResult[i + 2].op = SAADDIOP;
			midResult[i + 2].num2 = midResult[i + 1].num1;
		}
	}
	printf("    Algebraic Optimizing Done!\n");

	printf("    Dividng into blocks...\n");
	for (i = 0; i < midresultpos; i++) {
		if (i == 0) {
			Opblock[Opblockpos] = i;
			Opblockpos++;
		}
		else if (midResult[i].op == SAFUNCDEFOP || midResult[i].op == SADOOP || midResult[i].op == SALABELOP) {
			if (Opblock[Opblockpos - 1] != i) {
				Opblock[Opblockpos] = i;
				Opblockpos++;
			}
		}
		else if (midResult[i].op == SABZOP|| midResult[i].op == SABNZOP|| midResult[i].op == SAGOTOOP|| midResult[i].op == SACASEOP|| midResult[i].op == SACALLOP|| midResult[i].op == SANORETCALLOP) {
			Opblock[Opblockpos] = i + 1;
			Opblockpos++;
		}
	}
	printf("    Blocks get!\n");

	printf("    DAG works...\n");
	Op_DAG();
	printf("    DAG Done!\n");

	for (i = 0; i < midresultpos; i++) {
		if (midResult[i].op == SAFACEXPOP|| midResult[i].op == SAADDIOP|| midResult[i].op == SACASEOP|| midResult[i].op == SAASSIGNVAROP|| midResult[i].op == SAPRINTFREGOP) {
			midResult[i].num1 = -midResult[i].num1;
		}
		if (midResult[i].op == SAPUSHOP|| midResult[i].op == SARETOP) {
			midResult[i].num1 = -midResult[i].num1;
		}
		if (midResult[i].op == SAFACAROP) {
			midResult[i].num2 = -midResult[i].num2;
		}
		if (midResult[i].op >= SATERMMOP&&midResult[i].op<=SACONDOP|| midResult[i].op == SAASSIGNARROP) {
			midResult[i].num1 = -midResult[i].num1;
			midResult[i].num2 = -midResult[i].num2;
		}
	}
	return 0;
}

int Op_MidPrint() {
	int i = 0;
	int j = 0;
	int funcnum = 0;
	FILE* fpmid = NULL;
	fpmid = fopen("Result6_MidOp.txt", "w");
	for (i = 0; i < midresultpos; i++) {
		if (Opblock[j] == i) {
			fprintf(fpmid, "\n__BLOCK%d__\n", j);
			j++;
		}
		if (midResult[i].op == SAFUNCDEFOP) {
			if (VarListPart[midResult[i].num1][0].type == CHARSY) {
				fprintf(fpmid, "char ");
			}
			else if (VarListPart[midResult[i].num1][0].type == INTSY) {
				fprintf(fpmid, "int ");
			}
			else {
				fprintf(fpmid, "void ");
			}
			funcnum = midResult[i].num1;
			fprintf(fpmid, "%s()\n", VarListPart[midResult[i].num1][0].name);
		}
		if (midResult[i].op == SAFACCHOP) {
			fprintf(fpmid, "t%d=\'%c\'\n", midResult[i].result, midResult[i].num1);
		}
		if (midResult[i].op == SAFACNUMOP) {
			fprintf(fpmid, "t%d=%d\n", midResult[i].result, midResult[i].num1);
		}
		if (midResult[i].op == SAFACEXPOP) {
			fprintf(fpmid, "t%d=t%d\n", midResult[i].result, -midResult[i].num1);
		}
		if (midResult[i].op == SAFACAROP) {
			if (midResult[i].part == 0) {
				fprintf(fpmid, "t%d=%s[t%d]\n", midResult[i].result, VarListPart[funcnum][midResult[i].num1].name, -midResult[i].num2);
			}
			else {
				fprintf(fpmid, "t%d=%s[t%d]\n", midResult[i].result, VarList[midResult[i].num1].name, -midResult[i].num2);
			}
		}
		if (midResult[i].op == SAFACVAROP) {
			if (midResult[i].part == 0) {
				fprintf(fpmid, "t%d=%s\n", midResult[i].result, VarListPart[funcnum][midResult[i].num1].name);
			}
			else {
				fprintf(fpmid, "t%d=%s\n", midResult[i].result, VarList[midResult[i].num1].name);
			}
		}
		if (midResult[i].op == SAFUNCOVEROP) {
			fprintf(fpmid, "\n");
		}

		if (midResult[i].op == SATERMMOP) {
			fprintf(fpmid, "t%d=t%d*t%d\n", midResult[i].result, -midResult[i].num1, -midResult[i].num2);
		}
		if (midResult[i].op == SATERMDOP) {
			fprintf(fpmid, "t%d=t%d/t%d\n", midResult[i].result, -midResult[i].num1, -midResult[i].num2);
		}

		if (midResult[i].op == SAEXPOPPOP) {
			fprintf(fpmid, "t%d=-t%d\n", midResult[i].result, -midResult[i].num1);
		}
		if (midResult[i].op == SAEXPADDOP) {
			fprintf(fpmid, "t%d=t%d+t%d\n", midResult[i].result, -midResult[i].num1, -midResult[i].num2);
		}
		if (midResult[i].op == SAEXPSUBOP) {
			fprintf(fpmid, "t%d=t%d-t%d\n", midResult[i].result, -midResult[i].num1, -midResult[i].num2);
		}
		if (midResult[i].op == SAADDIOP) {
			fprintf(fpmid, "t%d=t%d+%d\n", midResult[i].result, -midResult[i].num1, midResult[i].num2);
		}

		if (midResult[i].op == SACONDOP) {
			if (midResult[i].result == EQSY) {
				fprintf(fpmid, "t%d==t%d\n", -midResult[i].num1, -midResult[i].num2);
			}
			if (midResult[i].result == NEQSY) {
				fprintf(fpmid, "t%d!=t%d\n", -midResult[i].num1, -midResult[i].num2);
			}
			if (midResult[i].result == MORESY) {
				fprintf(fpmid, "t%d>t%d\n", -midResult[i].num1, -midResult[i].num2);
			}
			if (midResult[i].result == MOREEQSY) {
				fprintf(fpmid, "t%d>=t%d\n", -midResult[i].num1, -midResult[i].num2);
			}
			if (midResult[i].result == LESSSY) {
				fprintf(fpmid, "t%d<t%d\n", -midResult[i].num1, -midResult[i].num2);
			}
			if (midResult[i].result == LESSEQSY) {
				fprintf(fpmid, "t%d<=t%d\n", -midResult[i].num1, -midResult[i].num2);
			}
		}
		if (midResult[i].op == SABZOP) {
			fprintf(fpmid, "BZ LABEL_%d\n", midResult[i].num1);
		}
		if (midResult[i].op == SABNZOP) {
			fprintf(fpmid, "BNZ LABEL_%d\n", midResult[i].num1);
		}
		if (midResult[i].op == SAGOTOOP) {
			fprintf(fpmid, "GOTO LABEL_%d\n", midResult[i].num1);
		}
		if (midResult[i].op == SACASEOP) {
			fprintf(fpmid, "t%d==%d\n", -midResult[i].num1, midResult[i].num2);
			fprintf(fpmid, "BZ LABEL_%d\n", midResult[i].result);
		}
		if (midResult[i].op == SADOOP) {
			fprintf(fpmid, "LABEL_%d:\n", midResult[i].num1);
		}
		if (midResult[i].op == SALABELOP) {
			fprintf(fpmid, "LABEL_%d:\n", midResult[i].num1);
		}

		if (midResult[i].op == SAASSIGNVAROP) {
			if (midResult[i].part == 0) {
				fprintf(fpmid, "%s=t%d\n",VarListPart[funcnum][midResult[i].result].name,  -midResult[i].num1);
			}
			else {
				fprintf(fpmid, "%s=t%d\n", VarList[midResult[i].result].name, -midResult[i].num1);
			}
		}
		if (midResult[i].op == SAASSIGNARROP) {
			if (midResult[i].part == 0) {
				fprintf(fpmid, "%s[t%d]=t%d\n", VarListPart[funcnum][midResult[i].result].name, -midResult[i].num1, -midResult[i].num2);
			}
			else {
				fprintf(fpmid, "%s[t%d]=t%d\n", VarList[midResult[i].result].name, -midResult[i].num1, -midResult[i].num2);
			}
		}

		if (midResult[i].op == SAPRINTFSTROP) {
			fprintf(fpmid, "printf \"%s\"\n", string[midResult[i].num1]);
		}
		if (midResult[i].op == SAPRINTFREGOP) {
			fprintf(fpmid, "printf \"t%d\"\n", -midResult[i].num1);
		}
		if (midResult[i].op == SAPRINTFIDOP) {
			if (midResult[i].part == 0) {
				fprintf(fpmid, "printf \"%s\"\n", VarListPart[funcnum][midResult[i].result].name);
			}
			else {
				fprintf(fpmid, "printf \"%s\"\n", VarList[midResult[i].result].name);
			}
		}
		if (midResult[i].op == SASCANFOP) {
			if (midResult[i].part == 0) {
				fprintf(fpmid, "scanf \"%s\"\n", VarListPart[funcnum][midResult[i].result].name);
			}
			else {
				fprintf(fpmid, "scanf \"%s\"\n", VarList[midResult[i].result].name);
			}
		}

		if (midResult[i].op == SACALLOP) {
			fprintf(fpmid, "call %s\n", VarListPart[midResult[i].num1][0].name);
			fprintf(fpmid, "t%d=RET\n", midResult[i].result);
		}
		if (midResult[i].op == SANORETCALLOP) {
			fprintf(fpmid, "call %s\n", VarListPart[midResult[i].num1][0].name);
		}
		if (midResult[i].op == SAPUSHOP) {
			fprintf(fpmid, "push t%d\n", -midResult[i].num1);
		}

		if (midResult[i].op == SARETOP) {
			fprintf(fpmid, "ret t%d\n", -midResult[i].num1);
		}
		if (midResult[i].op == SANORETOP) {
			fprintf(fpmid, "ret\n");
		}
	}
	return 0;
}

int Op_DAG() {
	int midposi = 0;
	int midposj = 0;
	int midposk = 0;
	int nodeposi = 0;
	int nodeposj = 0;
	int blockposi = 0;
	int tempfind = -1;
	for (midposi = 0; midposi < midresultpos; midposi++) {
		if (Opblock[blockposi] == midposi) {
			Op_DAGinit();
			blockposi++;
		}
		if (midResult[midposi].op == SAFACVAROP) {
			for (nodeposj = 0; nodeposj < nodeposi; nodeposj++) {
				if (midResult[midposi].num1 == Opvartotemp[nodeposj].varno&&Opvartotemp[nodeposj].part == midResult[midposi].part) {
					tempfind = nodeposj;
					break;
				}
				else {
					tempfind = -1;
				}
			}
			if (tempfind == -1) {
				Opvartotemp[nodeposi].part = midResult[midposi].part;
				Opvartotemp[nodeposi].temp = midResult[midposi].result;
				Opvartotemp[nodeposi].varno = midResult[midposi].num1;
				nodeposi++;
			}
			else {
				midResult[midposi].op = SANOPOP;
				for (midposj = midposi; midposj < midresultpos; midposj++) {
					if (midResult[midposj].num1 == midResult[midposi].result) {
						if (midResult[midposj].op == SAFACEXPOP || midResult[midposj].op == SAADDIOP || midResult[midposj].op == SACASEOP || midResult[midposj].op == SAASSIGNVAROP || midResult[midposj].op == SAPRINTFREGOP|| midResult[midposj].op == SAPUSHOP || midResult[midposj].op == SARETOP) {
							midResult[midposj].num1 = Opvartotemp[tempfind].temp;
						}
						if (midResult[midposj].op >= SATERMMOP&&midResult[midposj].op <= SACONDOP || midResult[midposj].op == SAASSIGNARROP) {
							midResult[midposj].num1 = Opvartotemp[tempfind].temp;
						}
					}
					if (midResult[midposj].num2 == midResult[midposi].result) {
						if (midResult[midposj].op == SAFACAROP) {
							midResult[midposj].num2 = Opvartotemp[tempfind].temp;
						}
						if (midResult[midposj].op >= SATERMMOP&&midResult[midposj].op <= SACONDOP || midResult[midposj].op == SAASSIGNARROP) {
							midResult[midposj].num2 = Opvartotemp[tempfind].temp;
						}
					}
				}
			}
		}
		if (midResult[midposi].op == SAASSIGNVAROP) {
			for (nodeposj = 0; nodeposj < nodeposi; nodeposj++) {
				if (midResult[midposi].result == Opvartotemp[nodeposj].varno&&Opvartotemp[nodeposj].part == midResult[midposi].part) {
					tempfind = nodeposj;
					break;
				}
				else {
					tempfind = -1;
				}
			}
			if (tempfind == -1) {
				Opvartotemp[nodeposi].part = midResult[midposi].part;
				Opvartotemp[nodeposi].temp = midResult[midposi].result;
				Opvartotemp[nodeposi].varno = midResult[midposi].num1;
				nodeposi++;
			}
			else {
				Opvartotemp[tempfind].part = midResult[midposi].part;
				Opvartotemp[tempfind].temp = midResult[midposi].num1;
				Opvartotemp[tempfind].varno = midResult[midposi].result;
			}
		}
	}
	for (midposi = 0; midposi < midresultpos; midposi++) {
		if (midResult[midposi].op == SATERMMOP) {
			for (midposj = midposi + 1; midposj < midresultpos; midposj++) {
				if (midResult[midposj].op == SATERMMOP&&midResult[midposj].num1 == midResult[midposi].num1&&midResult[midposj].num2 == midResult[midposi].num2) {
					midResult[midposj].op = SANOPOP;
					for (midposk = midposj; midposk < midresultpos; midposk++) {
						if (midResult[midposk].num1 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACEXPOP || midResult[midposk].op == SAADDIOP || midResult[midposk].op == SACASEOP || midResult[midposk].op == SAASSIGNVAROP || midResult[midposk].op == SAPRINTFREGOP || midResult[midposk].op == SAPUSHOP || midResult[midposk].op == SARETOP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
						}
						if (midResult[midposk].num2 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACAROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
						}
					}
				}
			}
		}
		if (midResult[midposi].op == SATERMDOP) {
			for (midposj = midposi+1; midposj < midresultpos; midposj++) {
				if (midResult[midposj].op == SATERMDOP&&midResult[midposj].num1 == midResult[midposi].num1&&midResult[midposj].num2 == midResult[midposi].num2) {
					midResult[midposj].op = SANOPOP;
					for (midposk = midposj; midposk < midresultpos; midposk++) {
						if (midResult[midposk].num1 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACEXPOP || midResult[midposk].op == SAADDIOP || midResult[midposk].op == SACASEOP || midResult[midposk].op == SAASSIGNVAROP || midResult[midposk].op == SAPRINTFREGOP || midResult[midposk].op == SAPUSHOP || midResult[midposk].op == SARETOP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
						}
						if (midResult[midposk].num2 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACAROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
						}
					}
				}
			}
		}
		if (midResult[midposi].op == SAEXPADDOP) {
			for (midposj = midposi+1; midposj < midresultpos; midposj++) {
				if (midResult[midposj].op == SAEXPADDOP&&midResult[midposj].num1 == midResult[midposi].num1&&midResult[midposj].num2 == midResult[midposi].num2) {
					midResult[midposj].op = SANOPOP;
					for (midposk = midposj; midposk < midresultpos; midposk++) {
						if (midResult[midposk].num1 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACEXPOP || midResult[midposk].op == SAADDIOP || midResult[midposk].op == SACASEOP || midResult[midposk].op == SAASSIGNVAROP || midResult[midposk].op == SAPRINTFREGOP || midResult[midposk].op == SAPUSHOP || midResult[midposk].op == SARETOP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
						}
						if (midResult[midposk].num2 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACAROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
						}
					}
				}
			}
		}
		if (midResult[midposi].op == SAEXPSUBOP) {
			for (midposj = midposi+1; midposj < midresultpos; midposj++) {
				if (midResult[midposj].op == SAEXPSUBOP&&midResult[midposj].num1 == midResult[midposi].num1&&midResult[midposj].num2 == midResult[midposi].num2) {
					midResult[midposj].op = SANOPOP;
					for (midposk = midposj; midposk < midresultpos; midposk++) {
						if (midResult[midposk].num1 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACEXPOP || midResult[midposk].op == SAADDIOP || midResult[midposk].op == SACASEOP || midResult[midposk].op == SAASSIGNVAROP || midResult[midposk].op == SAPRINTFREGOP || midResult[midposk].op == SAPUSHOP || midResult[midposk].op == SARETOP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num1 = midResult[midposi].result;
							}
						}
						if (midResult[midposk].num2 == midResult[midposj].result) {
							if (midResult[midposk].op == SAFACAROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
							if (midResult[midposk].op >= SATERMMOP&&midResult[midposk].op <= SACONDOP || midResult[midposk].op == SAASSIGNARROP) {
								midResult[midposk].num2 = midResult[midposi].result;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
int Op_DAGinit() {
	int i = 0;
	for (i = 0; i < MAXTEMPVAR; i++) {
		Opvartotemp[i].part = 0;
		Opvartotemp[i].temp = 0;
		Opvartotemp[i].varno = 0;
	}
}
