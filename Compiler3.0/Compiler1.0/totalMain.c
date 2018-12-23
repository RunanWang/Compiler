#include "GrammarAnalyze.h"
#include "WordAnalyze.h"
#include "SemaAnalyze.h"
#include "VarList.h"
#include "MidToMIPS.h"
#include "Optimize.h"

int main() {
	SA_initmid();
	SA_printInit();
	grammerAna();
	printTable();
	FI_printMIPS();
	SA_printMIPSend();
	Optimize();
	printf("____Finished!____\n");
	return 0;
}