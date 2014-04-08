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
			//its binary or unary ops
			return true; 
	}
	return false;
}

int LICM::findNumOfOps(instr){
	switch (instr->opcode) {
		case LOAD_OP: 
		case STR_OP: 
		case MCPY_OP: 
		case LDC_OP:
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
		case NOT_OP: {
			return 1;
			break;
		}
		default:
			return 2;

	}
	return false;
}


bool LICM::checkRegLi(simple_reg *reg, set<int> defSet,set<int> loop){
	bool return_value = true;
	if(reg->kind == PESUDO_REG){
		for(set<int>::iterator ite=defSet.begin();ite!=defSet.end();ite++){
			simple_instr *defInstr = cfg->findInstrIndex(*ite);
			//check all its def instrctions, the return value is false when:
			//			its inside the loop and it is not one of the LI yet
			if(defInstr->u.base.dst->num == reg->num 
					&& loop.count(findBBInstr(defInstr)) != 0
					&& LI.count(findIndexInstr(defInstr)) == 0){
				return_value = false;
			}
		}
	}else if(reg->kind == TEMP_REG){
		simple_instr *defInstr = findTempDefInstr(reg->num);
		if(loop.count(findBBInstr(defInstr)) != 0
			&& LI.count(findIndexInstr(defInstr))==0){
			return_value = false;
		}
	}else {
		assert(false);
	}
	return return_value;
}
		
		




bool LICM::opIsIV(simple_instr *instr, set<int> LI){
	if(!isDef(instr)) return false;

	if(instr->opcode == LDC_OP) return true;
	else{
		set<int> defSet = udChain.findDefUse(cfg->findIndexInstr(instr));
		int numOfOp = findNumOfOps(instr);
		//for binary and unary instruction, check each of its registors
		//return true if:
		//	   all of its PESUDO_REG's definition is in LI or outside of 
		//		this loop. 
		//    AND all of its TMP_REG's definition is outside of the loop
		//		or is in LI
		bool return_value = true;
		if(numOfOp == 1){
			return_value &= checkRegIsLI(instr->u.base.src1);
		} else {
			assert(numOfOp == 2);
			return_value &= checkRegIsLI(instr->u.base.src1);
			return_value &= checkRegIsLI(instr->u.base.src2);
		}
	}
	return return_value;
}

void LICM::start(){
	cfg->genLoopSet();
	map<int, set<int>>loopSet = cfg->getLoopSet();
	loopNum = cfg->getLoopNum();

	for(int i = 0; i < loopNum; i++){
		set<int> loop = loopSet[i];
		bool changed = true;
		//regenerate the RDChain 
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

			//for each BB in this block, check each instruction if its LI.
			//if so, add to the set and changed become true;
			for(set<int>::iterator ite = loop.begin(); ite != loop.end();ite++){
				simple_instr *start, *end;
				start = this->cfg->getBlockStartInstr(blockIndex);
				end = this->cfg->getBlockEndInstr(blockIndex);
				simple_instr *tracer = start;
				assert(start != NULL);
				while(true){
					if(opIsIV(tracer,LI)){ //todo
						LI.insert(cfg->getInstrNum(tracer));
						changed = true;
					}
					if(tracer == end) break;
					else tracer = tracer->next;
				}
			}
		}
		moveCodeToPreheader(loop[0] - 1, LI);
		delete cfg;
		delete rd;
		delete udChain;
	}
}

			



				

				 


			 









