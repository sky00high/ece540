#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "RD.h"
#include <assert.h>
#include "Definition.h"
#include "UDChain.h"
extern "C"{
	#include <simple.h>
}

LICM::LICM(simple_instr *inlist, CFG *cfg, UDChain *udChain){
	this->inlist = inlist;
	this->cfg = cfg;
	this->udChain = udChain;
}

bool LICM::isDef(simple_instr *instr){
	switch (instr->opcode) {
		case LOAD_OP: 
		case STR_OP: 
		case MCPY_OP: 
			break;
		case LDC_OP:{
			return true;
			break;
		}
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP: 
		case CALL_OP: 
		case MBR_OP: 
		case LABEL_OP:
		case RET_OP: break;
		case CVT_OP:
		case CPY_OP:
		case NEG_OP:
		case NOT_OP:  
		default:
			else return false; 
	}
	return false;
}

	
void LICM::start(){
	cfg->genLoopSet();
	map<int, set<int>>loopSet = cfg->getLoopSet();
	loopNum = cfg->getLoopNum();

	for(int i = 0; i < loopNum; i++){
		set<int> loop = loopSet[i];
		bool changed = true;
		cfg = new CFG(inlist);
		cfg->findIDom();
		rd = new RD(thisCFG,inlist);
		rd->genDefList();
		rd->genGenSet();
		rd->genKillSet();
		rd->genRDOutSet();
		udChain = new UDChain(inlist, thisCFG, rd);
		set<int> LI;
		while(changed){
			changed = false;

			//regenerate the RDChain 

			//for each BB in this block, check each instruction if its LI.
			//if so, add to the set and changed become true;
			for(set<int>::iterator ite = loop.begin(); ite != loop.end();ite++){
				simple_instr *start, *end;
				start = this->cfg->getBlockStartInstr(blockIndex);
				end = this->cfg->getBlockEndInstr(blockIndex);

				simple_instr *tracer = start;
				assert(start != NULL);
				while(true){
					if(isDef(tracer) && opIsIV(tracer)){ //todo
						LI.insert(cfg->getInstrNum(tracer));
						changed = true;
					}
					if(tracer == end) break;
					else tracer = tracer->next;
				}
			}
		}
		moveCodeToPreheader(loop[0] - 1, LI);
	}
}

			



				

				 


			 









