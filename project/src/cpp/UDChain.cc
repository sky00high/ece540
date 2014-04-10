#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "RD.h"
#include <assert.h>
#include "Definition.h"
#include "UDChain.h"
extern "C"{
#include "print.h"
	#include <simple.h>
}

void UDChain::setEdge(int start, int end){
	assert(start < instrNum);
	assert(end < instrNum);
	
	UDGraph[start][end] = true;
}

void UDChain::clearEdge(int start, int end){
	assert(start < instrNum);
	assert(end < instrNum);
	
	UDGraph[start][end] = false;
}

bool UDChain::isEdgeSet(int start, int end){
	assert(start < instrNum);
	assert(end < instrNum);
	
	return UDGraph[start][end];
}

void UDChain::checkReg(simple_instr *instr,int blockIndex,simple_reg *lereg){
	if(lereg->kind == PSEUDO_REG){
		simple_instr *start, *end;
		start = this->cfg->getBlockStartInstr(blockIndex);

		simple_instr *tracer = start;
		bool isRedefined = false;
		simple_instr *lastDefined = NULL;
		while(tracer != instr){
			if(rd->isDef(tracer)){
				if(tracer->u.base.dst->num == lereg->num){
					isRedefined = true;
					lastDefined = tracer;
				}
			}
			tracer = tracer ->next;
		}
		if(!isRedefined){
			BitVector rdIn = (rd->getRDInSet())[blockIndex];
			map<Definition,int>defMap = rd->getDefMap();
			for(map<Definition,int>::const_iterator ite = defMap.begin();ite != defMap.end(); ite++){
				//set the edge if this def's instr not in this BB
				// and this def define the reg being used
				// and it is in the RDin set of this BB
				if(ite->first.instr->u.base.dst->num == lereg->num)
					if(rdIn.isSet(ite->second -1))
						setEdge(cfg->getInstrNum(ite->first.instr), cfg->getInstrNum(instr));
			}
		} else {
			assert(lastDefined != NULL);
			setEdge(cfg->getInstrNum(lastDefined),cfg->getInstrNum(instr));
		}
	}
}
	
void UDChain::checkVar(int blockIndex, simple_instr *tracer){
	switch (tracer->opcode) {
		case LOAD_OP: {
			 checkReg(tracer,blockIndex, tracer->u.base.src1);
			 break;
		}
		case STR_OP: {
			 checkReg(tracer,blockIndex, tracer->u.base.src1);
			 checkReg(tracer,blockIndex, tracer->u.base.src2);
			 break;
		}
		case MCPY_OP: {
			 checkReg(tracer,blockIndex, tracer->u.base.src1);
			 checkReg(tracer,blockIndex, tracer->u.base.src2);
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
			 checkReg(tracer,blockIndex, tracer->u.bj.src);
			 break;
		}
		case CALL_OP: {
			 unsigned n, nargs;
			 checkReg(tracer,blockIndex, tracer->u.call.proc);
			 nargs = tracer->u.call.nargs;
			 if (nargs != 0) {
			for (n = 0; n < nargs - 1; n++) {
				 checkReg(tracer,blockIndex, tracer->u.call.args[n]);
			}
			checkReg(tracer,blockIndex, tracer->u.call.args[nargs - 1]);
			 }
			 break;
		}
		case MBR_OP: {
			 checkReg(tracer,blockIndex, tracer->u.mbr.src);
			 break;
		}
		case LABEL_OP:{
			break;
		}
		case RET_OP: {
			 if (tracer->u.base.src1 != NO_REGISTER) {
				checkReg(tracer,blockIndex, tracer->u.base.src1);
			 }
			 break;
		}

		case CVT_OP:
		case CPY_OP:
		case NEG_OP:
		case NOT_OP: {
			 /* unary base instructions */
			 checkReg(tracer,blockIndex, tracer->u.base.src1);
			 break;
		}

		default: {
			 /* binary base instructions */
			 checkReg(tracer,blockIndex, tracer->u.base.src1);
			 checkReg(tracer,blockIndex, tracer->u.base.src2);
		}
	}
}

void UDChain::buildUDChainBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		checkVar(blockIndex, tracer);
		if(tracer == end) break;
		else tracer = tracer->next;
	}
}
UDChain::UDChain(simple_instr *inlist, CFG *cfg, RD *rd){
	int instrNum = 0;
	this->inlist = inlist;
	this->cfg = cfg;
	this->rd = rd;
	
	simple_instr *tracer = inlist;
	while(tracer != NULL){
		tracer = tracer->next;
		instrNum++;
	}
	this->instrNum = instrNum;

	//initialize UDGraph
	UDGraph = new bool*[instrNum];
	for(int i = 0; i < instrNum; i++){
		UDGraph[i] = new bool[instrNum];
		for(int j = 0; j < instrNum; j++){
			UDGraph[i][j] = false;
		}
	}

	//build the graph
	for(int i = 0; i < cfg->getTotalBlockNum(); i++){
		buildUDChainBlock(i);
	}
}


UDChain::~UDChain(){
	for(int i = 0; i < instrNum; i++){
			delete [] UDGraph[i];
	}
	delete [] UDGraph;
}

set<int> UDChain::findDefUse(int useIndex){
	set<int> defSet;
	for(int i = 0; i < instrNum;i++){
		if(UDGraph[i][useIndex]){
			defSet.insert(i);
		}
	}
	return defSet;
}

void UDChain::printUDChain(){
	for(int i = 0; i < instrNum; i++){
		for(int j = 0; j < instrNum; j++){
			if(isEdgeSet(i,j)){
				cout<<"def ";
				fprint_instr(stdout, cfg->findInstrIndex(i));
				cout<<"Use ";
				fprint_instr(stdout, cfg->findInstrIndex(j));
			}
		}
	}
}

