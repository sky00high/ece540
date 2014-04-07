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
	
	map<int,BitVector> genSet;
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
   //genGen Helpers
	void iniGenSet();
	void genGenBlock(int);

	//killGen Helpers

	void iniKillSet();
	bool instrInBB(simple_instr *instr,int blockIndex);
	int getIndexInstr(simple_instr *instr);
	bool defIsRedefined(simple_instr *instr, int var);
	bool instrSameDest(simple_instr *lhs, simple_instr *rhs);
	void genKillBlock(int BlockIndex);

	public:
		RD(CFG *cfg, simple_instr *inlist);

		//part1
		void genDefList();
		void printDefList();
		int getDefNum(){return defNum;}
	//Genset public
		void genGenSet();
		void printGenSet();
		//Killset public
		void genKillSet();
		void printKillSet();
		//RDout
		void genRDOutSet();
		void printRDOutSet();
		
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
	
