#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "ae.h"
#include <assert.h>
#include "DFA.h"
using namespace std;
extern "C"{
	#include <simple.h>
}



RE::RE(CFG *cfg, simple_instr *inlist){
	assert(cfg != NULL);
	assert(inlist != NULL);

	this->cfg = cfg;
	this->inlist = inlist;
	this->exprNum = 0;
}

//part 1 helper
simple_instr *RE::findInstr(int index){
	return this->cfg->findInstr(index);
}
Expression RE::findExprIndex(int index){
	for(map<Expression,int>::const_iterator ite = exprMap.begin(); 
		 ite != exprMap.end(); ite++){
		if(ite->second == index) return ite->first;
	}
	assert(false);
}
bool RE::isBinaryInstr(simple_instr *instr){
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
		case RET_OP: 
		case CVT_OP:
		case CPY_OP:
		case NEG_OP:
		case NOT_OP: break; 
		default: return true;
	}
	return false;
}

bool RE::isCVT(simple_instr *instr){
	return instr->opcode == CVT_OP && isPseudoReg(instr->u.base.src1);
}
bool RE::isPseudoReg(simple_reg *reg){

	return reg->kind ==  PSEUDO_REG;
}
//part 1 public
void RE::genExprList(){
	simple_instr *tracer = inlist;
	int instrIndex = 0;
	while(tracer){
		if(isExpr(tracer)){
			//binary operation
			Expression newExpr;
			if(tracer->opcode != CVT_OP){
				newExpr.op1 = tracer->u.base.src1->num;
				newExpr.op2 = tracer->u.base.src2->num;
				newExpr.opcode = tracer->opcode;
				newExpr.instrIndex = instrIndex;
				newExpr.instr = tracer;
			}else{
				newExpr.op1 = tracer->u.base.src1->num;
				newExpr.op2 = 777;
				newExpr.opcode = tracer->opcode;
				newExpr.instrIndex = instrIndex;
				newExpr.instr = tracer;
			}

			/*
			//debugging msg
			cout<<"find new expression, instrction number: "<<newExpr.instrIndex<<" ";
			print_reg(tracer->u.base.src1);cout<<" ";
			print_reg(tracer->u.base.src2);cout<<" ";
			cout<<"opcode " <<newExpr.opcode<<endl;
			//debugging done
			*/
			if(this->exprMap.count(newExpr) == 0){
				//cout<<" added into map"<<endl; //debugging msg
				exprMap[newExpr] = ++exprNum;
			}else{
				//debugging msg
			//	cout<<"   is added, ignore"<<endl;
				//done
			}
		}
		tracer = tracer->next;
		instrIndex++;
	}
}
void RE::printExprList(){
	for(int i = 1; i <= exprNum; i++){
		cout<<"expr "<<i<<" "<<findExprIndex(i).instrIndex<<endl;
	}
}
/*****************************************************************************/
//part2 helper
void RE::iniEvalSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		this->evalSet[i].ini(exprNum);
	}
}
bool RE::isExpr(simple_instr *instr){
	return isCVT(instr) || (isBinaryInstr(instr) &&
				isPseudoReg(instr->u.base.src1)&&
				isPseudoReg(instr->u.base.src2));
}
void RE::genEvalBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		if(isExpr(tracer)){
			//debugging msg
			/*
			cout<<"find expression, op"<<tracer->opcode<<" r"<<tracer->u.base.src1->num;
			cout<<",r"<<tracer->u.base.src2->num<<" ";
			*/
			//done
			bool isDefinedAfter = false;
			simple_instr *tracer2 = tracer;
			Expression newExpr(tracer);
			//Expression newExpr2(tracer,true);
			while(tracer2 != end->next){
				if(exprIsKilled(tracer2,newExpr)){
					isDefinedAfter = true;
				}
				tracer2 = tracer2->next;
			}
			if(!isDefinedAfter){
				if(exprMap.count(newExpr) != 0) 
					evalSet[blockIndex].set(exprMap[newExpr]-1);
			//	if(exprMap.count(newExpr2) != 0) 
			//		evalSet[blockIndex].set(exprMap[newExpr2]-1);
			}
			//debugging msg
			/*
			if(isDefinedAfter){
				cout<<" is defined after"<<endl;
			}else{
				cout <<" added into map"<<endl;
			}
			*/
			//done
		}
		if(tracer ==  end) break;
		else tracer = tracer->next;
	}
}
bool RE::exprIsKilled(simple_instr *instr, Expression expr){
	switch (instr->opcode) {
		case LOAD_OP: {
			if(instr->u.base.dst->kind == PSEUDO_REG)
				if(instr->u.base.dst->num == expr.op1 ||
					instr->u.base.dst->num == expr.op2)
					return true;
				
			 break;
		}
		case STR_OP: 
		case MCPY_OP: break;
		case LDC_OP: {
			if(instr->u.ldc.dst->kind == PSEUDO_REG)
				if(instr->u.ldc.dst->num == expr.op1 ||
					instr->u.ldc.dst->num == expr.op2)
					return true;
			 break;
		}
		case JMP_OP:	
		case BTRUE_OP:
		case BFALSE_OP: break;
		case CALL_OP: {
			 unsigned n, nargs;
			 if (instr->u.call.dst != NO_REGISTER) {
				if(instr->u.call.dst->kind == PSEUDO_REG)
					if(instr->u.call.dst->num == expr.op1 ||
						instr->u.call.dst->num == expr.op2)
						return true;
			 }
			 break;
		}
		case MBR_OP: break;
		case LABEL_OP: break; 
		case RET_OP: break;
		case CVT_OP:
		case CPY_OP:
		case NEG_OP:
		case NOT_OP: {
				if(instr->u.base.dst->kind == PSEUDO_REG)
					if(instr->u.base.dst->num == expr.op1 ||
						instr->u.base.dst->num == expr.op2)
						return true;
			 break;
		}
		default: {
				if(instr->u.base.dst->kind == PSEUDO_REG)
					if(instr->u.base.dst->num == expr.op1 ||
						instr->u.base.dst->num == expr.op2)
						return true;
		}
	}
	return false;
}

//part2
void RE::genEvalSet(){
	iniEvalSet();
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		genEvalBlock(i);
	}
}

void RE::printEvalSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		cout <<"eval_ae " <<i<<" "<<evalSet[i]<<endl;
	}
}


//part3 helpers
void RE::iniKillSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		this->killSet[i].ini(exprNum);
	}
}
void RE::genKillBlock(int blockIndex){
	simple_instr *start, *end;
	start = this->cfg->getBlockStartInstr(blockIndex);
	end = this->cfg->getBlockEndInstr(blockIndex);

	simple_instr *tracer = start;
	if(start != NULL) while(true){
		for(map<Expression,int>::const_iterator ite = exprMap.begin();
			 ite != exprMap.end(); ite++){
			if(exprIsKilled(tracer,ite->first)){
				killSet[blockIndex].set(ite->second -1);
			}
		}
		if(tracer ==  end) break;
		else tracer = tracer->next;
	}
}

//part3
void RE::genKillSet(){
	iniKillSet();
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		genKillBlock(i);
	}
}

void RE::printKillSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		cout <<"kill_ae " <<i<<" "<<killSet[i]<<endl;
	}
}

void RE::genREOutSet(){
	DFA newDFA(DFA::DIRECTION_FORWARD, DFA::PATH_ANYWAY, evalSet, killSet,
					cfg, exprNum);

	this->REOutSet = newDFA.getOutSet();
}
void AE::printAeOutSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum(); i++){
		cout <<"reaching_exprs_out " <<i<<" "<<aeOutSet[i]<<endl;
	}
}


