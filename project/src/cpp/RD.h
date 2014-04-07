#ifndef RD_H
#define RD_H 
#include "BitVector.h"
#include <assert.h>
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "Definition.h"
extern "C"{
	#include <simple.h>
}

class RD {
	CFG *cfg;
   map <Definition, int> defMap;
	simple_instr *inlist;
	
	map<int,BitVector> evalSet;
	map<int, BitVector> killSet;
	map<int, BitVector> RDOutSet;
	
	simple_instr *findInstr(int);
	bool isPseudoReg(simple_reg*);
	int defNum;
	bool isDef(simple_instr *instr);
	Definition findDefIndex(int index);
	/*
	void iniEvalSet();
	bool isExpr(simple_instr*);
	void genEvalBlock(int);
	bool exprIsKilled(simple_instr *instr, Expression expr);
	//part3 helpers
	void iniKillSet();
	void genKillBlock(int);
*/
	public:
	RD(CFG *cfg, simple_instr *inlist);

	//part1
	void genDefList();
	void printDefList();
	int getDefNum(){return defNum;}
/*
	//part2
	void genEvalSet();
	void printEvalSet();

	//part3
	void genKillSet();
	void printKillSet();

*/		

};

#endif
	
