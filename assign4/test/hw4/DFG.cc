using namespace std;
extern "C"{
	#include <simple.h>
}

#include <assert.h>
#include "BitVector.h"
#include <iostream>
#include <map>
#include "DFG.h"
#include "cfg.h"


//part1
simple_instr *DFG::findInstr(int index){
	return this->cfg->findInstr(index);
}
void DFG::checkReg(simple_reg *lereg){
	
	if(lereg->kind == PSEUDO_REG){
		if(varMap.count(lereg->num) == 0){
			varMap[lereg->num] = varNum + 1;
			varNum++;
//			cout<<"found r"<<lereg->num<<" index is "<<varNum<<endl;
		}
	}
}
	
void DFG::findVar(){
	simple_instr *tracer = inlist;
	while(tracer){
		switch (tracer->opcode) {
			case LOAD_OP: {
				 checkReg( tracer->u.base.dst);
				 checkReg( tracer->u.base.src1);
				 break;
			}
			case STR_OP: {
				 checkReg( tracer->u.base.src1);
				 checkReg( tracer->u.base.src2);
				 break;
			}

			case MCPY_OP: {
				 checkReg( tracer->u.base.src1);
				 checkReg( tracer->u.base.src2);
				 break;
			}

			case LDC_OP: {
				 checkReg( tracer->u.ldc.dst);
				 break;
			}
			case JMP_OP:{
				break;
			}
			case BTRUE_OP:
			case BFALSE_OP: {
				 checkReg( tracer->u.bj.src);
				 break;
			}

			case CALL_OP: {
				 unsigned n, nargs;
				 if (tracer->u.call.dst != NO_REGISTER) {
				checkReg( tracer->u.call.dst);
				 }
				 checkReg( tracer->u.call.proc);
				 nargs = tracer->u.call.nargs;
				 if (nargs != 0) {
				for (n = 0; n < nargs - 1; n++) {
					 checkReg( tracer->u.call.args[n]);
				}
				checkReg( tracer->u.call.args[nargs - 1]);
				 }
				 break;
			}
			case MBR_OP: {
				 checkReg( tracer->u.mbr.src);
				 break;
			}
			case LABEL_OP:{
				break;
			}
			case RET_OP: {
				 if (tracer->u.base.src1 != NO_REGISTER) {
					checkReg( tracer->u.base.src1);
				 }
				 break;
			}

			case CVT_OP:
			case CPY_OP:
			case NEG_OP:
			case NOT_OP: {
				 /* unary base instructions */
				 checkReg( tracer->u.base.dst);
				 checkReg( tracer->u.base.src1);
				 break;
			}

			default: {
				 /* binary base instructions */
				 checkReg( tracer->u.base.dst);
				 checkReg( tracer->u.base.src1);
				 checkReg( tracer->u.base.src2);
			}
		}
		tracer = tracer->next;
	}
}

int DFG::findVarIndex(int index){
	for(int i = 1; i <= varNum; i++){
		if(varMap[i] == index) return i;
	}
	//shouldn't be here
	assert(false);
	return -1;
}
void DFG::printVarList(){
	for(int i = 1; i <= varNum; i++){
		cout<<"var "<<i<<" r"<<findVarIndex(i)<<endl;
	}
}
//part2

void DFG::genDefBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		switch (tracer->opcode) {
			case LOAD_OP: {
				if(tracer->u.base.dst->kind == PSEUDO_REG)
					defSet[blockIndex].set(varMap[tracer->u.base.dst->num]- 1);
				 break;
			}
			case STR_OP: {
				 break;
			}

			case MCPY_OP: {
				 break;
			}

			case LDC_OP: {
				//cout<<"block index "<<blockIndex<<" variable num "<<tracer->u.ldc.dst->num<<" index "<<varMap[tracer->u.ldc.dst->num]<<endl;
				if(tracer->u.ldc.dst->kind == PSEUDO_REG)
					defSet[blockIndex].set(varMap[tracer->u.ldc.dst->num]- 1);
				//set_bit(defSet[blockIndex],varMap[tracer->u.ldc.dst->num] - 1,true);
				 break;
			}
			case JMP_OP:{
				break;
			}
			case BTRUE_OP:
			case BFALSE_OP: {
				 break;
			}

			case CALL_OP: {
				 unsigned n, nargs;
				 if (tracer->u.call.dst != NO_REGISTER) {
					if(tracer->u.call.dst->kind == PSEUDO_REG)
						defSet[blockIndex].set(varMap[tracer->u.call.dst->num]- 1);
					//set_bit(defSet[blockIndex],varMap[tracer->u.call.dst->num] - 1,true);
				 }
				 break;
			}
			case MBR_OP: {
				 break;
			}
			case LABEL_OP:{
				break;
			}
			case RET_OP: {
				 break;
			}

			case CVT_OP:
			case CPY_OP:
			case NEG_OP:
			case NOT_OP: {
					if(tracer->u.base.dst->kind == PSEUDO_REG)
						defSet[blockIndex].set(varMap[tracer->u.base.dst->num]- 1);
					//set_bit(defSet[blockIndex],varMap[tracer->u.base.dst->num] - 1,true);
				 break;
			}

			default: {
					if(tracer->u.base.dst->kind == PSEUDO_REG)
						defSet[blockIndex].set(varMap[tracer->u.base.dst->num]- 1);
				 /* binary base instructions */
			}
		}

		if(tracer ==  end) break;
		else tracer = tracer->next;


	}

}
void DFG::iniDefSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		this->defSet[i].ini(varNum);
	}
}


//part3
void DFG::checkRegUse(simple_instr *start, simple_instr *_end,BitVector &useVec, simple_reg *r){
	
	if(r->kind == PSEUDO_REG){
		simple_instr *end = _end;
		bool skip = start == end;
		if(!skip)end = end->prev;
      simple_instr *tracer = start;
		bool defAfter = false;
		int var = r->num;
		//cout<<"r"<<r->num<<" is used";
		if(!skip)while(true){
			//take case the situation this is the firstinstr
				switch (tracer->opcode) {
					case LOAD_OP: {
						if(tracer->u.base.dst->kind == PSEUDO_REG &&
							 tracer->u.base.dst->num == var)
							defAfter = true;
						 break;
					}
					case STR_OP:
					case MCPY_OP: break;
					case LDC_OP: {
						if(tracer->u.ldc.dst->kind == PSEUDO_REG &&
							 tracer->u.ldc.dst->num == var)
							defAfter = true;
						 break;
					}
					case JMP_OP:
					case BTRUE_OP:
					case BFALSE_OP: break;
					case CALL_OP: {
						 unsigned n, nargs;
						 if (tracer->u.call.dst != NO_REGISTER) {
							if(tracer->u.call.dst->kind == PSEUDO_REG &&
								 tracer->u.call.dst->num == var)
								defAfter = true;
						 }
						 break;
					}
					case MBR_OP: 
					case LABEL_OP:
					case RET_OP:break; 
					case CVT_OP:
					case CPY_OP:
					case NEG_OP:
					case NOT_OP:{ 
						if(tracer->u.base.dst->kind == PSEUDO_REG &&
							 tracer->u.base.dst->num == var)
							defAfter = true;
						 break;
					}

					default: {
						if(tracer->u.base.dst->kind == PSEUDO_REG &&
							 tracer->u.base.dst->num == var)
							defAfter = true;
						 /* binary base instructions */
					}
				}
			if(tracer ==  end) break;
			else tracer = tracer->next;
		}
		if(!defAfter)useVec.set(varMap[r->num] - 1);

		//if(defAfter) cout<<" but its defined afterwards";
		//else cout<< " and its not defiend afterwards";
		//cout<<endl;
	}

}
void DFG::genUseBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL)while(true){
		switch (tracer->opcode) {
			case LOAD_OP: {
				checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src1);
				 break;
			}
			case STR_OP: {
				checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src1);
				checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src2);
				 break;
			}

			case MCPY_OP: {
				checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src1);
				checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src2);
				 break;
			}

			case LDC_OP: {
				 break;
			}
			case JMP_OP:{
				break;
			}
			case BTRUE_OP:
			case BFALSE_OP: {
				checkRegUse(start,tracer,useSet[blockIndex],tracer->u.bj.src);
				 break;
			}

			case CALL_OP: {
				 unsigned n, nargs;
				 checkRegUse(start,tracer,useSet[blockIndex],tracer->u.call.proc);
				 nargs = tracer->u.call.nargs;
				 if (nargs != 0) {
					for (n = 0; n < nargs - 1; n++) {
						 checkRegUse(start,tracer,useSet[blockIndex],tracer->u.call.args[n]);
					}
					 checkRegUse(start,tracer,useSet[blockIndex],tracer->u.call.args[nargs-1]);
				 }
				 break;
			}
			case MBR_OP: {
				 checkRegUse(start,tracer,useSet[blockIndex],tracer->u.mbr.src);
				 break;
			}
			case LABEL_OP:{
				break;
			}
			case RET_OP: {
				 if (tracer->u.base.src1 != NO_REGISTER) {
					 checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src1);
				 }
				 break;
			}

			case CVT_OP:
			case CPY_OP:
			case NEG_OP:
			case NOT_OP: {
					checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src1);
					//set_bit(useSet[blockIndex],varMap[tracer->u.base.dst->num] - 1,true);
				 break;
			}

			default: {
					checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src1);
					checkRegUse(start,tracer,useSet[blockIndex],tracer->u.base.src2);
				 /* binary base instructions */
			}
		}

		if(tracer ==  end) break;
		else tracer = tracer->next;


	}

}
void DFG::iniUseSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum();i++){
		this->useSet[i].ini(varNum);
	}
}

void DFG::iniLiveOut(){
	for(int i = 0; i < this->cfg->getTotalBlockNum();i++){
		this->liveOut[i].ini(varNum);
	}
}
void DFG::iniLiveIn(){
	for(int i = 0; i < this->cfg->getTotalBlockNum();i++){
		this->liveIn[i].ini(varNum);
	}
}


//public 
DFG::DFG(CFG *_cfg, simple_instr *_inlist){
	assert(_cfg != NULL);
	assert(_inlist != NULL);
	this->cfg = _cfg;
	this->inlist = _inlist;
	this->varNum = 0;
}

void DFG::generateVarList(){
	findVar();
}
void DFG::genDefSet(){
	iniDefSet();
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		genDefBlock(i);
	}
}


void DFG::printDefSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		cout<<"def_lv " <<i<<" "<<defSet[i]<<endl;
	}
}

void DFG::genUseSet(){
	iniUseSet();
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		genUseBlock(i);
	}
}

void DFG::printUseSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		cout<<"use_lv " <<i<<" "<<useSet[i]<<endl;
	}
}



void DFG::genLiveOut(){
	iniLiveOut();
	iniLiveIn();
	
	bool changed = true;
	BitVector newOut;
	while(changed){
		changed = false;
		for(int i = 1; i < this->cfg->getTotalBlockNum() - 1; i++){
			BitVector oldIn = liveIn[i];

			newOut.ini(varNum);
			map<int,int>succ = cfg->getBlockSucc(i);

			for(map<int,int>::const_iterator ii = succ.begin(); 
				ii != succ.end(); ii++){
				newOut += liveIn[(*ii).first];
			}

			liveOut[i] = newOut;
			newOut-=defSet[i];
			newOut+=useSet[i];
			liveIn[i] = newOut;
			//liveIn[i] = liveOut[i] - defSet[i] + useSet[i];
			if(liveIn[i].value != oldIn.value) changed = true;
		}
	}
	newOut.ini(varNum);
	for(map<int,int>::const_iterator ii = cfg->getBlockSucc(0).begin(); 
		ii != cfg->getBlockSucc(0).end(); ii++){
		newOut += liveIn[(*ii).first];
	}
	liveOut[0] = newOut;

}

void DFG::printLiveOut(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		cout <<"live_out " <<i<<" "<<liveOut[i]<<endl;
	}
}
			
			
