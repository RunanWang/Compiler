#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "GrammarAnalyze.h"
#include "WordAnalyze.h"
#include "SemaAnalyze.h"
#include "VarList.h"
#include "MidToMIPS.h"

int GrWordpos = 0;					//用于记录读到的词的位置
int GrNumber = 0;					//用于传递来自于词法分析的整数（无符号）
char GrIden[MAXTOKEN] = { 0 };		//用于传递token
char GrWordchar = 0;				//用于临时存储char类型变量
int Grnumposi = 0;					//用于记录词法分析得出的num表中读到的数的位置
int Grcharposi = 0;					//用于记录词法分析中char表读到的char的位置
int Grtokenposi = 0;				//用于记录词法分析中得到的ID的位置
FILE* Grfp = NULL;					//用于输出语法分析结果的文件

int funcflag = 0;					//这个flag用于区别是在函数中还是在函数外（用于确定是在全局表还是在局部表插入变量）

//下面这个函数用于从词法分析中获得词。
int GetWord() {
	//由于没有对文法进行改写，使用的是原来的文法，所以这种文法是需要有预读的
	//采取的一种简便的做法是，将词法分析得到的词存入一个数组word中
	//如果是含有信息的词，例如num和string，就将它们单独存入另一个数组中
	//所以，用word[GrWordPos+n]的方式，可以实现预读。
	//而下面的这个函数实现了将word[GrWordPos]的值返回并将GrWordPos++，并返回相应的需要记录的信息
	int GrTempSym = 0;
	GrTempSym = word[GrWordpos];
	//GrTempSym是需要返回的word的类型（具体值见头文件中的定义）
	if (GrTempSym == NUMSY) {
		//如果是NUM类型需要同时读取它的值，词法分析时存在wordToGrammernum的数组中，需要在GetWord的同时读出来。
		GrNumber = wordToGrammernum[Grnumposi];
		Grnumposi++;
	}
	if (GrTempSym == IDSY) {
		//如果是ID类型需要同时读取它的值，词法分析时存在wordToGrammertoken的数组中，需要在GetWord的同时读出来。
		//读到GrIden这个变量中，主要后面使用时可能出现这个变量因为嵌套而被冲掉的情况（比如调用中参数和函数），要注意建立临时变量保存之。
		strcpy(GrIden, wordToGrammertoken[Grtokenposi]);
		Grtokenposi++;
	}
	if (GrTempSym == STRSY) {
		//如果是String类型需要同时读取它的值，词法分析时存在wordToGrammernum的数组中，需要在GetWord的同时读出来。
		//注意这里返回的是存储string的序号
		GrNumber = wordToGrammernum[Grnumposi];
		Grnumposi++;
	}
	if (GrTempSym == CHSY) {
		//如果是CH类型需要同时读取它的值，词法分析时存在wordToGrammernum的数组中，需要在GetWord的同时读出来。
		GrWordchar = wordToGrammerchar[Grcharposi];
		Grcharposi++;
	}
	//读完之后移动当前位置指针（指针指向下一个要读的词）
	GrWordpos++;
	//返回读到的值的类型值
	return GrTempSym;
}
//下面这个函数分析 <程序> 
int GrProgram() {
	//＜程序＞::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
	int GrTempSym = 0;
	//下面是常量
	if (word[GrWordpos] == CONSTSY) {
		//GrTempSym = GetWord();
		GrConstant();
	}
	//下面是变量
	while (word[GrWordpos + 2] != LBRACESY&&word[GrWordpos + 2] != LPARSY) {
		GrVar();
	}
	//下面是函数（根据文法，可以混在一起）
	//先把data段的信息打印出来
	SA_printString();
	//然后遇到的都是函数
	while (word[GrWordpos + 2] == LBRACESY || word[GrWordpos + 2] == LPARSY) {
		if (word[GrWordpos + 1] == MAINSY) {
			break;
		}
		if (word[GrWordpos] == VOIDSY&&word[GrWordpos + 1] != MAINSY) {
			GrNoRetFun();
		}
		else {
			GrRetFun();
		}
	}
	//下面是main函数
	if (word[GrWordpos + 1] == MAINSY) {
		GrMainFun();
		//program结束，输出
		//printf("This is a program.\n");
		fprintf(Grfp, "This is a program.\n");
	}
	else {
		printf("Grammar Analyze Error: There is no main function symbol!\n");
	}
	return 0;
}
//下面这个函数分析 <常量> ，将文法中<常量说明>和<常量定义>整合在了一起
int GrConstant() {
	int GrTempType = 0;
	int IDsy = 0;
	int Int = 0;
	char Char = 0;
	int temp = 0;
	/*
	＜常量说明＞ :: = const＜常量定义＞; { const＜常量定义＞; }
	＜常量定义＞   :: = int＜标识符＞＝＜整数＞{ ,＜标识符＞＝＜整数＞ }| char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
	*/
	do {
		//第一层循环针对常量说明中的{}
		GrTempType = GetWord();
		if (GrTempType == SEMICOLONSY) { GrTempType = GetWord(); }
		//else { printf("Grammar Analyze Error: There should be a ; after each constant declaration!\n"); }
		if (GrTempType == CONSTSY) { 
			if (word[GrWordpos] != INTSY && word[GrWordpos] != CHARSY) {
				printf("Grammar Analyze Error: There should be a char or int announced after each constant!\n");
				GrTempType = NOPSY;
			}
			else {
				GrTempType = GetWord();
			}
		}
		if (GrTempType != INTSY && GrTempType != CHARSY) {
			printf("Grammar Analyze Error: There should be a char or int announced after each constant!\n");
			GrTempType = NOPSY;
		}
		//else { printf("Grammar Analyze Error: There should be a \"const\" symbol after each constant declaration!\n"); }
		//上面三个语句意在读完CONSTSY之后的类型int或者char
		do {
			//第二层循环针对常量定义中的{}
			IDsy = GetWord();
			if (IDsy == COMMASY) { IDsy = GetWord(); }
			if (IDsy != IDSY) { printf("Grammar Analyze Error: There should be a \"ID\" symbol in each constant declaration!\n"); }
			//上面的语句意在读到ID，读不到报错
			//读=
			if (word[GrWordpos] != ASSIGNSY) { 
				printf("Grammar Analyze Error: There should be a \"=\" symbol in each constant declaration!\n"); 
			}
			else {
				GetWord();
			}
			//读立即数
			if (GrTempType == INTSY) {
				Int = GrInteger();
			}
			else if (GrTempType == CHARSY) {
				temp = GetWord();
				if (temp != CHSY) {
					printf("Grammar Analyze Error: There should be a char after an assign of a char constant!\n");
				}
				Char = GrWordchar;
			}
			else {
				printf("Grammar Analyze Error: There should be a char or int assigned for each constant!\n");
			}
			//入表
			InsertTable(CONSTANTEX, GrTempType, Int, Char, GrIden, funcflag, 0);
			//printf("This is a constant: %s\n", GrIden);
			fprintf(Grfp, "This is a constant: %s\n", GrIden);
		} while (word[GrWordpos] == COMMASY);
	} while (word[GrWordpos] == SEMICOLONSY&&word[GrWordpos + 1] == CONSTSY);
	//处理结尾的符号
	if ((word[GrWordpos] == SEMICOLONSY)) { GetWord(); }
	else {
		printf("Grammar Analyze Error: Error in constant: lack semicolon ;!\n");
	}
}
//下面这个函数用于分析 <整数>，用于给词法分析得到的结果加上正负号
int GrInteger() {
	int flag = 1;
	int sym = 0;
	int r = 0;
	if (word[GrWordpos] != NUMSY&&word[GrWordpos] != ADDSY&&word[GrWordpos] != MINUSSY) {
		printf("Grammar Analyze Error: There should be a num in an integer 1.\n");
		return 0;
	}
	sym = GetWord();
	if (sym != NUMSY) {
		if (sym == ADDSY) {
			flag = 1;
		}
		else if (sym == MINUSSY) {
			flag = -1;
		}
		if (word[GrWordpos] != NUMSY) { 
			printf("Grammar Analyze Error: There should be a num in an integer. 2\n");
			return 0;
		}
		else {
			GetWord();
		}
	}
	r = flag*GrNumber;
	return r;
}
//下面这个函数用于分析 <变量>
int GrVar() {
	/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
	＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’) } //＜无符号整数＞表示数组元素的个数，其值需大于0
	＜类型标识符＞      ::=  int | char
	*/
	int GrTempSym = 0;
	int IDsy = 0;
	int GrTempDim = 0;
	int GrTempPos = 0;
	GrTempSym = GetWord();
	do {
		//变量说明的处理在更高一层的处理中，这里只处理变量定义以后的分析
		if ((word[GrWordpos] == COMMASY)) { GetWord(); }
		IDsy = GetWord();
		if (IDsy != IDSY) { printf("Grammar Analyze Error: In Var: this symbol should be an ID, but it is not.\n"); }
		//下面处理数组
		if (word[GrWordpos] == LBRACKETSY) {
			GetWord();
			if (GetWord() != NUMSY) { printf("Grammar Analyze Error: In Array Declare: The array should be followed by a size after its name.\n"); }
			GrTempDim = GrNumber;
			if (GetWord() != RBRACKETSY) { printf("Grammar Analyze Error: In Array Declare: There should be a right bracket after array the size of the array.\n"); }
			GrTempPos = SA_alloc(GrTempSym, GrTempDim, GrIden, funcflag);
			InsertTable(ARRAYEX, GrTempSym, GrTempDim, 0, GrIden, funcflag, GrTempPos);			
			//printf("This is a ArrayVar: %s\n", GrIden);
			fprintf(Grfp, "This is a ArrayVar: %s\n", GrIden);
		}
		else {
			GrTempPos = SA_alloc(GrTempSym, 1, GrIden, funcflag);
			InsertTable(VAREX, GrTempSym, 0, 0, GrIden, funcflag, GrTempPos);
			//printf("This is a Var: %s\n", GrIden);
			fprintf(Grfp, "This is a Var: %s\n", GrIden);
		}
	} while (word[GrWordpos] == COMMASY);
	if ((word[GrWordpos] == SEMICOLONSY)) { GetWord(); }
	else {
		if (word[GrWordpos] == ASSIGNSY) {
			printf("Grammar Analyze Error: In Declaration: There should not be = after declare.\n");
			GetWord();
			GetWord();
		}
		else { printf("Grammar Analyze Error: In Declaration: There should be a ; after each declare.\n"); }
	}
}
//下面这个函数用于分析 <有返回值的函数>
int GrNoRetFun() {
	int GrFunRetType = 0;
	char GrTempIden[MAXTOKEN] = { 0 };
	int paranum = 0;
	int funi = 0;
	funcflag = 1;
	VarListPartNum++;
	VarListPosPart = 0;
	GrFunRetType = GetWord();
	GetWord();
	strcpy(GrTempIden, GrIden);
	printf("    Grammar Analyzing for the function:%s.\n", GrTempIden);
	fprintf(Grfp, "\nThis below is a no return function:%s.\n", GrTempIden);
	InsertTable(FUNEX, GrFunRetType, VarListPartNum, 0, GrTempIden, funcflag, paranum);
	InsertTable(FUNEX, GrFunRetType, VarListPartNum, 0, GrTempIden, 0, paranum);
	SA_funcdef(GrFunRetType, GrTempIden);
	if (word[GrWordpos] == LPARSY) {
		GetWord();
		//if (word[GrWordpos] == RPARSY) { printf("Grammar Analyze Error: In function: There should be a parameter in the paralist.\n"); }
		paranum = GrParaList();
		if (word[GrWordpos] != RPARSY) { printf("Grammar Analyze Error: In function: There should be a ) after the paralist.\n"); }
		else {
			GetWord();
		}
	}
	funi = SearchTable(GrTempIden);
	VarList[funi].paranum = paranum;
	if (word[GrWordpos] != LBRACESY) {
		printf("Grammar Analyze Error: In function: There should be a { before the statements begin.\n");
	}
	else {
		GetWord();
	}
	GrCompoundStat();
	if (word[GrWordpos] != RBRACESY) {
		printf("Grammar Analyze Error: In function: There should be a } after the statements end.\n");
	}
	else {
		GetWord();
	}
	SA_funcover();
	funcflag = 0;
}
//下面这个函数用于分析 <No返回值的函数>
int GrRetFun() {
	int GrFunRetType = 0;
	char GrTempIden[MAXTOKEN] = { 0 };
	int paranum = 0;
	int funi = 0;
	funcflag = 1;
	VarListPartNum++;
	VarListPosPart = 0;
	GrFunRetType = GetWord();
	GetWord();
	strcpy(GrTempIden, GrIden);
	printf("    Grammar Analyzing for the function:%s.\n", GrTempIden);
	fprintf(Grfp, "\nThis below is a return function:%s.\n", GrTempIden);
	InsertTable(FUNEX, GrFunRetType, VarListPartNum, 0, GrTempIden, funcflag, paranum);
	InsertTable(FUNEX, GrFunRetType, VarListPartNum, 0, GrTempIden, 0, paranum);
	SA_funcdef(GrFunRetType, GrTempIden);
	if (word[GrWordpos] == LPARSY) {
		GetWord();
		paranum = GrParaList();
		if (word[GrWordpos] != RPARSY) { printf("Grammar Analyze Error: Error in function: in paralist No )!\n"); }
		else {
			GetWord();
		}
	}
	funi = SearchTable(GrTempIden);
	VarList[funi].paranum = paranum;
	if (word[GrWordpos] != LBRACESY) {
		printf("Grammar Analyze Error: Error in return function: No { in the beginning of statement list!\n");
	}
	else {
		GetWord();
	}
	GrCompoundStat();
	if (word[GrWordpos] != RBRACESY) {
		printf("Grammar Analyze Error: Error in return function: No } at the end of statement list!\n");
	}
	else {
		GetWord();
	}
	SA_funcover();
	funcflag = 0;
}
//下面这个函数处理函数的参数列表
int GrParaList() {
	int GrParaSym = 0;
	int GrTempPos = 0;
	int paranum = 0;
	do {
		if (word[GrWordpos] == COMMASY) { GetWord(); }
		GrParaSym = GetWord();
		if (word[GrWordpos] == IDSY) {
			GetWord();
			paranum++;
			GrTempPos = SA_para(GrParaSym, 1, GrIden, funcflag);
			InsertTable(PARAEX, GrParaSym, 0, 0, GrIden, funcflag, GrTempPos);
			//printf("This is a Parameter: %s.\n", GrIden);
			fprintf(Grfp, "This is a Parameter: %s.\n", GrIden);
		}
		else {
			printf("Grammar Analyze Error: Error in ParaList: No ParaID !\n");
		}
	} while (word[GrWordpos] == COMMASY);
	return paranum;
}
//下面这个函数处理复合语句（常量+变量+语句）
int GrCompoundStat() {
	if (word[GrWordpos] == CONSTSY) {
		GetWord();
		GrConstant();
	}
	while (word[GrWordpos] == INTSY || word[GrWordpos] == CHARSY) {
		GrVar();
	}
	while (word[GrWordpos] != RBRACESY) {
		GrStatement();
	}
}
//下面这个函数处理主函数
int GrMainFun() {
	funcflag = 1;
	VarListPosPart = 0;
	VarListPartNum++;
	//printf("\n_______\n");
	InsertTable(FUNEX, MAINSY, VarListPartNum, 0, "main", funcflag, 0);
	InsertTable(FUNEX, MAINSY, VarListPartNum, 0, "main", 0, 0);
	printf("    Grammar Analyzing for the main function.\n");
	fprintf(Grfp, "\n\nThis is the main function.\n");
	SA_funcdef(MAINSY, "main");
	if (word[GrWordpos] == VOIDSY) {
		GetWord();
	}
	else {
		printf("Grammar Analyze Error: Error in main: No Void before main!\n");
	}
	if (word[GrWordpos] == MAINSY) {
		GetWord();
	}
	else {
		printf("Grammar Analyze Error: Error in main: No Main where there should be void main()!\n");
	}
	if (word[GrWordpos] == LPARSY) {
		GetWord();
	}
	else {
		printf("Grammar Analyze Error: Error in main: No ( where there should be void main()!\n");
	}
	if (word[GrWordpos] == RPARSY) {
		GetWord();
	}
	else {
		printf("Grammar Analyze Error: Error in main: No ) where there should be void main()!\n");
	}
	if (word[GrWordpos] == LBRACESY) {
		GetWord();
	}
	else {
		printf("Grammar Analyze Error: Error in main: No { where there should be void main(){}!\n");
	}
	GrCompoundStat();
	if (word[GrWordpos] == RBRACESY) {
		GetWord();
	}
	else {
		printf("Grammar Analyze Error: Error in main: No } where there should be void main(){}!\n");
	}
	funcflag = 0;
}

int GrFactor() {
	char FacCh = 0;
	int FacInt = 0;
	int FacTablei = 0;
	int Factemp = 0;
	int r = 0;
	int re = 0;
	int temparr = 0;
	if (word[GrWordpos] == CHSY) {
		GetWord();
		FacCh = GrWordchar;
		re = SA_factorCh(FacCh);
		//printf("This is a char factor: %c", FacCh);
		fprintf(Grfp, "This is a char factor: %c", FacCh);
	}
	else if (word[GrWordpos] == NUMSY || word[GrWordpos] == ADDSY || word[GrWordpos] == MINUSSY) {
		FacInt = GrInteger();
		re = SA_factorNum(FacInt);
		//printf("This is an int factor: %d", FacInt);
		fprintf(Grfp, "This is an int factor: %d", FacInt);
	}
	else if (word[GrWordpos] == LPARSY) {
		GetWord();
		r=GrExp();
		if (word[GrWordpos] == RPARSY) {
			GetWord();
			re = SA_factorExp(r);
			//printf("This is an expression factor.");
			fprintf(Grfp, "This is an expression factor.");
		}
		else {
			printf("Grammar Analyze Error: Error in factor: There should be a ) after the expression as a factor!\n");
		}
	}

	else if (word[GrWordpos] == IDSY) {
		GetWord();
		{//在函数内部，先找局部表
			FacTablei = SearchPartTable(GrIden);
			if (FacTablei == -1) {
				//说明局部表中无此项，找全局表
				Factemp = 1;
				FacTablei = SearchTable(GrIden);
				if (FacTablei == -1) {
					//全局表再没有说明缺项
					printf("Grammar Analyze Error: Error in factor: the id has not been declared!\n");
					Factemp = -1;
				}
			}
			if (Factemp == 0) {//在局部表中
				//函数（递归）
				if (VarListPart[VarListPartNum][FacTablei].cla == FUNEX) {
					re = GrRetCall(VarListPart[VarListPartNum][FacTablei].name);
					//printf("This is a return function factor:%s.", VarListPart[VarListPartNum][FacTablei].name);
					fprintf(Grfp, "This is a return function factor:%s.", VarListPart[VarListPartNum][FacTablei].name);
				}
				//数组
				else if (VarListPart[VarListPartNum][FacTablei].cla == ARRAYEX) {
					if (word[GrWordpos] == LBRACKETSY) {
						GetWord();
					}
					else printf("Grammar Analyze Error: Error in factor of array: No [!\n");
				    //检查越界
					if (word[GrWordpos] == NUMSY) {
						temparr = wordToGrammernum[Grnumposi];
						if (temparr >= VarListPart[VarListPartNum][FacTablei].num) {
							printf("Sema Analyze Error: The array is out of band!\n");
						}
					}
					if (word[GrWordpos + 1] == NUMSY&&word[GrWordpos] == MINUSSY) {
						temparr = wordToGrammernum[Grnumposi];
						if (temparr !=0) {
							printf("Sema Analyze Error: The pointer of the array is smaller than zero!\n");
						}
					}
					r = GrExp();
					if (word[GrWordpos] == RBRACKETSY) {
						GetWord();
					}
					else printf("Grammar Analyze Error: Error in factor of array: No ]!\n");
					re = SA_factorAR(VarListPart[VarListPartNum][FacTablei].name, r);
					//printf("This is an array factor:%s[].", VarListPart[VarListPartNum][FacTablei].name);
					fprintf(Grfp, "This is an array factor:%s[].", VarListPart[VarListPartNum][FacTablei].name);
				}
				//变量
				else {
					re = SA_factorVar(GrIden, VarListPartNum);
					//printf("This is a factor:%s.", GrIden);
					fprintf(Grfp, "This is a factor:%s.", GrIden);
				}
			}
			else if (Factemp != -1) {//在全局表中
				//函数
				if (VarList[FacTablei].cla == FUNEX) {
					//printf("This is a return function factor:%s.", VarList[FacTablei].name);
					fprintf(Grfp, "This is a return function factor:%s.", VarList[FacTablei].name);
					re = GrRetCall(VarList[FacTablei].name);
					
				}
				//数组
				else if (VarList[FacTablei].cla == ARRAYEX) {
					if (word[GrWordpos] == LBRACKETSY) {
						GetWord();
					}
					else printf("Grammar Analyze Error: Error in factor of array: No [!\n");
					//检查越界
					if (word[GrWordpos] == NUMSY) {
						temparr = wordToGrammernum[Grnumposi];
						if (temparr >= VarList[FacTablei].num) {
							printf("Sema Analyze Error: The array is out of band!\n");
						}
					}
					if (word[GrWordpos + 1] == NUMSY&&word[GrWordpos] == MINUSSY) {
						temparr = wordToGrammernum[Grnumposi];
						if (temparr != 0) {
							printf("Sema Analyze Error: The pointer of the array is smaller than zero!\n");
						}
					}
					r = GrExp();
					if (word[GrWordpos] == RBRACKETSY) {
						GetWord();
					}
					else printf("Grammar Analyze Error: Error in factor of array: No ]!\n");
					re = SA_factorAR(VarList[FacTablei].name, r);
					//printf("This is an array factor:%s[].", VarList[FacTablei].name);
					fprintf(Grfp, "This is an array factor:%s[].", VarList[FacTablei].name);
				}
				//变量
				else {
					re = SA_factorVar(GrIden, VarListPartNum);
					//printf("This is a factor:%s.", GrIden);
					fprintf(Grfp, "This is a factor:%s.", GrIden);
				}
			}
		}
	}

	else { printf("Grammar Analyze Error: error in factor: this is not a factor!\n"); }
	return re;
}

int GrTerm() {
	int r = 0;
	int f1 = 0;
	int f2 = 0;
	int type = 0;
	f1 = GrFactor();
	if (word[GrWordpos] != MULTISY && word[GrWordpos] != DIVSY) {
		return f1;
	}
	else {
		do {
			type = GetWord();
			f2 = GrFactor();
			if (type == MULTISY) {
				f1 = SA_termm(f1, f2);
			}
			else if (type == DIVSY) {
				f1 = SA_termd(f1, f2);
			}
		} while (word[GrWordpos] == MULTISY || word[GrWordpos] == DIVSY);
	}
	//printf("These factors above is a term * or / .");
	fprintf(Grfp, "These factors above is a term * or / .");
	return f1;
}

int GrExp() {
	int t1 = 0;
	int t2 = 0;
	int flag = 1;
	int type = 0;
	//首先处理表达式首的减号，加号直接忽略，减号相当于相反数
	if (word[GrWordpos] == ADDSY) {
		GetWord();
	}
	else if (word[GrWordpos] == MINUSSY) {
		flag = -1;
		GetWord();
	}
	t1 = GrTerm();
	if (flag == -1) {
		t1 = SA_expopp(t1);
	}
	if (word[GrWordpos] != ADDSY && word[GrWordpos] != MINUSSY) {
		return t1;
	}
	else {
		do {
			type = GetWord();
			t2 = GrTerm();
			if (type == ADDSY) {
				t1 = SA_expadd(t1, t2);
			}
			else if (type == MINUSSY) {
				t1 = SA_expsub(t1, t2);
			}
		} while (word[GrWordpos] == ADDSY || word[GrWordpos] == MINUSSY);
	}
	
	//printf("These terms above are a Expression.\n");
	fprintf(Grfp, "These terms above are a Expression.\n");
	return t1;
}

int GrStatement() {
	int StateIdi = 0;
	int StatePartIdi = 0;
	int tempsearch = 0;
	int re = 0;
	int re1 = 0;
	int re2 = 0;
	int end = 0;
	char name[MAXTOKEN] = { 0 };
	int temp = 0;

	switch (word[GrWordpos]) {
	case IFSY:
		GetWord();
		printf("\tGrammar Analyzing for the if statement.\n");
		fprintf(Grfp, "This is a if statement.\n");
		if (word[GrWordpos] == LPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in if statement: there should be a ( after if.\n");
		}
		GrCondition();
		re1 = SA_bz();
		if (word[GrWordpos] == RPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in if statement: there should be a ) after if.\n");
		}
		GrStatement();
		re2 = SA_goto();
		if (word[GrWordpos] == ELSESY) {
			//printf("This is a else statement.\n");
			fprintf(Grfp, "This is a else statement.\n");
			GetWord();
			SA_label(re1);
		}
		else {
			printf("Grammar Analyze Error: Error in if statement: there should be a else after if.\n");
		}
		GrStatement();
		SA_label(re2);
		break;
	case DOSY:
		GetWord();
		re1 = SA_do();
		printf("\tGrammar Analyzing for the do-while statement.\n");
		fprintf(Grfp, "This is a do statement.\n");
		GrStatement();
		if (word[GrWordpos] == WHILESY) {
			//printf("This is a while statement.\n");
			fprintf(Grfp, "This is a while statement.\n");
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in do-while statement: there should be a while after do.\n");
		}
		if (word[GrWordpos] == LPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in do-while statement: there should be a ( after while.\n");
		}
		GrCondition();
		SA_bnz(re1);
		if (word[GrWordpos] == RPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in do-while statement: there should be a ) after while.\n");
		}
		break;
	case LBRACESY:
		GetWord();
		//printf("This is a statements list statement.\n");
		fprintf(Grfp, "This is a statements list statement.\n");
		while (word[GrWordpos] != RBRACESY) {
			GrStatement();
		}
		GetWord();
		break;
	case SWITCHSY:
		GetWord();
		printf("\tGrammar Analyzing for the switch statement.\n");
		fprintf(Grfp, "This is a switch statement.\n");
		if (word[GrWordpos] == LPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in switch statement: there should be a ( after switch.\n");
		}
		re1 = GrExp();
		if (word[GrWordpos] == RPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in switch statement: there should be a ) after switch.\n");
		}
		if (word[GrWordpos] == LBRACESY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in switch statement: there should be a { after switch.\n");
		}
		do {
			if (word[GrWordpos] == CASESY) {
				//printf("This is a case statement.\n");
				fprintf(Grfp, "This is a case statement.\n");
				GetWord();
			}
			else {
				printf("Grammar Analyze Error: Error in switch statement: there should be a case after switch.\n");
			}
			if (word[GrWordpos] == NUMSY || word[GrWordpos] == CHSY|| word[GrWordpos] == ADDSY || word[GrWordpos] == MINUSSY) {
				if (word[GrWordpos] == NUMSY || word[GrWordpos] == ADDSY || word[GrWordpos] == MINUSSY) {
					re2 = GrInteger();
				}
				else {
					GetWord();
					re2 = (int)(GrWordchar);
				}
				re = SA_case(re1, re2);
			}
			else {
				printf("Grammar Analyze Error: Error in switch statement: there should be a num or char after case.\n");
			}
			if (word[GrWordpos] == COLONSY) {
				GetWord();
			}
			else {
				printf("Grammar Analyze Error: Error in switch statement: there should be a : after case.\n");
			}
			GrStatement();
			if (end == 0) {
				end = SA_goto();
			}
			else {
				SA_gotoswend(end);
			}
			SA_label(re);
		} while (word[GrWordpos] == CASESY);
		if (word[GrWordpos] == DEFAULTSY) {
			//printf("This is a default statement.\n");
			fprintf(Grfp, "This is a default statement.\n");
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in switch statement: there should be a default after switch.\n");
		}
		if (word[GrWordpos] == COLONSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in switch statement: there should be a : after default.\n");
		}
		GrStatement();
		SA_label(end);
		if (word[GrWordpos] == RBRACESY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in switch statement: there should be a } after switch.\n");
		}
		break;
	case IDSY:
		GetWord();
		StateIdi = SearchTable(GrIden);
		if ((StateIdi != -1) && (VarList[StateIdi].cla == FUNEX)) {
			//re = SA_noretcall(VarList[StateIdi].name);
			//printf("This is a function call statement.\n");
			fprintf(Grfp, "This is a function call statement.\n");
			if (VarList[StateIdi].type == VOIDSY) {
				GrNoRetCall(VarList[StateIdi].name);
			}
			else {
				GrRetCall(VarList[StateIdi].name);
			}
			if (word[GrWordpos] == SEMICOLONSY) {
				GetWord();
			}
			else {
				printf("Grammar Analyze Error: Error in statement: there should be a ; after a call.\n");
			}
		}
		else {
			StatePartIdi = SearchPartTable(GrIden);
			if (StateIdi == -1 && StatePartIdi == -1) { printf("Error in State: ID not exist!\n"); }
			else if (StatePartIdi != -1) {
				if (VarListPart[VarListPartNum][StatePartIdi].cla == ARRAYEX) {
					if (word[GrWordpos] == LBRACKETSY) {
						GetWord();
						strcpy(name, GrIden);
						//printf("This is a assign statement.%s[]=\n", GrIden);
						fprintf(Grfp, "This is a assign statement.%s[]=\n", GrIden);
					}
					else {
						printf("Grammar Analyze Error: Error in assign statement: there should be a [ after array.\n");
					}
					re1 = GrExp();
					
					if (word[GrWordpos] == RBRACKETSY) {
						GetWord();
					}
					else {
						printf("Grammar Analyze Error: Error in assign statement: there should be a ] after array.\n");
					}
				}
				else {
					strcpy(name, GrIden);
					//printf("This is a assign statement.%s=\n", GrIden);
					fprintf(Grfp, "This is a assign statement.%s=\n", GrIden);
				}
				if (word[GrWordpos] == ASSIGNSY) {
					GetWord();
				}
				else {
					printf("Grammar Analyze Error: Error in assign statement: there should be a = in assign.\n");
				}
				//类型检验
				if (VarListPart[VarListPartNum][StatePartIdi].cla == CONSTANTEX) {
					printf("Sema Analyze Error: a const is assigned!\n");
				}
				if (VarListPart[VarListPartNum][StatePartIdi].type == CHARSY) {
					if (word[GrWordpos] != CHSY&&word[GrWordpos] != IDSY&&word[GrWordpos] != LPARSY) {
						printf("Sema Analyze Error: an int is assigned to a char!\n");
					}
					if (word[GrWordpos] == IDSY) {
						tempsearch= SearchPartTable(wordToGrammertoken[Grtokenposi]);
						if (tempsearch == -1) {
							tempsearch = SearchTable(wordToGrammertoken[Grtokenposi]);
							if (VarListPart[VarListPartNum][StatePartIdi].type == VarList[tempsearch].type) {}
							else {
								printf("Sema Analyze Error: an int is assigned to a char var!\n");
							}
						}
						else {
							if(VarListPart[VarListPartNum][StatePartIdi].type == VarListPart[VarListPartNum][tempsearch].type){}
							else {
								printf("Sema Analyze Error: an int is assigned to a char var!\n");
							}
						}
					}
				}
				if (VarListPart[VarListPartNum][StatePartIdi].type == INTSY) {
					if (word[GrWordpos] == CHSY) {
						printf("Sema Analyze Error: a char is assigned to an int!\n");
					}
					if (word[GrWordpos] == IDSY) {
						tempsearch = SearchPartTable(wordToGrammertoken[Grtokenposi]);
						if (tempsearch == -1) {
							tempsearch = SearchTable(wordToGrammertoken[Grtokenposi]);
							if (VarListPart[VarListPartNum][StatePartIdi].type == VarList[tempsearch].type) {}
							else {
								printf("Sema Analyze Error: a char is assigned to an int var!\n");
							}
						}
						else {
							if (VarListPart[VarListPartNum][StatePartIdi].type == VarListPart[VarListPartNum][tempsearch].type) {}
							else {
								printf("Sema Analyze Error: an char is assigned to an int var!\n");
							}
						}
					}
				}
				re2 = GrExp();
				if (re1 != 0) {
					SA_assignarr(name, re1, re2);
				}
				else {
					SA_assignvar(name, re2);
				}
				if (word[GrWordpos] == SEMICOLONSY) {
					GetWord();
				}
				else {
					printf("Grammar Analyze Error: Error in assign statement: there should be a ; after an assign.\n");
				}
			}
			else {
				if (VarList[StateIdi].cla == ARRAYEX) {
					strcpy(name, GrIden);
					//printf("This is a assign statement.%s[]=\n", GrIden);
					fprintf(Grfp, "This is a assign statement.%s[]=\n", GrIden);
					if (word[GrWordpos] == LBRACKETSY) {
						GetWord();
					}
					else {
						printf("Grammar Analyze Error: Error in assign statement: there should be a [ after array.\n");
					}
					re1 = GrExp();
					if (word[GrWordpos] == RBRACKETSY) {
						GetWord();
					}
					else {
						printf("Grammar Analyze Error: Error in assign statement: there should be a ] after array.\n");
					}
				}
				else {
					strcpy(name, GrIden);
					//printf("This is a assign statement.%s=\n", GrIden);
					fprintf(Grfp, "This is a assign statement.%s=\n", GrIden);
				}
				if (word[GrWordpos] == ASSIGNSY) {
					GetWord();
				}
				else {
					printf("Grammar Analyze Error: Error in statement: there should be a = in assign.\n");
				}
				//类型检查
				if (VarList[StateIdi].cla == CONSTANTEX) {
					printf("Sema Analyze Error: a const is assigned!\n");
				}
				if (VarList[StateIdi].type == CHARSY) {
					if (word[GrWordpos] != CHSY&&word[GrWordpos] != IDSY&&word[GrWordpos] != LPARSY) {
						printf("Sema Analyze Error: an int is assigned to a char!\n");
					}
					if (word[GrWordpos] == IDSY) {
						tempsearch = SearchPartTable(wordToGrammertoken[Grtokenposi]);
						if (tempsearch == -1) {
							tempsearch = SearchTable(wordToGrammertoken[Grtokenposi]);
							if (VarList[StateIdi].type == VarList[tempsearch].type) {}
							else {
								printf("Sema Analyze Error: an int is assigned to a char var!\n");
							}
						}
						else {
							if (VarList[StateIdi].type == VarListPart[VarListPartNum][tempsearch].type) {}
							else {
								printf("Sema Analyze Error: an int is assigned to a char var!\n");
							}
						}
					}
				}
				if (VarList[StateIdi].type == INTSY) {
					if (word[GrWordpos] == CHSY) {
						printf("Sema Analyze Error: a char is assigned to an int!\n");
					}
					if (word[GrWordpos] == IDSY) {
						tempsearch = SearchPartTable(wordToGrammertoken[Grtokenposi]);
						if (tempsearch == -1) {
							tempsearch = SearchTable(wordToGrammertoken[Grtokenposi]);
							if (VarList[StateIdi].type == VarList[tempsearch].type) {}
							else {
								printf("Sema Analyze Error: a char is assigned to an int var!\n");
							}
						}
						else {
							if (VarList[StateIdi].type == VarListPart[VarListPartNum][tempsearch].type) {}
							else {
								printf("Sema Analyze Error: an char is assigned to an int var!\n");
							}
						}
					}
				}
				re2 = GrExp();
				if (re1 > 0) {
					SA_assignarr(name, re1, re2);
				}
				else {
					SA_assignvar(name, re2);
				}
				if (word[GrWordpos] == SEMICOLONSY) {
					GetWord();
				}
				else {
					printf("Grammar Analyze Error: Error in assign statement: there should be a ; after an assign.\n");
				}
			}
		}
		break;
	case SCANFSY:
		GetWord();
		//printf("This is a scanf statement.\n");
		fprintf(Grfp, "This is a scanf statement.\n");
		if (word[GrWordpos] == LPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ( after scanf.\n");
		}
		if (word[GrWordpos] == IDSY) {
			GetWord();
			SA_scanf(GrIden);
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be an ID after scanf.\n");
		}
		while (word[GrWordpos] == COMMASY) {
			GetWord();
			if (word[GrWordpos] == IDSY) {
				GetWord();
				SA_scanf(GrIden);
			}
			else {
				printf("Grammar Analyze Error: Error in statement: there should be an ID after scanf.\n");
			}
		}
		if (word[GrWordpos] == RPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ) after scanf.\n");
		}
		if (word[GrWordpos] == SEMICOLONSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ; after a scanf.\n");
		}
		break;
	case PRINTFSY:
		GetWord();
		//printf("This is a printf statement.\n");
		fprintf(Grfp, "This is a printf statement.\n");
		if (word[GrWordpos] == LPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ( after printf.\n");
		}
		if (word[GrWordpos] == STRSY) {
			GetWord();
			SA_printfstr();
		}
		if (word[GrWordpos] == COMMASY) {
			GetWord();
		} 
		{
			if (word[GrWordpos] == IDSY && word[GrWordpos + 1] != ADDSY&& word[GrWordpos + 1] != MINUSSY && word[GrWordpos + 1] != MULTISY && word[GrWordpos + 1] != DIVSY && word[GrWordpos + 1] != LBRACKETSY) {
				GetWord();
				temp = SearchTable(GrIden);
				if (temp == -1) {
					SA_printfid(GrIden);
				}
				else {
					if (VarList[temp].cla == FUNEX) {
						GrWordpos--;
						Grtokenposi--;
						re = GrExp();
						if (VarList[temp].type == CHARSY) {
							SA_printfreg(re, 1);
						}
						else {
							SA_printfreg(re, 0);
						}
					}
					else {
						SA_printfid(GrIden);
					}
				}
			}
			else if (word[GrWordpos] == ADDSY || word[GrWordpos] == MINUSSY || word[GrWordpos] == IDSY || word[GrWordpos] == CHSY || word[GrWordpos] == NUMSY || word[GrWordpos] == LPARSY) {
				temp = GrWordpos;
				re = GrExp();
				if (word[temp] == CHSY&& word[temp + 1] != ADDSY&& word[temp + 1] != MINUSSY && word[temp + 1] != MULTISY && word[temp + 1] != DIVSY) {
					SA_printfreg(re, 1);
				}
				//这里存在bug，如果数组指针存在运算，可能会导致表达式打印成字符
				else if (word[temp] == IDSY&& word[temp + 1] == LBRACKETSY && word[temp + 4] != ADDSY&& word[temp + 4] != MINUSSY && word[temp + 4] != MULTISY && word[temp + 4] != DIVSY) {
					SA_printfreg(re, 1);
				}
				else {
					SA_printfreg(re, 0);
				}
			}
		}
		if (word[GrWordpos] == RPARSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ) after printf.\n");
		}
		if (word[GrWordpos] == SEMICOLONSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ; after a printf.\n");
		}
		break;
	case SEMICOLONSY:
		GetWord();
		//printf("This is a ; statement.\n");
		fprintf(Grfp, "This is a ; statement.\n");
		break;
	case RETURNSY:
		GetWord();
		//printf("This is a return statement.\n");
		fprintf(Grfp, "This is a return statement.\n");
		if (word[GrWordpos] != LPARSY) {
			SA_noreturn();
		}
		if (word[GrWordpos] == LPARSY) {
			GetWord();
			re = GrExp();
			SA_return(re);
			if (word[GrWordpos] == RPARSY) {
				GetWord();
			}
			else {
				printf("Grammar Analyze Error: Error in statement: there should be a ) after return().\n");
			}
		}
		if (word[GrWordpos] == SEMICOLONSY) {
			GetWord();
		}
		else {
			printf("Grammar Analyze Error: Error in statement: there should be a ; after a return.\n");
		}
		break;
	default:
		printf("Grammar Analyze Error: Error in statement: This is not a statement!\n");
		GetWord();
	}
}

int GrCondition() {
	int type = 0;
	int e1 = 0;
	int e2 = 0;
	e1 = GrExp();
	if (word[GrWordpos] >= LESSSY&& word[GrWordpos] <= MOREEQSY || word[GrWordpos] == NEQSY) {
		type = GetWord();
		e2 = GrExp();
		SA_condition(e1, type, e2);
	}
	else {
		SA_condition(e1, MORESY, e2);
	}
	//printf("This is a condition.\n");
	fprintf(Grfp, "This is a condition.\n");
}

//int pushfunci = 0;
int GrRetCall(char * name) {
	int r = 0;
	//int pushstack[32][100] = { 0 };
	int pushstack[100] = { 0 };
	int pushi = 0;
	int i = 0;
	int funi = 0;
	//pushfunci++;
	if (word[GrWordpos] == LPARSY) {
		GetWord();
		r = GrExp();
		//SA_pushpara(r);
		//pushstack[pushfunci][pushi] = r;
		pushstack[pushi] = r;
		pushi++;
		while (word[GrWordpos] != RPARSY) {
			if (word[GrWordpos] == COMMASY) {
				GetWord();
			}
			r = GrExp();
			//SA_pushpara(r);
			//pushstack[pushfunci][pushi] = r;
			pushstack[pushi] = r;
			pushi++;
		}
		funi = SearchTable(name);
		if (pushi != VarList[funi].paranum) {
			printf("Sema Error in func %s: parameter num is not the same!\n", name);
		}
		GetWord();
		for (i = 0; i < pushi; i++) {
			//SA_pushpara(pushstack[pushfunci][i]);
			//pushstack[pushfunci][i] = 0;
			SA_pushpara(pushstack[i]);
			pushstack[i] = 0;
		}
		//pushfunci--;
	}
	r = SA_call(name);
	//printf("This is a call.\n");
	fprintf(Grfp, "This is a call.\n");
	return r;
}

int GrNoRetCall(char* name) {
	int r = 0;
	int pushnum = 0;
	int funi = 0;
	SA_pushover();
	if (word[GrWordpos] == LPARSY) {
		GetWord();
		r = GrExp();
		SA_pushpara(r);
		pushnum++;
		while (word[GrWordpos] != RPARSY) {
			if (word[GrWordpos] == COMMASY) {
				GetWord();
			}
			r = GrExp();
			SA_pushpara(r);
			pushnum++;
		}
		GetWord();
	}
	funi = SearchTable(name);
	if (pushnum != VarList[funi].paranum) {
		printf("Sema Error in func %s: parameter num is not the same!\n", name);
	}
	r = SA_noretcall(name);
	//GetWord();
	
	//printf("This is a call.\n");
	fprintf(Grfp, "This is a call.\n");
	return r;
}

int grammerAna() {
	Grfp = fopen("Result2_Grammer.txt", "w");
	wordAna();
	printf("____Grammar Analyzing...____\n");
	//GetWordFile();
	GrProgram();
	printf("____Grammer Analyze Done!____\n\n");
	fprintf(Grfp, "Congratulations!\nGrammer Analyze Complete!\n");
	fclose(Grfp);
}

