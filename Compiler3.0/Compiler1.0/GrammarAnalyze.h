#pragma once
#define MAXCODELENGTH	40960
#define MAXTOKEN 64

#define MAXVARLISTNUM 128

#define MAXWORDNUM 4096

#define CASESY			1 //case
#define CHARSY			2 //char
#define CONSTSY			3 //const
#define DEFAULTSY		4 //default
#define DOSY			5 //do
#define ELSESY			6 //else
#define IFSY			7 //if
#define INTSY			8 //int
#define MAINSY			9 //main
#define PRINTFSY		10//printf
#define RETURNSY		11//return
#define SCANFSY			12//scanf
#define SWITCHSY		13//switch
#define VOIDSY			14//void
#define WHILESY			15//while

#define SINGLEQUOTESY	16//'
#define MINUSSY			17//-
#define NEQSY			18//!=
#define DOUBLEEQUOTESY	19//"
#define LPARSY			20//(
#define RPARSY			21//)
#define MULTISY			22//*
#define COMMASY			23//,
#define DIVSY			24// /
#define SEMICOLONSY		25//;
#define LBRACKETSY		26//[
#define RBRACKETSY		27//]
#define LBRACESY		28//{
#define RBRACESY		29//}
#define ADDSY			30//+
#define LESSSY			31//<
#define LESSEQSY		32//<=
#define ASSIGNSY		33//=
#define EQSY			34//==
#define MORESY			35//>
#define MOREEQSY		36//>=
#define COLONSY			37//:

#define IDSY			38
#define NUMSY			39
#define STRSY			40
#define CHSY			41

#define CONSTANTEX		42
#define ARRAYEX			43
#define VAREX			44
#define FUNEX			45
#define PARAEX			46
#define TEMPEX			47

#define NOPSY           48

int grammerAna();