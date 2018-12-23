#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SemaAnalyze.h"
#include "WordAnalyze.h"
#include "VarList.h"
#include "GrammarAnalyze.h"
#include "MidToMIPS.h"

int FI_printMIPS() {
	FILE*fp = NULL;
	int i = 0;
	int j = 0;
	int k = 1;
	int paranum = 0;
	int paranumi = 0;
	int funnum = 0;
	int funsp = 0;
	char name[32] = { 0 };
	int tempsp[512] = { 0 };
	int functnum[32] = { 0 };
	
	fp = fopen("Result4_MIPS.txt", "a");
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
					fprintf(fp, "sw $t1, -%d($fp)\n", VarListPart[funnum][j].memposbegin);
					fprintf(fp, "addi $sp, $sp, -4\n");
				}
				if (VarListPart[funnum][j].cla == ARRAYEX) {
					fprintf(fp, "addi $sp, $sp, -%d\n", 4*VarListPart[funnum][j].num);
				}
				j++;
			}
			while (paranum != 0) {
					fprintf(fp, "lw $t1, %d($fp)\n",4*paranum+8);
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
					tempsp[midResult[j].result] = funsp;
					funsp = funsp + 4;
					//k++;
				}
				if (midResult[j].op == SACALLOP) {
					functnum[funnum]++;
					//tempsp[k] = funsp;
					tempsp[midResult[j].result] = funsp;
					funsp = funsp + 4;
					//k++;
				}
				j++;
			}
			fprintf(fp, "addi $sp, $sp, -%d\n",4*functnum[funnum]);
		}
		if (midResult[i].op == SAFACCHOP) {
			fprintf(fp, "li $t1 %d\n", midResult[i].num1);
			fprintf(fp, "sw $t1, -%d($fp)\n",tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SAFACNUMOP) {
			fprintf(fp, "li $t1 %d\n", midResult[i].num1);
			fprintf(fp, "sw $t1, -%d($fp)\n", tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SAFACEXPOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "sw $t1, -%d($fp)\n", tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SAFACAROP) {
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].num1].name);
				fprintf(fp, "lw $t2, -%d($fp)\n" ,tempsp[midResult[i].num2]);
				fprintf(fp, "sll $t2, $t2, 2 \n");
				fprintf(fp, "lw $t1, %s($t2)\n", name);
				fprintf(fp, "sw $t1, -%d($fp)\n", tempsp[midResult[i].result]);
			}
			else {
				fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
				fprintf(fp, "sll $t2, $t2, 2 \n");
				fprintf(fp, "addi $t2,$t2,%d\n", VarListPart[funnum][midResult[i].num1].memposbegin);
				fprintf(fp, "sub $t2, $fp, $t2\n");
				fprintf(fp, "lw $t1, ($t2)\n");
				fprintf(fp, "sw $t1, -%d($fp)\n", tempsp[midResult[i].result]);
			}
		}
		if (midResult[i].op == SAFACVAROP) {
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].num1].name);
				fprintf(fp, "lw $t1, %s\n", name);
				fprintf(fp, "sw $t1, -%d($fp)\n", tempsp[midResult[i].result]);
			}
			else {
				fprintf(fp, "lw $t1, -%d($fp)\n", VarListPart[funnum][midResult[i].num1].memposbegin);
				fprintf(fp, "sw $t1, -%d($fp)\n", tempsp[midResult[i].result]);
			}
		}
		if (midResult[i].op == SAFUNCOVEROP) {
			fprintf(fp, "jr $ra\n");
		}

		if (midResult[i].op == SATERMMOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
			fprintf(fp, "mult $t1 $t2\n");
			fprintf(fp, "mflo $t3\n");
			fprintf(fp, "sw $t3, -%d($fp)\n", tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SATERMDOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
			fprintf(fp, "div $t1 $t2\n");
			fprintf(fp, "mflo $t3\n");
			fprintf(fp, "sw $t3, -%d($fp)\n", tempsp[midResult[i].result]);
		}

		if (midResult[i].op == SAEXPOPPOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "sub $t3 $0 $t1\n");
			fprintf(fp, "sw $t3, -%d($fp)\n", tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SAEXPADDOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
			fprintf(fp, "add $t3 $t1 $t2\n");
			fprintf(fp, "sw $t3, -%d($fp)\n", tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SAEXPSUBOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
			fprintf(fp, "sub $t3 $t1 $t2\n");
			fprintf(fp, "sw $t3, -%d($fp)\n", tempsp[midResult[i].result]);
		}

		if (midResult[i].op == SACONDOP) {
			if (midResult[i].num2 == 0) {
				if (midResult[i + 1].op == SABZOP) {
					if (midResult[i].result == MORESY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "beq $t1, $0, Label%d\n", midResult[i + 1].num1);
					}
				}
				if (midResult[i + 1].op == SABNZOP) {
					if (midResult[i].result == MORESY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "bne $t1, $0, Label%d\n", midResult[i + 1].num1);
					}
				}
			}
			else {
				if (midResult[i + 1].op == SABZOP) {
					if (midResult[i].result == EQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "bne $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == NEQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "beq $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == MORESY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "ble $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == MOREEQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "blt $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "bge $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSEQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "bgt $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
				}
				if (midResult[i + 1].op == SABNZOP) {
					if (midResult[i].result == EQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "beq $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == NEQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "bne $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == MORESY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "bgt $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == MOREEQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "bge $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "blt $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
					if (midResult[i].result == LESSEQSY) {
						fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
						fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
						fprintf(fp, "ble $t1, $t2, Label%d\n", midResult[i + 1].num1);
					}
				}
			}
		}
		if (midResult[i].op == SAGOTOOP) {
			fprintf(fp, "j Label%d\n", midResult[i].num1);
		}
		if (midResult[i].op == SACASEOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "bne $t1, %d, Label%d\n", midResult[i].num2,midResult[i].result);
		}
		if (midResult[i].op == SADOOP) {
			fprintf(fp, "Label%d:\n", midResult[i].num1);
		}
		if (midResult[i].op == SALABELOP) {
			fprintf(fp, "Label%d:\n", midResult[i].num1);
		}

		if (midResult[i].op == SAASSIGNVAROP) {
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].result].name);
				fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
				fprintf(fp, "sw $t1, %s\n",name);
			}
			else {
				fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
				fprintf(fp, "sw $t1, -%d($fp)\n", VarListPart[funnum][midResult[i].result].memposbegin);
			}
		}
		if (midResult[i].op == SAASSIGNARROP) {
			if (midResult[i].part == 1) {
				strcpy(name, VarList[midResult[i].result].name);
				fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
				fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
				fprintf(fp, "sll $t1, $t1, 2 \n");
				fprintf(fp, "sw $t2, %s($t1)\n", name);
			}
			else {
				fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
				fprintf(fp, "lw $t2, -%d($fp)\n", tempsp[midResult[i].num2]);
				fprintf(fp, "sll $t1, $t1, 2 \n");
				fprintf(fp, "addi $t1, $t1, %d\n", VarListPart[funnum][midResult[i].result].memposbegin);
				fprintf(fp, "sub $t1, $fp, $t1\n");
				fprintf(fp, "sw $t2, ($t1)\n");
			}
		}

		if (midResult[i].op == SAPRINTFSTROP) {
			fprintf(fp, "li $v0, 4\n");
			fprintf(fp, "la $a0, str%d\n",midResult[i].num1);
			fprintf(fp, "syscall\n");
		}
		if (midResult[i].op == SAPRINTFREGOP) {
			if (midResult[i].num2 == 0) {
				fprintf(fp, "li $v0, 1\n");
			}
			else {
				fprintf(fp, "li $v0, 11\n");
			}
			fprintf(fp, "lw $a0, -%d($fp)\n", tempsp[midResult[i].num1]);
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
			fprintf(fp, "sw $a0, -%d($fp)\n",tempsp[midResult[i].result]);
		}
		if (midResult[i].op == SANORETCALLOP) {
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
		}
		if (midResult[i].op == SAPUSHOP) {
			fprintf(fp, "lw $t1, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "sw $t1, ($sp)\n");
			fprintf(fp, "addi $sp, $sp, -4\n");
		}

		if (midResult[i].op == SARETOP) {
			fprintf(fp, "lw $a0, -%d($fp)\n", tempsp[midResult[i].num1]);
			fprintf(fp, "jr $ra\n");
		}
		if (midResult[i].op == SANORETOP) {
			
			fprintf(fp, "jr $ra\n");
		}
	}
	fclose(fp);
}
