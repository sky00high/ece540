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
	

