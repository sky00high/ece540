#ifndef RE_H
#define RE_H 
#include "BitVector.h"
#include <assert.h>
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "Expression.h"
extern "C"{
	#include <simple.h>
}

class RE {
	CFG *cfg;
   map <Expression, int> exprMap;
	simple_instr *inlist;
	
	map<int,BitVector> evalSet;
	map<int, BitVector> killSet;
	map<int, BitVector> REOutSet;
	
	bool isCVT(simple_instr*);
	//part1 helpers
	simple_instr *findInstr(int);
	Expression findExprIndex(int);
	bool isBinaryInstr(simple_instr *);
	bool isPseudoReg(simple_reg*);
	//part2 helpers
	void iniEvalSet();
	bool isExpr(simple_instr*);
	void genEvalBlock(int);
	bool exprIsKilled(simple_instr *instr, Expression expr);
	//part3 helpers
	void iniKillSet();
	void genKillBlock(int);

	public:
	RE(CFG *cfg, simple_instr *inlist);

	//part1
	void genExprList();
	void printExprList();
	int getExprNum(){return exprNum;}

	//part2
	void genEvalSet();
	void printEvalSet();

	//part3
	void genKillSet();
	void printKillSet();

		






};

#endif
	
