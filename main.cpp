#ifndef HELPER_H
#define HELPER_H
#include "helper.h" 
#endif
#ifndef CONFIG_H
#define CONFIG_H
#include "config.h" 
#endif
#ifndef OUTPUT_H
#define OUTPUT_H
#include "output.h"
#endif

using namespace std;

// select data structure - 0:Binary Tree - 1:Simple Skip List - 2:LockedSkipList
#if  DATASTRUCTURE == 0
#include "BinTree.h"
binTree* ds;
binTree* newDS(){
	return new bintree();
}
#endif
#if DATASTRUCTURE == 1
#include "simpleslist.h"
SimpleSList* ds;
SimpleSList* newDS(){
	return new SimpleSList();
}
#endif
#if  DATASTRUCTURE == 2
#include "Skiplist.h"
LazySkipList* ds;
LazySkipList* newDS(){
	return new LazySkipList();
}
#endif

#define COUNTER64                               // comment for 32 bit counter
#define VINT    UINT64                          //  64 bit counter
#define ALIGNED_MALLOC(sz, align) _aligned_malloc(sz, align) // cache line size
#define GINDX(n)    (g+n*lineSz/sizeof(VINT))   //

//variable initialisation
UINT64 tstart;                                  // start of test in ms
int lineSz;                                     // cache line size
int maxThread;                                  // max # of threads

THREADH *threadH;                               // thread handles
UINT64 *ops;                                    // for ops per thread
UINT64 *aborts;                                 // for counting aborts
Result *r;                                      // results
UINT indx;                                      // results index

volatile VINT *g;                               // NB: position of volatile

//shared thread variables
int range = 0;

// rand generator for 64bit ints
UINT64 nrand(UINT64 &r)
{
	r ^= r >> 12;   // a
	r ^= r << 25;   // b
	r ^= r >> 27;   // c
	return r * 2685821657736338717LL;
}

// simple skip TTS Lock
void skipTTS(){
	int randin = rand();
	int reads = 1;
	int readno = MODIFY % NOPS;
	Node* node;
	for (int i = 0; i < NOPS; i++) {
		randin = rand();
		int in = randin % (range * 2);	//reduce call to rand by using odd to add rand/2 and even to remove rand/2
		if (reads != 0){
			do {
				while (ds->lock == 1)_mm_pause();
			} while (InterlockedExchange(&ds->lock, 1));
			ds->contains(in/2);
			ds->lock = 0;
			if (reads > readno){
				reads++;
			}
			else{
				reads = 0;
			}
		}
		else{
			if (in % 2 == 0){
				in = in / 2;
				node = NULL;
				do {
					while (ds->lock == 1)_mm_pause();
				} while (InterlockedExchange(&ds->lock, 1));
				ds->remove(in, node);
				ds->lock = 0;
				delete node;
			}
			else{
				in = in / 2;
				node = new Node(in, randomLevel(ds->max));
				do {
					while (ds->lock == 1)_mm_pause();
				} while (InterlockedExchange(&ds->lock, 1));
					if(!ds->add(in)){
						delete node;
				}
				ds->lock = 0; //need to move memory management out of lock
			}
		}
	}
}

// fine grained skip TTS Lock
void skipPTTS(){
	int randin;
	for (int i = 0; i < NOPS; i++) {
		randin = rand();
		int in = randin % (range * 2);	//reduce call to rand by using odd to add rand/2 and even to remove rand/2
		if (in % 2 == 0){
			in = in/2;
			ds->remove(in);
		}
		else{
			in = in / 2;
			ds->add(in);
		}
	}
}

// BinTree TTS Lock
/*
void TTS(){
	UINT64 randin = rand();
	UINT64 result;
	for (int i = 0; i < NOPS; i++) {
		randin = nrand(randin);
		UINT64 in = randin % (range*2);	//reduce call to rand by using odd to add rand/2 and even to remove rand/2

		if (in %2 ==0){
			in = in/2;
			do {
				while (ds->lock == 1)_mm_pause();
			} while (InterlockedExchange(&ds->lock, 1));
			Node* removed = ds->remove(in);
			ds->lock = 0;
			delete removed;
		}
		else{
			Node* newNode= new Node();
			in = in/2;
			newNode->key = in;
			do {
				while (ds->lock == 1)_mm_pause();
			} while (InterlockedExchange(&ds->lock, 1));
			result = ds->add(newNode);
			ds->lock = 0;
			if(!result){
				delete newNode;
			}
		}
		
	}
}
*/

// BinTree HLE Lock
/*
void HLE(){
	for (int i = 0; i < NOPS; i++) {
		UINT64 in = rand() % range * 2;	//reduce call to rand by using odd to add rand/2 and even to remove rand/2
		while (_InterlockedExchange_HLEAcquire(&tree->lock, 1)){
			do {
				_mm_pause();
			} while (tree->lock == 1);
		}
		if (in % 2 == 0){
			in = in / 2;
			//cout << " removing " << in;
			Node* removed = tree->remove(in);
			delete removed;
		}
		else{
			Node* newNode = new Node();
			in = in / 2;
			newNode->key = in;
			if (tree->add(newNode)){
				//cout << " added" << in;
			}
			else{
				delete newNode;
				//cout << " couldn't add " << in;
			}
		}
		_Store_HLERelease(&tree->lock, 0);
	}
}
*/

// worker threads run code for NSECONDS
WORKER worker(void *vthread){
    int thread = (int)((size_t) vthread);

    UINT64 n = 0;
    volatile VINT *opsCompete = GINDX(thread);

    UINT64 nabort = 0;

    runThreadOnCPU(thread % ncpu);
    while (1) {
		//skipPTTS();
		skipTTS();
        //TTS();
		//RTM()
		//HLE()

        n += NOPS;
        // check if runtime exceeded
        if ((getWallClockMS() - tstart) > NSECONDS*1000)
            break;

    }
    ops[thread] = n;
    aborts[thread] = nabort;
    return 0;
}

// main
int main() 
{
    ncpu = getNumberOfCPUs();   // number of logical CPUs
	maxThread = 2 * ncpu;       // max number of threads

    // get cache info
    lineSz = getCacheLineSz();
    //lineSz *= 2;

	//output config and processor info
	outputConfig(ncpu,  maxThread, lineSz);

    // check if RTM supported
	/*
	if (!rtmSupported()) {
        cout << "RTM (restricted transactional memory) NOT supported by this CPU" << endl;
        quit();
        return 1;
    }
	*/

    //allocate global variables NB: each element in g is stored in a different cache line to stop false sharing
    threadH = (THREADH*) ALIGNED_MALLOC(maxThread*sizeof(THREADH), lineSz);             // thread handles
    ops = (UINT64*) ALIGNED_MALLOC(maxThread*sizeof(UINT64), lineSz);                   // for ops per thread
    aborts = (UINT64*) ALIGNED_MALLOC(maxThread*sizeof(UINT64), lineSz);                // for counting aborts
    g = (VINT*) ALIGNED_MALLOC((maxThread + 1)*lineSz, lineSz);                         // local and shared global variables
    r = (Result*) ALIGNED_MALLOC(5*maxThread*sizeof(Result), lineSz);                   // for results
    memset(r, 0, 5*maxThread*sizeof(Result));                                           // zero

    indx = 0;

    // run tests
	outputHeader();
    UINT64 ops1 = 1;
	for (int i = 1; i < 5; i++){
		range = (int)(pow(2,(4*i)));
		ds = newDS();
		if (FILL){ 
			ds->fill(range); 
		};
		for (int nt = 1; nt <= 4; nt *= 2, indx++) {
            //  zero shared memory
			for (int thread = 1; thread < nt; thread++){
				*(GINDX(thread)) = 0;   // thread local
			}
			*(GINDX(maxThread)) = 0;    // shared

            // get start time
            tstart = getWallClockMS();
			
            // create worker threads
			for (int thread = 0; thread < nt; thread++){
				createThread(&threadH[thread], worker, (void*)(size_t)thread);
			}


            // wait for ALL worker threads to finish
            waitForThreadsToFinish(nt, threadH);
            UINT64 rt = getWallClockMS() - tstart;

            // save results and output summary to console
            for (int thread = 0; thread < nt; thread++) {
                r[indx].ops += ops[thread];
                r[indx].incs += *(GINDX(thread));		
                r[indx].aborts += aborts[thread];
			}
            r[indx].incs += *(GINDX(maxThread));
			if (nt == 1){
				ops1 = r[indx].ops;
			}
            r[indx].nt = nt;
            r[indx].rt = rt;
			outputResult(r, indx, range);

            // delete thread handles
            for (int thread = 0; thread < nt; thread++)
                closeThread(threadH[thread]);
		}
	}
		
	// output results to logfile
	endResultOutput(r, indx);

    quit();
    return 0;
}
// eof