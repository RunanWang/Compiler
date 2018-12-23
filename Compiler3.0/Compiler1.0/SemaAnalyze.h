#pragma once
#define MAXMID 1024

#define SAALLOCOP		1
#define SADATAOP		2
#define SAFUNCDEFOP		3
#define SAPARAOP		4
#define SAFACCHOP		5
#define SAFACNUMOP		6
#define SAFACEXPOP		7

#define SAFACAROP		10
#define SAFACVAROP		11
#define SATERMMOP		12
#define SATERMDOP		13
#define SAEXPOPPOP		14
#define SAEXPADDOP		15
#define SAEXPSUBOP		16
#define SACONDOP		17
#define SARETOP			18
#define SANORETOP		19
#define SAPRINTFSTROP	20
#define SAPRINTFREGOP	21
#define SASCANFOP		22
#define SANORETCALLOP	23
#define SAASSIGNARROP	24
#define SAASSIGNVAROP	25
#define SABZOP			26
#define SAGOTOOP		27
#define SALABELOP		28
#define SADOOP			29
#define SABNZOP			30
#define SACASEOP		31
#define SAPUSHOVEROP	32
#define SACALLOP		38
#define SAPUSHOP		39
#define SAPRINTFIDOP	40
#define SAFUNCOVEROP	41

#define SAADDIOP		50
#define SANOPOP			51
int SA_alloc(int Type, int size, char* name, int funcflag);
int SA_para(int Type, int size, char* name, int funcflag);
int SA_funcover();
int SA_funcdef(int Type, char* name);

int SA_call(char* name);
int SA_noretcall(char* name);

int SA_factorCh(char ch);
int SA_factorNum(int num);
int SA_factorExp(int t);
int SA_factorAR(char* name, int t);
int SA_factorVar(char* name, int varnum);

int SA_termm(int f1, int f2);
int SA_termd(int f1, int f2);

int SA_expopp(int t);
int SA_expadd(int t1, int t2);
int SA_expsub(int t1, int t2);

int SA_assignarr(char* name, int re1, int re2);
int SA_assignvar(char* name, int re2);

int SA_condition(int e1, int type, int e2);
int SA_bz();
int SA_bnz(int l);
int SA_goto();
int SA_gotoswend(int l);
int SA_do();
int SA_label(int l);
int SA_case(int re1, int re2);

int SA_pushpara(int t);
int SA_pushover();

int SA_printfstr();
int SA_printfreg(int re, int flag);
int SA_scanf(char* GrIden);
int SA_printfid(char* GrIden);

int SA_return(int re);
int SA_noreturn();

int SA_printInit();
int SA_printString();

typedef struct MidFour {
	int op;
	int num1;
	int num2;
	int result;
	int part;
}MidFour;
extern MidFour midResult[MAXMID];

extern int midresultpos;
int SA_initmid();
int SA_printMIPSend();