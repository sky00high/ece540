using namespace std;
extern "C"{
	#include <simple.h>
}

#include <assert.h>
#include "BitVector.h"
#include <iostream>
#include <map>
#include "cfg.h"
#include "DFA.h"
//helpers
void DFA::iniOutSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum();i++){
		this->outSet[i].ini(vectorWidth);
			
	}
	if(direction = DIRECTION_FORWARD && path == PATH_ALLWAY){
		for(int i = 1 ; i<this->cfg->getTotalBlockNum() - 1;i++){
			this->outSet[i].setAll();
		}
	}
}
void DFA::iniInSet(){
	for(int i = 0; i < this->cfg->getTotalBlockNum();i++){
		this->inSet[i].ini(vectorWidth);
	}
}
void DFA::analysisBackward(){
	bool changed = true;
	BitVector newOut;
	while(changed){
		changed = false;
		for(int i = 1; i < this->cfg->getTotalBlockNum() - 1; i++){
			BitVector oldIn = inSet[i];

			newOut.ini(vectorWidth);
			map<int,int>succ = cfg->getBlockSucc(i);

			for(map<int,int>::const_iterator ii = succ.begin(); 
				ii != succ.end(); ii++){
				newOut += inSet[(*ii).first];
			}

			outSet[i] = newOut;
			newOut-=killSet[i];
			newOut+=genSet[i];
			inSet[i] = newOut;
			//inSet[i] = outSet[i] - killSet[i] + genSet[i];
			if(inSet[i].value != oldIn.value) changed = true;
		}
	}
	newOut.ini(vectorWidth);
	for(map<int,int>::const_iterator ii = cfg->getBlockSucc(0).begin(); 
		ii != cfg->getBlockSucc(0).end(); ii++){
		newOut += inSet[(*ii).first];
	}
	outSet[0] = newOut;
}


void DFA::analysisForward(){
	bool changed = true;
	BitVector newIn;
	while(changed){
		changed = false;
		for(int i = 1; i < this->cfg->getTotalBlockNum() - 1; i++){
			BitVector oldOut = outSet[i];

			newIn.ini(vectorWidth);
			map<int,int>pred = cfg->getBlockPred(i);

			if(path == PATH_ANYWAY){
				for(map<int,int>::const_iterator ii = pred.begin(); 
					ii != pred.end(); ii++){
					newIn += outSet[(*ii).first];
				}
			} else{
				map<int,int>::const_iterator ii = pred.begin();
				newIn+=outSet[(*ii).first];
				ii++;
				while(ii != pred.end()){
					//newIn.join(inSet[(*ii).first]);
					newIn = BitVector::join(newIn, outSet[ii->first]);
					ii++;
				}
			}

			inSet[i] = newIn;
			newIn-=killSet[i];
			newIn+=genSet[i];
			outSet[i] = newIn;
			//inSet[i] = outSet[i] - killSet[i] + genSet[i];
			if(outSet[i].value != oldOut.value) changed = true;
		}
	}
	newIn.ini(vectorWidth);
	map<int,int>pred = cfg->getBlockPred(cfg->getTotalBlockNum() - 1);//the predessor of exit
	map<int,int>::const_iterator ii = pred.begin(); 
	newIn += outSet[ii->first];
	ii++;
	while(ii != pred.end()){
	//	newIn.join(inSet[ii->first]);
		newIn = BitVector::join(newIn, outSet[ii->first]);
		ii++;
	}
	inSet[cfg->getTotalBlockNum()-1] = newIn;
	outSet[cfg->getTotalBlockNum()-1] = newIn;
}
DFA::DFA(int direction, int path, map<int,BitVector> genSet,
		 map<int,BitVector> killSet, CFG *cfg, int vectorWidth){
	this->vectorWidth = vectorWidth;
	this->direction = direction;
	this->path = path;
	this->cfg = cfg;
	this->genSet = genSet;
	this->killSet = killSet;

	iniOutSet();
	iniInSet();

	if(direction == DIRECTION_FORWARD){
		analysisForward();
	} else {
		analysisBackward();
	}
}


