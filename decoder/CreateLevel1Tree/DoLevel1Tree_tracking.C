//this file is almost identical to ReadEvTree.C but has LEVEL1_Tree_DEBUG defined
//such that I can build different tree branches for debugging

//#define LEVEL1_Tree_DEBUG 0
#include "ReadEvTree_tracking.C"

void DoLevel1Tree_tracking(int run_start, int run_end)
{ 
  DoLevel1Tree(run_start, run_end);
}
