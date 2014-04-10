#include <stdio.h>
extern "C"{
	#include <simple.h>
	#include "print.h"
}
#include <iostream>
#include <assert.h>
#include <map>
#include "cfg.h"
#include <stack>

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
		if(tracer->opcode == RET_OP){
			int thisIndex = findBasicBlockInstr(i);
			int targetIndex = getTotalBlockNum()-1;
			
			cfgMap[targetIndex].addPreb(thisIndex);
			cfgMap[thisIndex].addSucc(targetIndex);

		}else if(simple_op_format(tracer->opcode) == BJ_FORM){
		//if(simple_op_format(tracer->opcode) == BJ_FORM){
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
			if(i != 0 && tracer->prev->opcode != JMP_OP && tracer->prev->opcode != MBR_OP && tracer->prev->opcode != RET_OP){
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
bool CFG::edgeExist(int start, int end){
	BasicBlock sourceBlock = cfgMap[start];
	BasicBlock endBlock = cfgMap[end];

	if(sourceBlock.succ.count(end) != 0 && endBlock.preb.count(start) != 0) return true;
	if(sourceBlock.succ.count(end) == 0 && endBlock.preb.count(start) == 0) return false;
	
	cout<<"something is wrong"<<end;
	return false;
}

void CFG::findREdge(){
	set<edge> _rEdge;
	//iterate every pair of nodes,  if there exist an edge that point node i to node j,
	//and if node j dominate node i, it is a retreating edge and add it to the set.
	for(int i =1; i < totalBlockNum-1; i ++){
		for(int j = 1; j < totalBlockNum-1; j++){
			if(i==j) continue;

			if(edgeExist(i,j) && cfgMap[i].dom.count(j) != 0){
				/*
				//debugging msg
				cout<<"findREdge from block "<<i<<" to "<<j<<" instr "<<cfgMap[i].endNum<<" to "
					<<cfgMap[j].startNum<<endl;
				//debugging msg done
				*/

				edge newEdge;
				newEdge.start = i;
				newEdge.end = j;
				newEdge.startInstr = findInstr(cfgMap[i].endNum);
				newEdge.endInstr = findInstr(cfgMap[j].startNum);
				_rEdge.insert(newEdge);
			}
		}
	}
	this->rEdge = _rEdge;
	/*
//debug
	cout<<"infindREdge "<<endl;
	for(set<edge>::iterator edgeIte = rEdge.begin();edgeIte!= rEdge.end();edgeIte++){
		cout<<edgeIte->start<<" "<<edgeIte->end<<endl;
	}
	cout<< "done"<<endl;

//debug done
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
void CFG::printDom(){
	cout<<"block 0\n\tdom"<<endl;
	for(int i = 1; i < totalBlockNum - 1; i++){
		cout<<"block "<<i<<endl;
		cout<<"\tdom";
	   for(set<int>::iterator ii = cfgMap[i].dom.begin(); ii != cfgMap[i].dom.end(); ii++){
		  cout<<" "<<*ii;
	   }
		cout<<"\n";
	}
	cout<<"block "<<totalBlockNum - 1<<endl;
	cout<<"\tdom "<<totalBlockNum - 2<<endl;
}


void CFG::printBlock(){
	for(int i = 0; i < totalBlockNum; i ++){
		cout<<cfgMap[i];
	}
}

simple_instr *CFG::findInstr(int index){
	simple_instr *target = inlist;
	for(int i = 0; i < index; i++){
		target = target->next;
	}
	return target;
}

void CFG::insertPreHeader(stack<simple_instr*> loopStarters){
	//take care of test12.  when one side of the retreating edge's is not JUMp or MBR
	for(set<edge>::iterator edgeIte = rEdge.begin(); edgeIte != rEdge.end(); edgeIte++){
		if(edgeIte->startInstr->next == edgeIte->endInstr){ //if it doesn't "jump"
			simple_instr *newJump = new_instr(JMP_OP,simple_type_void);
			newJump->u.bj.target = edgeIte->endInstr->u.label.lab;
			newJump->u.bj.src = NULL;
			
			edgeIte->startInstr->next = newJump;
			newJump->prev = edgeIte->startInstr;
			newJump->next = edgeIte->endInstr;
			edgeIte->endInstr->prev = newJump;

			totalInstrNum++;
		}
	}
					

	set<simple_instr*> addedStarters;
	while(!loopStarters.empty()){
		simple_instr *startInstr = loopStarters.top();
		loopStarters.pop();
		if(addedStarters.count(startInstr) != 0) continue;
		simple_instr *newHeader = new_instr(LABEL_OP,simple_type_void);
		simple_instr *pre = startInstr->prev;
		newHeader->u.label.lab = new_label();

		simple_instr *tracer = inlist;
		for(int i = 0; i < totalInstrNum; i++){
			
			//first we check if this is one of the node on the retreating edges.
			bool pass = false;
			for(set<edge>::iterator ite = rEdge.begin(); ite != rEdge.end(); ite++){
				if((*ite).startInstr == tracer && (*ite).endInstr == startInstr) pass = true;
			}
			if(pass)continue;

			//if not, we check if this instruction is going to jump to the previous header,
			//if so we change the target to the new preheader.
			if(simple_op_format(tracer->opcode) == BJ_FORM){
				if(tracer->u.bj.target == startInstr->u.label.lab) tracer->u.bj.target = newHeader->u.label.lab;
			} else if(tracer->opcode == MBR_OP){
				int ntargets = tracer->u.mbr.ntargets;
				for(int j = 0 ; j < ntargets; j++){
					if(tracer->u.mbr.targets[j] == startInstr->u.label.lab){
						tracer->u.mbr.targets[j] = newHeader->u.label.lab;
					}
				}
			} 
			tracer = tracer->next;
		}
		addedStarters.insert(startInstr);
		pre->next = newHeader;
		newHeader->prev = pre;
		newHeader->next = startInstr;
		startInstr->prev = newHeader;
		totalInstrNum++;
	}
}


void CFG::genLoopHeader(){ 
	//find all the REdges and store them in private member ses<edge> rEdge
	findREdge();
	
	//push all the simple_instr that is head of a nature loop into a stack
	stack <simple_instr *> loopStarters;
	for(set<edge>::iterator ite = rEdge.begin(); ite != rEdge.end(); ite++){
		loopStarters.push(findInstr(cfgMap[(*ite).end].startNum));
	}

	//insertPreHeaders into the inlist
	insertPreHeader(loopStarters);
}
CFG::CFG(simple_instr *inlist){
	
	map <int,int> leadVector;
	this->totalBlockNum = 0;
	this->loopNum = 0;
	this->totalInstrNum = buildLeadVector(inlist, leadVector);
	this->inlist = inlist;
	iniBasicBlock(inlist, leadVector);
	buildCFG(inlist);
	loopGenerated = false;
}

//A2part2

void CFG::insertOnStack(int node,stack<int> &tStack, set<int> &loop){
	if(loop.count(node) == 0){
		loop.insert(node);
		tStack.push(node);
	}
}
set<int> CFG::findLoop(int start, int end){

	stack<int> tStack;
	set<int> loop;

	//Set loop(H)
   loop.insert(start);

	//insert_on_stack(t)
	insertOnStack(end,tStack,loop);


	while(!tStack.empty()){
		//m = pop(S)
		int m = tStack.top();
		tStack.pop();

      //for each pred(m)
		BasicBlock BB = cfgMap[m];
		for(map<int,int>::const_iterator ii = BB.preb.begin(); ii != BB.preb.end(); ii++){
			//insert_on_stack p
			insertOnStack((*ii).first,tStack,loop);
		}
	}

	return loop;
}

set<int> CFG::findExitNode(set<int> loop){
	set<int> exitNode;
	//for each of the BB in the loop
	for(set<int>::iterator ite=loop.begin(); ite!=loop.end();ite++){
		map<int,int> succ = getBlockSucc(*ite);
		//for each of the succ of the BB
		for(map<int,int>::const_iterator ite2=succ.begin();ite2!=succ.end();ite2++){
			//if the succ is not in the loop, the BB is an exit node, add it!
			if(loop.count(ite2->first) == 0){
				exitNode.insert(*ite);
			}
		}
	}
	return exitNode;
}
	 
void CFG::genLoopSet(){
	if(loopGenerated ) return;
	set<edge>::iterator edgeIte = rEdge.begin();


	while(edgeIte != rEdge.end()){
		set<int> loop = findLoop(edgeIte->end,edgeIte->start);
		int startBlockNum = edgeIte->end;
		set<int> backedge;
		backedge.insert(edgeIte->start);
		edgeIte++;
		//the rEdge is sorted by the end of edge that is why this while loop will work
		while(edgeIte->end == startBlockNum && edgeIte != rEdge.end()){
			backedge.insert(edgeIte->start);
			set<int> newLoop = findLoop(edgeIte->end, edgeIte->start);
			for(set<int>::iterator newLoopIte = newLoop.begin(); 
					newLoopIte != newLoop.end();newLoopIte++){
				loop.insert(*newLoopIte);
			}
			edgeIte++;
		}
		exitNodeSet[loopNum] = findExitNode(loop);
		loopStart[loopNum] = startBlockNum;
		loopSet[loopNum++] = loop;
	}
	loopGenerated = true;
}


void CFG::printLoop(){
	set<edge>::iterator edgeIte = rEdge.begin();

	int loopIndex = 0;

	while(edgeIte != rEdge.end()){
		set<int> loop = findLoop(edgeIte->end,edgeIte->start);
		int startBlockNum = edgeIte->end;
		set<int> backedge;
		backedge.insert(edgeIte->start);
		

		edgeIte++;
		//the rEdge is sorted by the end of edge that is why this while loop will work
		while(edgeIte->end == startBlockNum && edgeIte != rEdge.end()){

	//		cout<<"found another backedge with same staring block num"<< "start from "<<edgeIte->end;//debugging msg

			backedge.insert(edgeIte->start);
			set<int> newLoop = findLoop(edgeIte->end, edgeIte->start);

//			cout<<"\tithas "<<newLoop.size()<<" blocks"<<endl; //debugging msg
			for(set<int>::iterator newLoopIte = newLoop.begin(); newLoopIte != newLoop.end();
					newLoopIte++){
				loop.insert(*newLoopIte);
			}
			edgeIte++;
		}

		cout<<"natloop "<<loopIndex<<endl;
		cout<<"\tbackedge "<<startBlockNum;
		for(set<int>::iterator backedgeIte = backedge.begin(); backedgeIte != backedge.end(); backedgeIte++){
			cout<<" "<<*backedgeIte;
		}
		cout<<endl;
		cout<<"\tpreheader "<<startBlockNum - 1<<endl;
		cout<<"\tloop_blocks";
		for(set<int>::iterator loopIte = loop.begin(); loopIte != loop.end(); loopIte ++){
			cout<<" "<<*loopIte;
		}
		cout<<endl;
		loopIndex++;
	}
}

int CFG::getTotalLoopNum(){

	set<int>loopStarters;
	for(set<edge>::iterator edgeIte = rEdge.begin(); edgeIte!=rEdge.end(); edgeIte++){
		loopStarters.insert(edgeIte->end);
	}
	return loopStarters.size();
}
		
			
simple_instr *CFG::getBlockStartInstr(int blockIndex){
	if(cfgMap[blockIndex].instrNum != 0){
		return findInstr(cfgMap[blockIndex].startNum);
	}
	return NULL;
}
simple_instr *CFG::getBlockEndInstr(int blockIndex){
	if(cfgMap[blockIndex].instrNum != 0){
		return findInstr(cfgMap[blockIndex].endNum);
	}
	return NULL;
}

map<int,int> CFG::getBlockSucc(int blockIndex){
	return cfgMap[blockIndex].succ;
}
map<int,int> CFG::getBlockPred(int blockIndex){
	return cfgMap[blockIndex].preb;
}

int CFG::getInstrNum(simple_instr *instr){
	simple_instr *tracer = inlist;
	int index = 0;

	while(tracer != NULL){
		if(tracer == instr) return index;
		tracer = tracer->next;
		index ++;
	}
	assert(false);
}

int CFG::findIndexInstr(simple_instr *instr){
	return getInstrNum(instr);
}

simple_instr *CFG::findInstrIndex(int index){
	return findInstr(index);
}

int CFG::findBBInstr(simple_instr *instr){
	return findBasicBlockInstr(this->findIndexInstr(instr));
}
int CFG::findBBIndex(int index){
	return findBasicBlockInstr(index);
}

map<int,set<int> > CFG::getLoopSet(){
	if(!loopGenerated){
		genLoopSet();
	}
	return loopSet;
}

set<int> CFG::getLoop(int loopIndex){
	if(!loopGenerated){
		genLoopSet();
	}
	return loopSet[loopIndex];
}

set<int> CFG::getExitNode(int loopIndex){
	if(!loopGenerated){
		genLoopSet();
	}
	return exitNodeSet[loopIndex];
}

int CFG::getLoopStart(int loopIndex){
	if(!loopGenerated){
		genLoopSet();
	}
	return loopStart[loopIndex];
}

bool CFG::ifDom(int BB1, int BB2){
	return cfgMap[BB2].dom.count(BB1) != 0;
}

void CFG::fullPrint(){
	assert(loopGenerated);
	for(int i = 0; i < totalBlockNum; i++){
		cout<<"Block "<<i<<"\t";
		BasicBlock BB = cfgMap[i];
		for(int k = 0; k < loopNum; k++){
			set<int> loop = loopSet[k];
			set<int> exitNode = exitNodeSet[k];
			int loopStarter = loopStart[k];
			if(i == loopStarter){
				cout<<k<<"(start) ";
			}
			if(loop.count(i) != 0){
				cout<<k<<" ";
			}
			if(exitNode.count(i) != 0){
				cout<<k<<"(exit) ";
			}
		}
		cout<<endl;
		cout<<"\tsuccessors ";
		if(BB.succ.size() != 0){
			for(map<int,int>::const_iterator ii = BB.succ.begin(); ii != BB.succ.end(); ++ii){
				cout<<" "<<(*ii).first;
			}
		}
		cout <<endl;

		cout<<"\tpredecessors ";
		if(BB.preb.size() != 0){
			for(map<int,int>::const_iterator ii = BB.preb.begin(); ii != BB.preb.end(); ++ii){
				cout<<" "<<(*ii).first;
			}
		}
		cout <<endl;
		if(BB.instrNum == 0) continue;
		for(int j = BB.startNum; j <= BB.endNum; j++){
			cout<<"  "<<j<<" : ";
			fprint_instr(stdout,findInstrIndex(j));
		}
	}
	//printDom();
	//printIDom();
	
}

void CFG::printInstr(){
	int index = 0;
	simple_instr *tracer = inlist;

	while(tracer){
		//cout<<index<<" :";
		fprint_instr(stdout,tracer);
		tracer = tracer->next;
		index ++;
	}
	
}


	


	

