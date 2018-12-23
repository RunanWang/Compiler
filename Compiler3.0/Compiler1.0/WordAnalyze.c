#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "WordAnalyze.h"

//////////////////////////////////////////////////////////////////////////////
char code[MAXCODELENGTH] = { 0 };					//源代码
int pos = 0;										//源代码指针（处理位置）
char token[MAXTOKEN] = { 0 };						//临时储存单词
int tokenpos = 0;									//存储单词的指针
int num = 0;										//用于换算整数
int stringnum = 0;									//统计共有多少个string
char chout = 0;										//用于输出字符
FILE* fpout;										//用于用文件输出答案

//下面这些变量和数组用于在语法分析和词法分析之间传递相应的值
char string[MAXNUM][256] = { { 0 } };				//字符串
int wordToGrammernum[MAXNUM] = { 0 };				//数值
char wordToGrammerchar[MAXNUM] = { 0 };				//字符
char wordToGrammertoken[MAXNUM][MAXTOKEN] = { 0 };	//标识符
int word[MAXWORDNUM] = { 0 };						//保留字代号
int wordposi = 0;									//保留字位置
int numposi = 0;									//数值位置
int charposi = 0;									//字符位置
int tokenposi = 0;									//标识符位置
int codeline = 1;
//这个数组是定义的保留字编号宏所对应的含义
char reserch[50][20] = { "CASESY",		"CHARSY",	"CONSTSY",	"DEFAULTSY",	"DOSY",
"ELSESY",		"IFSY",		"INTSY",	"MAINSY",		"PRINTFSY",
"RETURNSY",		"SCANFSY",	"SWITCHSY",	"VOIDSY",		"WHILESY",
"SINGLEQUOTESY","MINUSSY",	"NEQSY",	"DOUBLEEQUOTESY","LPARSY",
"RPARSY",		"MULTISY",	"COMMASY",	"DIVSY",		"SEMICOLONSY",
"LBRACKETSY",	"RBRACKETSY","LBRACESY","RBRACESY",		"ADDSY",
"LESSSY",		"LESSEQSY",	"ASSIGNSY",	"EQSY",			"MORESY",
"MOREEQSY",		"COLONSY",	"IDSY",		"NUMSY",		"STRSY",
"CHSY" };											

////////////////////////////////////////////////////////////////////////////
//这个函数实现的是将存在文件中的code转移到code数组中，需要输入需要翻译的文件的路径
void getcode() {
	FILE *fpin = NULL;
	char path[200] = { 0 };			//将path置空
	int i = 0;						//i用于code中计数
	char ch = 0;					//ch用于临时存储读到的字符
	printf("Please enter the path of the code: \n");
	scanf("%s", path);				//读路径到path
	fpin = fopen(path, "r");		//打开文件
	ch = getc(fpin);				//获得字符
	while (ch != EOF) {
		code[i] = ch;				//转移字符
		ch = getc(fpin);
		i++;
	}
	fclose(fpin);
}
//这个函数判断code[pos]是不是letter（或下划线），是返回1，不是返回0
int isLetter() {
	if (code[pos] >= 65 && code[pos] <= 90 || code[pos] >= 97 && code[pos] <= 122 || code[pos] == '_') {
		return 1;
	}
	else return 0;
}
//这个函数判断code[pos]是不是数字，是返回1，不是返回0
int isDigit() {
	if (code[pos] >= 48 && code[pos] <= 57) {
		return 1;
	}
	else return 0;
}
//这个函数清空token区
void clearToken() {
	int i = 0;
	for (i = 0; i < MAXTOKEN; i++) {
		token[i] = 0;
	}
}
//这个函数将读到的code[pos]放入token[tokenpos]中，并转换大小写
void catToken() {
	if (code[pos] >= 'A'&&code[pos] <= 'Z') {
		token[tokenpos] = code[pos] - 'A' + 'a';
	}//将所有字符转换为小写字母（不区分大小写）
	else {
		token[tokenpos] = code[pos];
	}
	tokenpos++;
}
//这个函数判断token是否是保留字，如果是，会返回对应保留字的值，并转到wordout输出，不是返回0
int Reserve() {
	if (!strcmp(token, "case")) {
		wordout(CASESY);
		return CASESY;
	}
	else if (!strcmp(token, "char")) {
		wordout(CHARSY);
		return CHARSY;
	}
	else if (!strcmp(token, "const")) {
		wordout(CONSTSY);
		return CONSTSY;
	}
	else if (!strcmp(token, "default")) {
		wordout(DEFAULTSY);
		return DEFAULTSY;
	}
	else if (!strcmp(token, "do")) {
		wordout(DOSY);
		return DOSY;
	}
	else if (!strcmp(token, "else")) {
		wordout(ELSESY);
		return ELSESY;
	}
	else if (!strcmp(token, "if")) {
		wordout(IFSY);
		return IFSY;
	}
	else if (!strcmp(token, "int")) {
		wordout(INTSY);
		return INTSY;
	}
	else if (!strcmp(token, "main")) {
		wordout(MAINSY);
		return MAINSY;
	}
	else if (!strcmp(token, "printf")) {
		wordout(PRINTFSY);
		return PRINTFSY;
	}
	else if (!strcmp(token, "return")) {
		wordout(RETURNSY);
		return RETURNSY;
	}
	else if (!strcmp(token, "scanf")) {
		wordout(SCANFSY);
		return SCANFSY;
	}
	else if (!strcmp(token, "switch")) {
		wordout(SWITCHSY);
		return SWITCHSY;
	}
	else if (!strcmp(token, "void")) {
		wordout(VOIDSY);
		return VOIDSY;
	}
	else if (!strcmp(token, "while")) {
		wordout(WHILESY);
		return WHILESY;
	}
	else {
		return 0;
	}
}
//这个函数将输出词法分析结果到输出控制台和指定输出文件
int wordout(int typesym) {
	//是保留字，打印保留字对应的序号和内容
	//printf("%d ", typesym);
	//printf("%s ", reserch[typesym - 1]);
	if (typesym != LINESY) {
		fprintf(fpout, "%d ", typesym);
		word[wordposi] = typesym;
		fprintf(fpout, "%s ", reserch[typesym - 1]);
	}
	else {
		word[wordposi] = typesym;
	}
	//是整数值，还要打印整数值
	if (typesym == NUMSY) {
		//printf("%d ", num);
		fprintf(fpout, "%d ", num);
		wordToGrammernum[numposi] = num;
		numposi++;
	}
	//是标识符，还要打印标识符是什么。后面如果要建立变量表可以在这里插入一个函数
	//settable()，将token[]存入并查重
	else if (typesym == IDSY) {
		//printf("%s ", token);
		fprintf(fpout, "%s ", token);
		strcpy(wordToGrammertoken[tokenposi], token);
		tokenposi++;
	}
	//是string类型，打印在string数组中的序号和内容
	else if (typesym == STRSY) {
		//printf("pos:%d %s", stringnum - 1, string[stringnum - 1]);
		fprintf(fpout, "pos:%d %s", stringnum - 1, string[stringnum - 1]);
		wordToGrammernum[numposi] = stringnum - 1;
		numposi++;
	}
	//是char类型，额外输出cahr是什么
	else if (typesym == CHSY) {
		//printf("char:'%c'", chout);
		fprintf(fpout, "char:'%c'", chout);
		wordToGrammerchar[charposi] = chout;
		charposi++;
	}
	else {
		//fprintf(fpout, "0");
	}
	//printf("\n");
	fprintf(fpout, "\n");
	wordposi++;
}
//下面这个函数将字符转换为数字
int transNum() {
	int i = 0;
	int temp = 0;
	for (i = 0; token[i] != 0; i++) {
		temp = temp * 10 + (token[i] - '0');
	}
	return temp;
}
//下面这个函数检查单引号中的char是否合法
void charwork() {
	char ch = 0;
	ch = code[pos];
	if (ch >= 'a'&&ch <= 'z' || ch >= 'A'&&ch <= 'Z' || ch == '_' || ch <= '9'&&ch >= '0' || ch == '+' || ch == '*' || ch == '-' || ch == '/') {
		chout = ch;
	}
	else {
		printf("Word Analyze Error: In line%d: Invalid Char in a '': %c\n",codeline, ch);
	}
	wordout(CHSY);
}
//下面的这个函数获取下一个标识符
int getsym() {
	int reserveflag = 0;
	clearToken();
	tokenpos = 0;
	//去掉空字符，统计行数
	while (code[pos] == ' ' || code[pos] == '\n' || code[pos] == '\t') {
		if (code[pos] == '\n') {
			codeline++;
		}
		pos++;
	}
	//先判断是否是字母，是字母则可能是标识符
	if (isLetter()) {
		while (isLetter() || isDigit()) {
			catToken();
			pos++;
		}
		pos--;
		reserveflag = Reserve();
		if (reserveflag) {
			return reserveflag;
		}
		else {
			wordout(IDSY);
			return(IDSY);
		}
	}
	//判断是否是数字
	else if (isDigit()) {
		while (isDigit()) {
			catToken();
			pos++;
		}
		pos--;
		num = 0;
		num = transNum();
		wordout(NUMSY);
		return NUMSY;
	}
	//判断各种符号
	else if (code[pos] == '-') {
		wordout(MINUSSY);
		return MINUSSY;
	}
	else if (code[pos] == '\'') {
		//wordout(SINGLEQUOTESY);
		pos++;
		charwork();
		pos++;
		if (code[pos] == '\'') {
			//wordout(SINGLEQUOTESY);
			return SINGLEQUOTESY;
		}
		else {
			printf("Error! Only one single quote! '\n");
			return 0;
		}
	}
	else if (code[pos] == '!') {
		pos++;
		if (code[pos] == '=') {
			wordout(NEQSY);
			return NEQSY;
		}
		else {
			printf("\n Error in finding ! \n");
			pos--;
			return -1;
		}
	}
	else if (code[pos] == '\"') {
		int i = 0;
		//wordout(DOUBLEEQUOTESY);
		pos++;
		while (code[pos] != '\"') {
			if (code[pos] == '\\') {
				string[stringnum][i] = '\\';
				i++;
			}
			string[stringnum][i] = code[pos];
			i++;
			pos++;
		}
		string[stringnum][i] = '\\';
		i++;
		string[stringnum][i] = 'n';
		i++;
		stringnum++;
		wordout(STRSY);
		//wordout(DOUBLEEQUOTESY);
	}
	else if (code[pos] == '(') {
		wordout(LPARSY);
		return LPARSY;
	}
	else if (code[pos] == ')') {
		wordout(RPARSY);
		return RPARSY;
	}
	else if (code[pos] == '*') {
		wordout(MULTISY);
		return MULTISY;
	}
	else if (code[pos] == ',') {
		wordout(COMMASY);
		return COMMASY;
	}
	else if (code[pos] == '/') {
		wordout(DIVSY);
		return DIVSY;
	}
	else if (code[pos] == ';') {
		wordout(SEMICOLONSY);
		return SEMICOLONSY;
	}
	else if (code[pos] == '[') {
		wordout(LBRACKETSY);
		return LBRACKETSY;
	}
	else if (code[pos] == ']') {
		wordout(RBRACKETSY);
		return RBRACKETSY;
	}
	else if (code[pos] == '{') {
		wordout(LBRACESY);
		return LBRACESY;
	}
	else if (code[pos] == '}') {
		wordout(RBRACESY);
		return RBRACESY;
	}
	else if (code[pos] == '+') {
		wordout(ADDSY);
		return ADDSY;
	}
	else if (code[pos] == '<') {
		pos++;
		if (code[pos] == '=') {
			wordout(LESSEQSY);
			return LESSEQSY;
		}
		pos--;
		wordout(LESSSY);
		return(LESSSY);
	}
	else if (code[pos] == '=') {
		pos++;
		if (code[pos] == '=') {
			wordout(EQSY);
			return EQSY;
		}
		pos--;
		wordout(ASSIGNSY);
		return ASSIGNSY;
	}
	else if (code[pos] == '>') {
		pos++;
		if (code[pos] == '=') {
			wordout(MOREEQSY);
			return MOREEQSY;
		}
		pos--;
		wordout(MORESY);
		return MORESY;
	}
	else if (code[pos] == ':') {
		wordout(COLONSY);
		return COLONSY;
	}
	else {
		printf("Word Analyze Error: In line%d: the word %c:%d can not be recognized!pos:%d\n", codeline, code[pos], code[pos], pos);
		return 0;
	}
}
//下面这个函数将以上的函数进行调用
int wordAna() {
	int r = 0;
	getcode();
	fpout = fopen("Result1_Word.txt", "w");
	printf("____Word Analyzing...____\n");
	while (code[pos] != 0) {
		r = getsym();
		pos++;
		if (r == 0) {
			printf("WordAnalyze Wrong!\n");
		}
	}
	fprintf(fpout, "WordAnalyze Done!\n");
	fclose(fpout);
	printf("____WordAnalyze Done!____\n\n");
}
