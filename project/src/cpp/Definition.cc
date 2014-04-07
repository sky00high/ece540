#include <assert.h>
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "Definition.h"
extern "C"{
	#include <simple.h>
}


bool Definition::operator==(const Definition &rhs) const{
	return this->instrIndex == rhs.instrIndex;
}

bool Definition::operator<(const Definition &rhs) const{
	return this->instrIndex<rhs.instrIndex;
}


Definition::Definition(){
}

Definition::Definition(simple_instr *instr, int instrIndex){
	this->instr = instr;
	this->instrIndex = instrIndex;
}




