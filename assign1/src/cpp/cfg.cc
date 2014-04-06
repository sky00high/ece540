#include <stdio.h>
extern "C"{
	#include <simple.h>
}
#include <iostream>
#include <map>
#include "cfg.h"

using namespace std;

int BasicBlock::operator==(const BasicBlock &rhs) const{
	if(rhs.blockNum == this->blockNum) return 1;
	return 0;
}

int BasicBlock::operator< (const BasicBlock &rhs) const{
	if(this->blockNum < rhs.blockNum) return 1;
	return 0;

}
BasicBlock::BasicBlock(){
	instrNum = 0;
	blockNum = 0;
	startNum = 0;
	endNum = 0;
}
BasicBlock::BasicBlock(int _instrNum, int start, int end){
	instrNum = _instrNum;
	startNum = start;
	endNum = end;
	
}

void BasicBlock::addPreb(int blockIndex){
	preb[blockIndex] = 1;
}

void BasicBlock::addSucc(int blockIndex){
	succ[blockIndex] = 1;
}

ostream &operator<<(ostream &os, const BasicBlock &BB){
	cout<<"block "<<BB.blockNum<<endl;
	cout<<"\tinstrs "<<BB.instrNum;
	if(BB.instrNum != 0){
		for(int i = BB.startNum; i<=BB.endNum; i++){
			cout<<" "<<i;
		}
	}
	cout<<endl;

	cout<<"\tsuccessors "<<BB.succ.size();
	if(BB.succ.size() != 0){
		for(map<int,int>::const_iterator ii = BB.succ.begin(); ii != BB.succ.end(); ++ii){
			cout<<" "<<(*ii).first;
		}
	}
	cout <<endl;

	cout<<"\tpredecessors "<<BB.preb.size();
	if(BB.preb.size() != 0){
		for(map<int,int>::const_iterator ii = BB.preb.begin(); ii != BB.preb.end(); ++ii){
			cout<<" "<<(*ii).first;
		}
	}
	cout <<endl;
}

int CFG::buildLeadVector(simple_instr *inlist, map<int,int> &leadVector){
	simple_instr *i = inlist;
	int instrNum = 0;
	

   //set all element in leadvector to be 0 and get 
	// the total number of instructions
	while(i){
		leadVector[instrNum] = 0;
		instrNum++;
		i = i->next;
	}

	leadVector[0] = 1;

	i = inlist;
	for(int j = 0; j < instrNum; j++){
		if(simple_op_format(i->opcode) == BJ_FORM || i->opcode == MBR_OP){
			leadVector[j + 1] = 1;
		} else if(i->opcode == LABEL_OP){
			leadVector[j] = 1;
		}
		i = i->next;
	}
 /* 
	//Debugging purpose
   cout<<"buildLeadVector: intotal "<<instrNum<<" instrctions, lead vector: ";
	for(int j = 0; j < instrNum; j++){
		cout<<leadVector[j];
	}
	cout<<endl;
	//end debugging
	*/


	return instrNum;
}

void CFG::addBlock(int instrNum, int start, int end){
	BasicBlock newBlock(instrNum,start,end);
	newBlock.blockNum = totalBlockNum;

/*
	//debugging msg
	cout<<"Block "<<newBlock.blockNum<<": instrNum: "<<newBlock.instrNum;
	cout<<"\n\t from "<<newBlock.startNum<<" to "<<newBlock.endNum;
	cout<<endl;
	//debugging msg down
	*/

	cfgMap[totalBlockNum] = newBlock;
	totalBlockNum++;

	
}
void CFG::iniBasicBlock(simple_instr *inlist, map<int,int> &leadVector){
	addBlock(0,0,0);
	int tracer = 0;
	simple_instr *instrTracer = inlist;

	for(int i= 0; i < totalInstrNum; i++){
		if(leadVector[i] == 1){
			tracer = i;
			if(leadVector[i+1] != 1){
				i++;
				while(leadVector[i+1] != 1 && i < totalInstrNum - 1) i++;
				addBlock(i-tracer + 1,tracer,i);
			}
			else addBlock(1,tracer,tracer);
		}
	}
			
	addBlock(0,0,0);

}
	

	
void CFG::buildCFG(simple_instr *inlist){

	simple_instr *tracer = inlist;
	map<string,int> labelTable;
	int index = 0;


   //build a hash table which index is the name of label and the data is the index of
	//instruction. 
   while(tracer){
		if(tracer->opcode == LABEL_OP){ 
			labelTable[tracer->u.label.lab->name] = index;
		}
		tracer = tracer->next;
		index++;
	}

   //add approprety succ and preb to each basic Block
	tracer = inlist;
	for(int i = 0; i < totalInstrNum; i++){
		if(simple_op_format(tracer->opcode) == BJ_FORM){
			switch(tracer->opcode){
				case JMP_OP:{
					int thisIndex = findBasicBlockInstr(i);
					int targetIndex = findBasicBlockInstr(labelTable[tracer->u.bj.target->name]);
					cfgMap[targetIndex].addPreb(thisIndex);
					cfgMap[thisIndex].addSucc(targetIndex);
					break;
				}
				case BTRUE_OP:
				case BFALSE_OP:{
					int thisIndex = findBasicBlockInstr(i);
					int targetIndex = findBasicBlockInstr(labelTable[tracer->u.bj.target->name]);
					cfgMap[targetIndex].addPreb(thisIndex);
					cfgMap[thisIndex].addSucc(targetIndex);
					cfgMap[thisIndex].addSucc(thisIndex + 1);
					cfgMap[thisIndex + 1].addPreb(thisIndex);
					break;
				}
				default:
					cout<<"something is wrong"<<endl;
			}
		} else if(tracer->opcode == MBR_OP){
			int thisIndex = findBasicBlockInstr(i);
			int ntargets = tracer->u.mbr.ntargets;

			for(int j = 0; j < ntargets; j++){
				int targetIndex = findBasicBlockInstr(labelTable[tracer->u.mbr.targets[j]->name]);
				cfgMap[targetIndex].addPreb(thisIndex);
				cfgMap[thisIndex].addSucc(targetIndex);
			}

			int targetIndex = findBasicBlockInstr(labelTable[tracer->u.mbr.deflab->name]);
			cfgMap[targetIndex].addPreb(thisIndex);
			cfgMap[thisIndex].addSucc(targetIndex);
		}else if(tracer->opcode == LABEL_OP){
			if(i != 0 && tracer->prev->opcode != JMP_OP && tracer->prev->opcode != MBR_OP){
				int thisIndex = findBasicBlockInstr(i);
				cfgMap[thisIndex].addPreb(thisIndex - 1);
				cfgMap[thisIndex - 1].addSucc(thisIndex);
			}
		}
		tracer = tracer->next;
	}
	if(totalBlockNum >1){

		cfgMap[0].addSucc(1);
		cfgMap[1].addPreb(0);
		cfgMap[totalBlockNum-1].addPreb(totalBlockNum-2);
		cfgMap[totalBlockNum-2].addSucc(totalBlockNum-1);
	}


}

				

int CFG::findBasicBlockInstr(int instrNum){
	for(int i = 0; i < totalBlockNum; i++){
		if(cfgMap[i].instrNum == 0) continue;
		if(cfgMap[i].startNum <= instrNum && cfgMap[i].endNum >=instrNum){
			return i;
		}
	}
	return -1;
}


void CFG::findDom(){
	//entry has one dominator, itself
	cfgMap[0].dom.insert(0);
	//ini every other basicblok's dominator to every node none entry and exit
	for(int i = 1; i < totalBlockNum ; i++){
		for(int j = 0; j < totalBlockNum ; j++){
			cfgMap[i].dom.insert(j);
		}
	}

	bool changed = true;
	while(changed){
		changed = false;
		for(int i = 1; i < totalBlockNum - 1; i++){
			set<int> oldDom = cfgMap[i].dom;
			set<int> newDom;
			BasicBlock BB = cfgMap[i];
			for(int j = 0; j < totalBlockNum; j++){
				bool containThisOne = true;
				//check this BB if its in every of i's pred's dominator list
				//if so add it to i's dominator's list
				for(map<int,int>::const_iterator predIte = BB.preb.begin();
													 predIte != BB.preb.end(); ++predIte){
					if(cfgMap[(*predIte).first].dom.count(j) == 0) containThisOne = false;
				}
				if(containThisOne || j == i) newDom.insert(j); 
			}
			if(newDom != oldDom) changed = true;
			cfgMap[i].dom = newDom;
		}
	}
/*
  //Debugging msg
  for(int i = 1; i < totalBlockNum -1; i++){
	  cout<<"block "<<i<<"\n\tdominators:";
	  for(set<int>::iterator ii = cfgMap[i].dom.begin(); ii != cfgMap[i].dom.end(); ii++){
		  cout<<*ii<<" ";
	  }
	  cout <<endl;
  }
  //debugging done
  */
}

void CFG::findIDom(){
	findDom();

   //initiate ddom set for each n belongs to N-{entry}
	// DOMd(n) = DOM(n) - n
	for(int i = 1; i < totalBlockNum - 1; i++){
		for(set<int>::iterator ii = cfgMap[i].dom.begin(); ii != cfgMap[i].dom.end(); ii++){
			if(*ii == i) continue;
			cfgMap[i].iDom.insert(*ii);
		}
	}

   //for each n belongs to N-{entry}
	for(int n = 1; n < totalBlockNum - 1; n ++){
		//for each s belongs to DOMd(n)
		for(set<int>::iterator s = cfgMap[n].iDom.begin(); s != cfgMap[n].iDom.end(); s++){
			//for each t belongs to DOMd(n) - {s}
			for(set<int>::iterator t = cfgMap[n].iDom.begin(); t != cfgMap[n].iDom.end(); t++){
				if(*t == *s) continue;
				//if t belongs to DOMp(s)
				if(cfgMap[*s].dom.count(*t) != 0){
					//DOMd(n) = DOMd(n) - {t}
					cfgMap[n].iDom.erase(*t);
				}
			}
		}
	}

}

void CFG::printIDom(){
	cout<<"block 0\n\tidom"<<endl;
	for(int i = 1; i < totalBlockNum - 1; i++){
		cout<<"block "<<i<<endl;
		cout<<"\tidom";
	   for(set<int>::iterator ii = cfgMap[i].iDom.begin(); ii != cfgMap[i].iDom.end(); ii++){
		  cout<<" "<<*ii;
	   }
		cout<<"\n";
	}
	cout<<"block "<<totalBlockNum - 1<<endl;
	cout<<"\tidom "<<totalBlockNum - 2<<endl;
}


void CFG::printBlock(){
	for(int i = 0; i < totalBlockNum; i ++){
		cout<<cfgMap[i];
	}
}
CFG::CFG(simple_instr *inlist){
	
	map <int,int> leadVector;
	this->totalBlockNum = 0;
	this->totalInstrNum = buildLeadVector(inlist, leadVector);
	iniBasicBlock(inlist, leadVector);
	buildCFG(inlist);


}



	


		



	



	
