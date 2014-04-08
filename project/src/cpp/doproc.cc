#include <stdio.h>
extern "C" { 
#include <simple.h>
}

#include  "cfg.h"
#include "RD.h"
#include "DFG.h"
#include "RD.h"
// data structures you should consider using are vector and hash_map from the STL
// refer to the following link as a starting point if you are not familiar with them: 
// http://www.sgi.com/tech/stl/Vector.html
// http://www.sgi.com/tech/stl/hash_map.html 

simple_instr* do_procedure (simple_instr *inlist, char *proc_name)
{
    // build flow control graph 
	 CFG *thisCFG = new CFG(inlist);
	 thisCFG->findIDom();
	 thisCFG->genLoopHeader();
	 delete thisCFG;

	 thisCFG = new CFG(inlist);
	 thisCFG->findIDom();
	 RD *thisRD = new RD(thisCFG,inlist);

	 thisRD->genDefList();
	 cout<<"Definitions "<<proc_name<<" "<<thisRD->getDefNum()<<endl;
	 thisRD->printDefList();
	 cout<<endl;

	 thisRD->genGenSet();
	 cout<<"eval_RD_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisRD->printGenSet();
	 cout<<endl;

	 thisRD->genKillSet();
	 cout<<"kill_RD_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisRD->printKillSet();
	 cout<<endl;

	 thisRD->genRDOutSet();
	 cout<<"available_exprs "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisRD->printRDOutSet();
	 cout<<endl;
	 delete thisCFG;
	 delete thisRD;
    return inlist;
}
