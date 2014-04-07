#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "RD.h"
#include <assert.h>
#include "DFA.h"
#include "Definition.h"
extern "C"{
	#include <simple.h>
}

using namespace std;

RD::RD(CFG *cfg, simple_instr *inlist){
	assert(cfg != NULL);
	assert(inlist != NULL);

	this->cfg = cfg;
	this->inlist = inlist;
	this->defNum = 0;
}

simple_instr *RD::findInstr(int index){
	return this->cfg->findInstr(index);
}
bool RD::isPseudoReg(simple_reg *reg){

	return reg->kind ==  PSEUDO_REG;
}

bool RD::isDef(simple_instr *instr){
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
		case NOT_OP:  
		default:
			if(isPseudoReg(instr->u.base.dst))return true;
			else return false; 
	}
	return false;
}

	

void RD::genDefList(){
	simple_instr *tracer = inlist;
	int instrIndex = 0;


	while(tracer){
		if(isDef(tracer)){
			Definition newDef(tracer,instrIndex);
			defMap[newDef] = ++defNum;
		}
		tracer = tracer->next;
		instrIndex++;
	}

}

Definition RD::findDefIndex(int index){
	for(map<Definition,int>::const_iterator ite = defMap.begin();
		ite != defMap.end(); ite++){
		if(ite->second == index) return ite->first;
	}
}

void RD::printDefList(){
	for(int i = 1; i <= defNum; i++){
		cout<<"def "<<i<<" "<<findDefIndex(i).instrIndex<<endl;
	}
}

//genSet
//helpsers
void RD::iniGenSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		this->genSet[i].ini(defNum);
	}
}
	
int RD::getIndexInstr(simple_instr *instr){
	simple_instr *tracer = inlist;
	int index = 0;

	while(tracer != NULL){
		if(tracer == instr) return index;
		tracer = tracer->next;
		index ++;
	}
}

bool RD::defIsRedefined(simple_instr *instr, int var){
	assert(isDef(instr));
	return instr->u.base.dst->num == var;
}

void RD::genGenBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		if(isDef(tracer)){
			bool isDefinedAfter = false;
			simple_instr *tracer2 = tracer;
			Definition newDef(tracer,getIndexInstr(tracer));
			int var = tracer->u.base.dst->num;
			while(tracer2!=end->next){
				if(defIsRedefined(tracer2,var)){
					isDefinedAfter = true;
				}
				tracer2 = tracer2->next;
			}

			if(!isDefinedAfter){
				assert(defMap.count(newDef) == 0);
				genSet[blockIndex].set(defMap[newDef] - 1);
			}
		}
		if(tracer == end) break;
		else tracer = tracer->next;
	}
}

//public
void RD::genGenSet(){
	iniGenSet();
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		genGenBlock(i);
	}
}
////////////////////////////////////////////////////////////////////////
//killSet
///////////////////////////////////////////////////////////////////////

//helper
void RD::iniKillSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		this->killSet[i].ini(defNum);
	}
}
bool RD::instrInBB(simple_instr *instr,int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		if(tracer == instr) return true;
		if(tracer == end) break;
		else tracer = tracer -> next;
	}
	return false;
}
bool RD::instrSameDest(simple_instr *lhs, simple_instr *rhs){
	assert(isDef(lhs));
	assert(isDef(rhs));

	return lhs->u.base.dst->num == rhs->u.base.dst->num;
}



void RD::genKillBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		if(isDef(tracer)){
			for(map<Definition, int>::const_iterator ite = defMap.begin();
				ite != defMap.end(); ite++){
				if(!instrInBB(tracer,blockIndex) &&
						instrSameDest(tracer,ite->first.instr))
					killSet[blockIndex].set(ite->second-1);
			}
		}
		if(tracer == end) break;
		else tracer = tracer->next;
	}
}
	

//public
void RD::genKillSet(){
	iniKillSet();
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		genKillBlock(i);
	}
}


////////////////////////////////////////////////////////
//RDSET
////////////////////////////////////////////////////////


void RD::genRDOutSet(){
	DFA newDFA(DFA::DIRECTION_FORWARD, DFA::PATH_ANYWAY, 
					genSet,killSet,cfg,defNum);

	this->RDOutSet = newDFA.getOutSet();
}



