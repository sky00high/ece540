#ifndef DFA_H
#define DFA_h
extern "C"{
	#include <simple.h>
}

#include <assert.h>
#include "BitVector.h"
#include <iostream>
#include <map>
#include "DFG.h"
#include "cfg.h"


class DFA{

	int direction;
	int path;
	map<int,BitVector> genSet;
	map<int,BitVector> killSet;
	map<int,BitVector> inSet;
	map<int,BitVector> outSet;
	CFG *cfg;
	int vectorWidth;
	void iniOutSet();
	void iniInSet();
	void analysisBackward();
	void analysisForward();

	public:
	static const int DIRECTION_FORWARD = 0;
	static const int DIRECTION_BACKWARD = 1;
	static const int PATH_ALLWAY = 0;
	static const int PATH_ANYWAY = 1;
	map<int,BitVector> getOutSet(){return outSet;}
	DFA(int direction, int path, map<int,BitVector> genSet,
		 map<int,BitVector> killSet, CFG *cfg, int vectorWidth);

};

#endif
