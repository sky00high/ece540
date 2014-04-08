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
	
	map<int, BitVector> genSet;
	map<int, BitVector> killSet;
	map<int, BitVector> RDOutSet;
	map<int, BitVector> RDInSet;
	
	simple_instr *findInstr(int);
	bool isPseudoReg(simple_reg*);
	int defNum;
	Definition findDefIndex(int index);

   //genGen Helpers
	void iniGenSet();
	void genGenBlock(int);

	//killGen Helpers

	void iniKillSet();
	int getIndexInstr(simple_instr *instr);
	bool defIsRedefined(simple_instr *instr, int var);
	bool instrSameDest(simple_instr *lhs, simple_instr *rhs);
	void genKillBlock(int BlockIndex);

	public:
		map<Definition, int> getDefMap(){ return defMap;}
		RD(CFG *cfg, simple_instr *inlist);

		//part1
		bool isDef(simple_instr *instr);
		void genDefList();
		void printDefList();
		int getDefNum(){return defNum;}
		//Genset public
		void genGenSet();
		void printGenSet();
		//Killset public
		bool instrInBB(simple_instr *instr,int blockIndex);
		void genKillSet();
		void printKillSet();
		//RDout
		void genRDOutSet();
		void printRDOutSet();

		//other

		map<int, BitVector> getRDInSet(){return RDInSet;}
		
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
	
