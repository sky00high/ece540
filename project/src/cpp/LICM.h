#ifndef LICM_H
#define LICM_H
#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include <assert.h>
#include "Definition.h"
#include "UDChain.h"
extern "C"{
	#include <simple.h>
}
class LICM{
	simple_instr *inlist;
	CFG *cfg;
	UDChain *udChain;
	RD *rd;

	bool isDef(simple_instr *instr);
	int findNumOfOps(simple_instr *instr);
	bool opIsLI(simple_instr *instr, set<int> LI, set<int> loop);
	bool opIsLI(simple_instr *instr, set<int>LI);
	bool checkRegIsLI(simple_reg *reg, set<int> defSet,set<int> loop,
							 set<int> LI); 
	simple_instr *findTempDefInstr(int varNum);
	void moveCodeToPreheader(int preHeaderIndex, set<int> LI,int loopIndex);
	void debugDump(int loopIndex, set<int> LI);
	simple_reg *findTargetReg(simple_instr *instr);
	bool cmpReg(simple_reg *reg1, simple_reg *reg2);
	bool isUse(simple_instr *instr, simple_reg *reg);
	void moveInstr(int preHeader, set<void*> confirmedToMove);

	public:
		LICM(simple_instr *inlist);
		~LICM();
		void start();







};

#endif
