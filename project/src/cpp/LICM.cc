#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "RD.h"
#include <assert.h>
#include "Definition.h"
#include "UDChain.h"
#include "LICM.h"
extern "C"{
	#include <simple.h>
	#include "print.h"
}

LICM::LICM(simple_instr *inlist){
	this->inlist = inlist;
}

bool LICM::isDef(simple_instr *instr){
	switch (instr->opcode) {
		case LOAD_OP: 
			return true;
			break;
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

int LICM::findNumOfOps(simple_instr *instr){
	switch (instr->opcode) {
		case LOAD_OP:{
			return 1;
			break;
		}
			
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
	return -1;
}


simple_instr *LICM::findTempDefInstr(int varNum){
	simple_instr *tracer = inlist;
	while(tracer){
		if(isDef(tracer)){
			if(tracer->opcode == LDC_OP
					&& tracer->u.ldc.dst->kind == TEMP_REG
					&& tracer->u.ldc.dst->num == varNum){
				return tracer;
			}else if(tracer->u.base.dst->kind == TEMP_REG
							&& tracer->u.base.dst->num == varNum){
				return tracer;
			} 
		}
		tracer = tracer->next;
	}
	cout<<"didn't find, varNum is "<<varNum<<endl;
	assert(false);
	return NULL;
}

bool LICM::checkRegIsLI(simple_reg *reg, set<int> defSet,set<int> loop, 
									set<int> LI){
	bool return_value = true;
	if(reg->kind == PSEUDO_REG){
		for(set<int>::iterator ite=defSet.begin();ite!=defSet.end();ite++){
			simple_instr *defInstr = cfg->findInstrIndex(*ite);
			//check all its def instrctions, the return value is false when:
			//			its inside the loop and it is not one of the LI yet
			assert(defInstr->u.base.dst->kind == PSEUDO_REG);
			if(defInstr->u.base.dst->num == reg->num 
					//&& loop.count(cfg->findBBInstr(defInstr)) != 0
					//&& LI.count(cfg->findIndexInstr(defInstr)) == 0){
					&& loop.count(cfg->findBBInstr(defInstr)) != 0){
				return_value = false;
			}
		}
	}else if(reg->kind == TEMP_REG){
		simple_instr *defInstr = findTempDefInstr(reg->num);
		/*
		if(loop.count(cfg->findBBInstr(defInstr)) != 0
			&& LI.count(cfg->findIndexInstr(defInstr))==0){
			return_value = false;
		}
		*/
		if(defInstr->opcode != LDC_OP) return_value = false;
	}else {
		assert(false);
	}
	return return_value;
}
		
		




bool LICM::opIsLI(simple_instr *instr, set<int> LI, set<int> loop){
	if(!isDef(instr)) return false;

	if(instr->opcode == LDC_OP) return true;
	else{
		set<int> defSet = udChain->findDefUse(cfg->findIndexInstr(instr));
		int numOfOp = findNumOfOps(instr);
		assert(numOfOp != -1);
		//for binary and unary instruction, check each of its registors
		//return true if:
		//	   all of its PESUDO_REG's definition is in LI or outside of 
		//		this loop. 
		//    AND all of its TMP_REG's definition is outside of the loop
		//		or is in LI
		bool return_value = true;
		if(numOfOp == 1){
			return_value &= checkRegIsLI(instr->u.base.src1,defSet,loop,LI);


		} else {
			assert(numOfOp == 2);
			return_value &= checkRegIsLI(instr->u.base.src1,defSet,loop,LI);
			return_value &= checkRegIsLI(instr->u.base.src2,defSet,loop,LI);
		}
		return return_value;
	}
}

void LICM::start(){
	cfg = new CFG(inlist);
	cfg->findIDom();
	cfg->findREdge();
	cfg->genLoopSet();
	//map<int, set<int> >loopSet = cfg->getLoopSet();
	int loopNum = cfg->getLoopNum();
	delete cfg;
	cfg = NULL;

	for(int i = 0; i < loopNum; i++){
		bool changed = true;
		//regenerate the RDChain 
		cfg = new CFG(inlist);
		cfg->findIDom();
		cfg->findREdge();
		set<int> loop = (cfg->getLoopSet())[i];
		rd = new RD(cfg,inlist);
		rd->genDefList();
		rd->genGenSet();
		rd->genKillSet();
		rd->genRDOutSet();
		udChain = new UDChain(inlist, cfg, rd);
		set<int> LI;
		while(changed){
			changed = false;

			//for each BB in this block, check each instruction if its LI.
			//if so, add to the set and changed become true;
			for(set<int>::iterator ite = loop.begin(); ite != loop.end();ite++){
				simple_instr *start, *end;
				start = this->cfg->getBlockStartInstr(*ite);
				end = this->cfg->getBlockEndInstr(*ite);
				simple_instr *tracer = start;
				assert(start != NULL);
				while(true){
					if(opIsLI(tracer,LI,loop)){ //todo
						if(LI.count( cfg->findIndexInstr(tracer)) == 0){
							LI.insert(cfg->findIndexInstr(tracer));
							changed = true;
						}
					}
					if(tracer == end) break;
					else tracer = tracer->next;
				}
			}
		}
		set<int>::iterator ite = loop.begin();
		moveCodeToPreheader(*ite - 1, LI);
		debugDump(i,LI);
		delete cfg;
		delete rd;
		delete udChain;
		cfg = NULL;
		rd = NULL;
		udChain = NULL;
	}
}

void LICM::moveCodeToPreheader(int preHeaderIndex, set<int> LI){
	set<void*> confirmedToMove;
	for(set<int>::iterator ite = LI.begin(); ite != LI.end(); ite++){
		/* its gona be move iff
		 1. BB s dominates all exits of L &&
		2. v is not defined elsewhere in L &&
		3. all uses of v in L can only be reached by the
		definition of v in s )
		*/
	}

		

	
	return;
}
			
void LICM::debugDump(int loopIndex, set<int> LI){
	cout<<"loop # "<<loopIndex<<" LI are:"<<endl;
	for(set<int>::iterator ite = LI.begin(); ite != LI.end(); ite++){
		cout<<*ite<<": ";
		fprint_instr(stdout,cfg->findInstrIndex(*ite));
	}
	cout<<endl;
}
