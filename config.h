#define NOPS			100						// number of ops per iteration per thread
#define MODIFY			0						// MODIFY:NOPS = ratio of operations that are modifications to total ops, remainder are reads
#define NSECONDS		2                       // run each test for NSECONDS
#define DATASTRUCTURE	1						// select data structure - 0:Binary Tree - 1:Simple Skip List - 2:LockedSkipList
#define FILL			0						// before using datastructure half fill it 0=No 1=Yes
#define LOG				0						// 0 = no log file 1 = log file
#define MAX_LEVEL		7						// max height for skiplists