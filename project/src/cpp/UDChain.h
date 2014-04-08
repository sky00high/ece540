#ifndef UDCHAIN_H
#define UDCHAIN_H

#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "RD.h"
#include <assert.h>
#include "Definition.h"
extern "C"{
	#include <simple.h>
}

class UDChain{
	int instrNum;
	bool **UDGraph;
	simple_instr *inlist;
	CFG *cfg;
	RD *rd;
	void checkReg(simple_instr *instr,int blockIndex,simple_reg *lereg);
	void checkVar(int blockIndex, simple_instr *tracer);
	void buildUDChainBlock(int blockIndex);



	public:
		UDChain(simple_instr *inlist, CFG *cfg, RD *rd); 
		~UDChain();
		
		void setEdge(int start, int end);
		void clearEdge(int start, int end);

		bool isEdgeSet(int start, int end);




};
#endif
