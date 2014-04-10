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

	if(instr->opcode == LDC_OP) return false;
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
		cout<<endl;
		//cfg->fullPrint();
		cout<<endl;
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
		debugDump(i,LI);
		moveCodeToPreheader(cfg->getLoopStart(i)-1, LI,i);
		delete cfg;
		delete rd;
		delete udChain;
		cfg = NULL;
		rd = NULL;
		udChain = NULL;
	}
}

void LICM::findConstant(simple_instr *instr, set<void*> &LI){
	if(!isDef(instr)) return ;

	if(instr->opcode == LDC_OP) return;
	else{
		int numOfOp = findNumOfOps(instr);
		assert(numOfOp != -1);
		if(numOfOp == 1){
			checkRegIsConst(instr->u.base.src1, LI);
		} else  {
			assert(numOfOp == 2);
			checkRegIsConst(instr->u.base.src1, LI);
			checkRegIsConst(instr->u.base.src2, LI);
		}
	}
}
void LICM::checkRegIsConst(simple_reg *reg, set<void*> &LI){
	if(reg->kind == TEMP_REG){
		simple_instr *ldcInstr = findTempDefInstr(reg->num);
		assert(ldcInstr->opcode == LDC_OP);
		LI.insert((void*)ldcInstr);
	}
}
simple_reg *LICM::findTargetReg(simple_instr *instr){
	if(isDef(instr)){
		if(instr->opcode == LDC_OP) return instr->u.ldc.dst;
		else return instr->u.base.dst;
	}
	return NULL;
}
void LICM::moveCodeToPreheader(int preHeaderIndex, set<int> LI, int loopIndex){
	set<void*> confirmedToMove;
	set<int> loop = (cfg->getLoopSet())[loopIndex];
	set<int> exitNode = cfg->getExitNode(loopIndex);

	for(set<int>::iterator ite = LI.begin(); ite != LI.end(); ite++){
		/* its gona be move iff
		 1. BB s dominates all exits of L &&
		2. v is not defined elsewhere in L &&
		3. all uses of v in L can only be reached by the
		definition of v in s )
		*/
		bool ifMove = true;
		simple_instr *LIInstr = cfg->findInstrIndex(*ite);
		//1.
		for(set<int>::iterator exitNodeIte = exitNode.begin(); 
												exitNodeIte != exitNode.end(); exitNodeIte++){
			if(!cfg->ifDom(cfg->findBBIndex(*ite),*exitNodeIte)){
				cout<<*ite<<"fail #1"<<endl;
				ifMove = false;
			}
		}
		//2.and3
		simple_reg *targetReg = findTargetReg(LIInstr);
		assert(targetReg != NULL);
		if(targetReg->kind == PSEUDO_REG){
			//loop through all BB in loop
			for(set<int>::iterator loopIte = loop.begin(); 
													loopIte !=loop.end(); loopIte++){
				//loop through all instr in this BB
				simple_instr *start, *end;
				start = this->cfg->getBlockStartInstr(*loopIte);
				end = this->cfg->getBlockEndInstr(*loopIte);
				simple_instr *tracer = start;
				if(start != NULL) while(true){
					//2. v is not defined elsewhere in L &&
					if(tracer != LIInstr && isDef(tracer)){
						simple_reg *targetReg2 = findTargetReg(tracer);
						if(targetReg2->kind == PSEUDO_REG){
							if(targetReg2->num == targetReg->num){
								ifMove = false;
								cout<<*ite<<"fail #2"<<endl;
								//debug//
							}
						}
					}
					//3.
					//check if this Instrction is a Use of this particular variable
					//if so, use UDChain to find all the defs for this use. 
					//Go through all the defs, if the def is defining this variable and
					//it is not our LIInstr, ifMove become false;
					if(isUse(tracer, targetReg)){
						set<int> defSet = udChain->findDefUse(cfg->findIndexInstr(tracer));
						for(set<int>::iterator defIte =defSet.begin();defIte!=defSet.end();defIte++){
							simple_instr *defInstr = cfg->findInstrIndex(*defIte);
							if(defInstr->u.base.dst->num == targetReg->num
									&& defInstr != LIInstr){
								ifMove = false;
								cout<<*ite<<"fail #3"<<endl;
							}
						}
					}
					if(tracer == end) break;
					else tracer = tracer->next;
				}
			}
		}
		if(ifMove) confirmedToMove.insert((void*)LIInstr);
	}
	
	for(set<void*>::iterator confirmedIte = confirmedToMove.begin(); 
						confirmedIte != confirmedToMove.end();confirmedIte++){
		findConstant((simple_instr*)*confirmedIte,confirmedToMove);
	}
	//debug msg
	cout<<"for loop "<<loopIndex<<" instr will be moved is "<<endl;
	for(set<void*>::iterator ite=confirmedToMove.begin();
				ite != confirmedToMove.end();ite++){
		cout<<cfg->findIndexInstr((simple_instr*)*ite)<<":";
		fprint_instr(stdout, (simple_instr*)*ite);
	}
	//debug done
	moveInstr(preHeaderIndex, confirmedToMove);
	//cfg->printInstr();


	return;
}
void LICM::moveInstr(int preHeader, set<void*> confirmedToMove){
	simple_instr *insertPoint = cfg->getBlockStartInstr(preHeader), *tracer = inlist;

	while(tracer){
		if(confirmedToMove.count((void*)tracer) != 0){
			//debug
			cout<<"start moving instr \t";
			fprint_instr(stdout,tracer);
			cout.flush();
			//debugdone

			if(tracer->opcode != LDC_OP 
						&& tracer->u.base.dst->kind == TEMP_REG){
				simple_reg *oldReg = tracer->u.base.dst;
				simple_reg *newReg = new_register(tracer->type,PSEUDO_REG);
				tracer->u.base.dst = newReg;
				replaceReg(oldReg,newReg);
			}
				
			simple_instr *temp1,*temp2;
			temp1 = tracer->prev;
			temp2 = tracer->next;
			temp1->next = temp2;
			temp2->prev = temp1;

			tracer->next = insertPoint->next;
			tracer->prev = insertPoint;
			insertPoint->next = tracer;

			insertPoint = insertPoint->next;
			tracer = temp2;
			cout<<" done"<<endl;//debug
			cout.flush();

		}else{
			tracer = tracer->next;
		}
	}
}
			
	
void LICM::debugDump(int loopIndex, set<int> LI){
	cout<<"loop # "<<loopIndex<<" LI are:"<<endl;
	for(set<int>::iterator ite = LI.begin(); ite != LI.end(); ite++){
		cout<<*ite<<": ";
		fprint_instr(stdout,cfg->findInstrIndex(*ite));
	}
	cout<<endl;
}

bool LICM::cmpReg(simple_reg *reg1, simple_reg *reg2){
	if(reg1->kind == reg2->kind 
			&& reg1->num == reg2->num){
		return true;
	}

	return false;
}
bool LICM::isUse(simple_instr *tracer, simple_reg *reg){
	bool result = true;
	switch (tracer->opcode) {
		case LOAD_OP: {
			 result &= cmpReg(reg,tracer->u.base.src1);
			 break;
		}
		case STR_OP: {
			 result &= cmpReg(reg,tracer->u.base.src1);
			 result &= cmpReg(reg,tracer->u.base.src2);
			 break;
		}
		case MCPY_OP: {
			 result &= cmpReg(reg,tracer->u.base.src1);
			 result &= cmpReg(reg,tracer->u.base.src2);
			 break;
		}
		case LDC_OP: {
			//check
			 break;
		}
		case JMP_OP:{
			break;
		}
		case BTRUE_OP:
		case BFALSE_OP: {
			 result &= cmpReg(reg,tracer->u.bj.src);
			 break;
		}
		case CALL_OP: {
			 unsigned n, nargs;
			 result &= cmpReg(reg,tracer->u.call.proc);
			 nargs = tracer->u.call.nargs;
			 if (nargs != 0) {
			for (n = 0; n < nargs - 1; n++) {
				 result &= cmpReg(reg,tracer->u.call.args[n]);
			}
			result &= cmpReg(reg,tracer->u.call.args[nargs - 1]);
			 }
			 break;
		}
		case MBR_OP: {
			 result &= cmpReg(reg,tracer->u.mbr.src);
			 break;
		}
		case LABEL_OP:{
			break;
		}
		case RET_OP: {
			 if (tracer->u.base.src1 != NO_REGISTER) {
				result &= cmpReg(reg,tracer->u.base.src1);
			 }
			 break;
		}

		case CVT_OP:
		case CPY_OP:
		case NEG_OP:
		case NOT_OP: {
			 /* unary base instructions */
			 result &= cmpReg(reg,tracer->u.base.src1);
			 break;
		}

		default: {
			 /* binary base instructions */
			 result &= cmpReg(reg,tracer->u.base.src1);
			 result &= cmpReg(reg,tracer->u.base.src2);
		}
		return result;
	}
}


LICM::~LICM(){
	if(cfg!= NULL) delete cfg;
	if(udChain != NULL) delete udChain;
	if(rd !=  NULL) delete rd;
}


void LICM::replaceReg(simple_reg *oldReg, simple_reg *newReg){
	assert(oldReg->kind == TEMP_REG);

	simple_instr *tracer = inlist;
	while(tracer){
		ifUseReplace(tracer, oldReg, newReg);
		tracer = tracer->next;
	}
}
			


void LICM::ifUseReplace(simple_instr *tracer, simple_reg *reg, 
									simple_reg *newReg){
	bool result = true;
	switch (tracer->opcode) {
		case LOAD_OP: {
			 result = cmpReg(reg,tracer->u.base.src1);
			 if(result)tracer->u.base.src1 = newReg;

			 break;
		}
		case STR_OP: {
			 result = cmpReg(reg,tracer->u.base.src1);
			 if(result)tracer->u.base.src1 = newReg;
			 result = cmpReg(reg,tracer->u.base.src2);
			 if(result)tracer->u.base.src2 = newReg;
			 break;
		}
		case MCPY_OP: {
			 result = cmpReg(reg,tracer->u.base.src1);
			 if(result)tracer->u.base.src1 = newReg;
			 result = cmpReg(reg,tracer->u.base.src2);
			 if(result)tracer->u.base.src2 = newReg;
			 break;
		}
		case LDC_OP: {
			//check
			 break;
		}
		case JMP_OP:{
			break;
		}
		case BTRUE_OP:
		case BFALSE_OP: {
			 result = cmpReg(reg,tracer->u.bj.src);
			 if(result)tracer->u.bj.src = newReg;
			 break;
		}
		case CALL_OP: {
			 unsigned n, nargs;
			 result = cmpReg(reg,tracer->u.call.proc);
			 nargs = tracer->u.call.nargs;
			 if (nargs != 0) {
			for (n = 0; n < nargs - 1; n++) {
				 result = cmpReg(reg,tracer->u.call.args[n]);
				 if(result)tracer->u.call.args[n] = newReg;
			}
			result = cmpReg(reg,tracer->u.call.args[nargs - 1]);
			 if(result)tracer->u.call.args[nargs - 1] = newReg;
			 }
			 break;
		}
		case MBR_OP: {
			 result = cmpReg(reg,tracer->u.mbr.src);
			 if(result)tracer->u.mbr.src= newReg;
			 break;
		}
		case LABEL_OP:{
			break;
		}
		case RET_OP: {
			 if (tracer->u.base.src1 != NO_REGISTER) {
				result = cmpReg(reg,tracer->u.base.src1);
				 if(result)tracer->u.base.src1= newReg;
			 }
			 break;
		}

		case CVT_OP:
		case CPY_OP:
		case NEG_OP:
		case NOT_OP: {
			 /* unary base instructions */
			 result = cmpReg(reg,tracer->u.base.src1);
			 if(result)tracer->u.base.src1= newReg;
			 break;
		}

		default: {
			 /* binary base instructions */
			 result = cmpReg(reg,tracer->u.base.src1);
				 if(result)tracer->u.base.src1= newReg;
			 result = cmpReg(reg,tracer->u.base.src2);
				 if(result)tracer->u.base.src2= newReg;
		}
	}
}


