#ifndef HELPER_H
#define HELPER_H
#include "helper.h" 
#endif
#ifndef CONFIG_H
#define CONFIG_H
#include "config.h" 
#endif
#include "BinTree.h"
#include "output.h"

using namespace std;                            // cout

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
Result *r;                                      // results
UINT indx;                                      // results index

volatile VINT *g;                               // NB: position of volatile

//shared thread variables
binTree tree;
volatile long lock = 0;
int range = 0;

// TTS lock
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

// HLE Lock
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

// RTM Lock
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

// worker threads run code for NSECONDS
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

// main
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
    // get cache info
    //
    lineSz = getCacheLineSz();
    //lineSz *= 2;
	outputConfig(ncpu,  maxThread,  dateAndTime,  lineSz);

	//
    // check if RTM supported
    //
	/*
	if (!rtmSupported()) {
        cout << "RTM (restricted transactional memory) NOT supported by this CPU" << endl;
        quit();
        return 1;
    }
	*/


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
	outputHeader();
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
			outputResult(r, indx, range);

            //
            // delete thread handles
            //
            for (int thread = 0; thread < nt; thread++)
                closeThread(threadH[thread]);
		}
	}
		

	// output results so they can easily be pasted into a spread sheet from console window

	endResultOutput(r, indx);

    quit();

    return 0;

}

// eof