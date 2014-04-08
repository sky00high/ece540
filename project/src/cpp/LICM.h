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

	bool isDef(simple_instr *instr);
	bool opIsIV(simple_instr *instr);
	void moveCodeToPreheader(int preHeader, set<int> LI);

	public:
		LICM(simple_instr *inlist, CFG *cfg, UDChain *udChain);
		void start();







};

#endif
