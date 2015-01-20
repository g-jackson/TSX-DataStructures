#include <iostream>                             // cout
#include <iomanip>                              // setprecision
#include "helper.h"                             //

using namespace std;                            // cout

#define K           1024                        //
#define GB          (K*K*K)                     //
#define NOPS        100							//
#define NSECONDS    2                           // run each test for NSECONDS
#define COUNTER64                               // comment for 32 bit counter

#ifdef COUNTER64
#define VINT    UINT64                          //  64 bit counter
#else
#define VINT    UINT                            //  32 bit counter
#endif

#define ALIGNED_MALLOC(sz, align) _aligned_malloc(sz, align) // cache line size
#define GINDX(n)    (g+n*lineSz/sizeof(VINT))   //

UINT64 tstart;                                  // start of test in ms
int sharing;                                    // % sharing
int lineSz;                                     // cache line size
int maxThread;                                  // max # of threads

THREADH *threadH;                               // thread handles
UINT64 *ops;                                    // for ops per thread
UINT64 *aborts;                                 // for counting aborts

typedef struct {
    int sharing;                                // sharing
    int nt;                                     // # threads
    UINT64 rt;                                  // run time (ms)
    UINT64 ops;                                 // ops
    UINT64 incs;                                // should be equal ops
    UINT64 aborts;                              //
} Result;

Result *r;                                      // results
UINT indx;                                      // results index

volatile VINT *g;                               // NB: position of volatile

//
// test memory allocation [see lecture notes]
//
ALIGN(64) UINT64 cnt0;
ALIGN(64) UINT64 cnt1;
ALIGN(64) UINT64 cnt2;
UINT64 cnt3;                                    // NB: in Debug mode allocated in cache line occupied by cnt0

class Node {
public:
	INT64 key;
	Node *left;
	Node *right;
	Node() { key = 0; right = left = NULL; } // default constructor
};

class BST {
public:
	Node *root; // initially NULL
	volatile long lock; 
	INT64 contains(INT64); // return 1 if key in tree
	INT64 add(Node*); // add node to tree
	Node* remove(INT64); // remove key from tree
	// int rtmAdd(Node*); // add node to tree
	// Node* rtmRemove(INT64); // remove key from tree
	INT64 hleAdd(Node*);
	Node* hleRemove(INT64);
	INT64 isValidTree();
	INT64 maxVal(Node*);
	INT64 minVal(Node*);
	INT64 isValid(Node*);
	BST(){
		root = NULL;
		lock = 0;
	}; // constructor
};

INT64 BST::minVal(Node* in){
	if (in == NULL){
		return -1;
	}
	else if (in->left != NULL){
		return minVal(in->left);
	}
	else{
		return in->key;
	}
}

INT64 BST::maxVal(Node* in){
	if (in == NULL){
		return -1;
	}
	else if (in->right != NULL){
		return minVal(in->right);
	}
	else{
		return in->key;
	}
}

INT64 BST::isValidTree(){
	return isValid(root);
}

INT64 BST::isValid(Node* in){
	if (in == NULL)
		return(true);

	/* false if the max of the left is > than us */
	if (in->left != NULL && maxVal(in->left) > in->key)
		return(false);

	/* false if the min of the right is <= than us */
	if (in->right != NULL && minVal(in->right) < in->key)
		return(false);

	/* false if, recursively, the left or right is not a BST */
	if (!isValid(in->left) || !isValid(in->right))
		return(false);

	/* passing all that, it's a BST */
	return(true);
}

INT64 BST::contains(INT64 n){
	Node* volatile *pp = &root;
	Node *p = root;
	while (p->key != n){
		if (p->key < n){
			if (p->right != NULL){
				p = p->right;
			}
			else{
				return 0;
			}
		}
		else{
			if (p->left != NULL){
				p = p->left;
			}
			else{
				return 0;
			}
		}
	}
	return 1;
}

INT64 BST::add(Node *n){
	Node* volatile *pp = &root;
	Node *p = root;
	while (p) {
		if (n->key < p->key) {
			pp = &p->left;
		}
		else if (n->key > p->key) {
			pp = &p->right;
		}
		else {
			return 0;
		}
		p = *pp;
	}
	*pp = n;
	return 1;
}

Node* BST::remove(INT64 key){
	Node **pp = &root;
	Node *p = root;
	while (p) {
		if (key < p->key) {
			pp = &p->left;
		}
		else if (key > p->key) {
			pp = &p->right;
		}
		else {
			break;
		}
		p = *pp;
	}
	if (p == NULL)	return NULL;
	if (p->left == NULL && p->right == NULL) {
		*pp = NULL; // NO children
	}
	else if (p->left == NULL) {
		*pp = p->right; // ONE child
	}
	else if (p->right == NULL) {
		*pp = p->left; // ONE child
	}
	else {
		Node *r = p->right; // TWO children
		Node **ppr = &p->right; // find min key in right sub tree
		while (r->left) {
			ppr = &r->left;
			r = r->left;
		}
		p->key = r->key; // could move...
		p = r; // node instead
		*ppr = r->right;
	}
	return p; // return removed node
}

INT64 BST::hleAdd(Node *n){

}

Node* BST::hleRemove(INT64 key){

}

//shared thread variables
BST tree;
volatile long lock = 0;
int range = 0;

//test and test and set lock
void TTS(){
	for (int i = 0; i < NOPS; i++) {
		int in = rand() % range*2;	//reduce call to rand by using odd to add rand/2 and even to remove rand/2
		do {										
			while (lock == 1)_mm_pause();			
		} while (InterlockedExchange(&lock, 1));
		if (in %2 ==0){
			in = in/2;
			//cout << " removing " << in;
			Node* removed = tree.remove(in);
			delete removed;
		}
		else{
			Node* newNode= new Node();
			in = in/2;
			newNode->key = in;
			if(tree.add(newNode)){
				//cout << " added" << in;
			}
			else{
				delete newNode;
				//cout << " couldn't add " << in;
			}
		}
		lock = 0;
	}
}

//HLE Lock
void HLE(){
	for (int i = 0; i < NOPS; i++) {
		int in = rand() % range * 2;	//reduce call to rand by using odd to add rand/2 and even to remove rand/2
		while (_InterlockedExchange_HLEAcquire(&lock, 1)){
			do {
				_mm_pause();
			} while (lock == 1);
		}
		if (in % 2 == 0){
			in = in / 2;
			//cout << " removing " << in;
			Node* removed = tree.hleRemove(in);
			delete removed;
		}
		else{
			Node* newNode = new Node();
			in = in / 2;
			newNode->key = in;
			if (tree.hleAdd(newNode)){
				//cout << " added" << in;
			}
			else{
				delete newNode;
				//cout << " couldn't add " << in;
			}
		}
		_Store_HLERelease(&lock, 0);
	}
}

//RTM Lock
void RTM(){
	for (int i = 0; i < NOPS; i++) {
		int in = rand() % range * 2;	//reduce call to rand by using odd to add rand/2 and even to remove rand/2
		if (in % 2 == 0){
			in = in / 2;
			//cout << " removing " << in;
			Node* removed = tree.remove(in);
			_xend();
			delete removed;
		}
		else{
			Node* newNode = new Node();
			in = in / 2;
			newNode->key = in;
			_xbegin();
			if (tree.add(newNode)){
				//cout << " added" << in;
			}
			else{
				delete newNode;
				//cout << " couldn't add " << in;
			}
			_xend();
		}
	}
}

//
// worker threads run code for NSECONDS
//
WORKER worker(void *vthread){
    int thread = (int)((size_t) vthread);

    UINT64 n = 0;
    volatile VINT *opsCompete = GINDX(thread);

    UINT64 nabort = 0;

    runThreadOnCPU(thread % ncpu);
    while (1) {

        //
        // do some work
        //
        TTS();
		//RTM()
		//HLE()

        n += NOPS;

        //
        // check if runtime exceeded
        //
        if ((getWallClockMS() - tstart) > NSECONDS*1000)
            break;

    }
    ops[thread] = n;
    aborts[thread] = nabort;
    return 0;
}

//
// main
//

int main()
{
    ncpu = getNumberOfCPUs();   // number of logical CPUs
    maxThread = 2 * ncpu;       // max number of threads

    //
    // get date
    //
    char dateAndTime[256];
    getDateAndTime(dateAndTime, sizeof(dateAndTime));

    //
    // console output
    //
    cout << getHostName() << " " << getOSName() << " sharing " << (is64bitExe() ? "(64" : "(32") << "bit EXE)" ;
#ifdef _DEBUG
    cout << " DEBUG";
#else
    cout << " RELEASE";
#endif
    cout << " NCPUS=" << ncpu << " RAM=" << (getPhysicalMemSz() + GB - 1) / GB << "GB " << dateAndTime << endl;
#ifdef COUNTER64
    cout << "COUNTER64";
#else
    cout << "COUNTER32";
#endif
#ifdef FALSESHARING
    cout << " FALSESHARING";
#endif
	cout << " NOPS=" << NOPS << " NSECONDS=" << NSECONDS;
#ifdef USEPMS
    cout << " USEPMS";
#endif
    cout << endl;
    cout << "Intel" << (cpu64bit() ? "64" : "32") << " family " << cpuFamily() << " model " << cpuModel() << " stepping " << cpuStepping() << " " << cpuBrandString() << endl;
#ifdef USEPMS
    cout << "performance monitoring version " << pmversion() << ", " << nfixedCtr() << " x " << fixedCtrW() << "bit fixed counters, " << npmc() << " x " << pmcW() << "bit performance counters" << endl;
#endif

    //
    // get cache info
    //
    lineSz = getCacheLineSz();
    //lineSz *= 2;

    if ((&cnt3 >= &cnt0) && (&cnt3 < (&cnt0 + lineSz/sizeof(UINT64))))
        cout << "Warning: cnt3 shares cache line used by cnt0" << endl;
    if ((&cnt3 >= &cnt1) && (&cnt3 < (&cnt1 + lineSz / sizeof(UINT64))))
        cout << "Warning: cnt3 shares cache line used by cnt1" << endl;
    if ((&cnt3 >= &cnt2) && (&cnt3 < (&cnt2 + lineSz / sizeof(UINT64))))
        cout << "Warning: cnt2 shares cache line used by cnt1" << endl;


    //
    // check if RTM supported
    //
    if (!rtmSupported()) {
        cout << "RTM (restricted transactional memory) NOT supported by this CPU" << endl;
        quit();
        return 1;
    }

    cout << endl;

    //
    // allocate global variable
    //
    // NB: each element in g is stored in a different cache line to stop false sharing
    //
    threadH = (THREADH*) ALIGNED_MALLOC(maxThread*sizeof(THREADH), lineSz);             // thread handles
    ops = (UINT64*) ALIGNED_MALLOC(maxThread*sizeof(UINT64), lineSz);                   // for ops per thread
    aborts = (UINT64*) ALIGNED_MALLOC(maxThread*sizeof(UINT64), lineSz);                // for counting aborts
    g = (VINT*) ALIGNED_MALLOC((maxThread + 1)*lineSz, lineSz);                         // local and shared global variables


    r = (Result*) ALIGNED_MALLOC(5*maxThread*sizeof(Result), lineSz);                   // for results
    memset(r, 0, 5*maxThread*sizeof(Result));                                           // zero

    indx = 0;

    //
    // use thousands comma separator
    //
    setCommaLocale();

    //
    // header
    //
    cout << setw(4) << "nt";
    cout << setw(6) << "rt";
    cout << setw(16) << "ops";
    cout << setw(6) << "rel";
    cout << setw(8) << "commit";
	cout << setw(10) << "range";
	cout << setw(10) << "valid";
    cout << endl;
    cout << setw(4) << "--";        // nt
    cout << setw(6) << "--";        // rt
    cout << setw(16) << "---";      // ops
    cout << setw(6) << "---";       // rel
    cout << setw(8) << "------";
	cout << setw(10) << "--------";
	cout << setw(10) << "--------";
    cout << endl;

    //
    // boost process priority
    // boost current thread priority to make sure all threads created before they start to run
    //
#ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
    //
    // run tests
    //
    UINT64 ops1 = 1;

	for (int i = 1; i < 6; i++){
		range = (int)(pow(2,(4*i)));
		for (int nt = 1; nt <= maxThread; nt *= 2, indx++) {
            //
            //  zero shared memory
            //
            for (int thread = 1; thread < nt; thread++)
                *(GINDX(thread)) = 0;   // thread local
            *(GINDX(maxThread)) = 0;    // shared


            //
            // get start time
            //
            tstart = getWallClockMS();
			
            //
            // create worker threads
            //
            for (int thread = 0; thread < nt; thread++)
                createThread(&threadH[thread], worker, (void*)(size_t)thread);

            //
            // wait for ALL worker threads to finish
            //
            waitForThreadsToFinish(nt, threadH);
            UINT64 rt = getWallClockMS() - tstart;

            //
            // save results and output summary to console
            //
            for (int thread = 0; thread < nt; thread++) {
                r[indx].ops += ops[thread];
                r[indx].incs += *(GINDX(thread));		
                r[indx].aborts += aborts[thread];
			}
            r[indx].incs += *(GINDX(maxThread));
            if ((sharing == 0) && (nt == 1))
                ops1 = r[indx].ops;
            r[indx].nt = nt;
            r[indx].rt = rt;

            cout << setw(4) << nt;
            cout << setw(6) << fixed << setprecision(2) << (double) rt / 1000;
            cout << setw(16) << r[indx].ops;
            cout << setw(6) << fixed << setprecision(2) << (double) r[indx].ops / ops1;
            cout << setw(7) << fixed << setprecision(0) << 100.0 * (r[indx].ops - r[indx].aborts) / r[indx].ops << "%    ";
			cout << setw(5) << range;
			if (tree.isValidTree()){
				cout  << "   valid tree";
			}
			else{
				cout <<  " invalid tree";
			}
            cout << endl;

            //
            // delete thread handles
            //
            for (int thread = 0; thread < nt; thread++)
                closeThread(threadH[thread]);
		}
	}
		
    cout << endl;

    //
    // output results so they can easily be pasted into a spread sheet from console window
    //
    setLocale();
    cout << "sharing/nt/rt/ops/incs";
    cout << "/aborts";
    cout << endl;
    for (UINT i = 0; i < indx; i++) {
        cout << "/"  << r[i].nt << "/" << r[i].rt << "/"  << r[i].ops << "/" << r[i].incs;
        cout << "/" << r[i].aborts;
        cout << endl;
    }
    cout << endl;

    quit();

    return 0;

}

// eof