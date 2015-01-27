#include "output.h"                             //

using namespace std;                            // cout

void output(int ncpu, int maxThread, char dateAndTime[256], int lineSz){
	getDateAndTime(dateAndTime, sizeof(dateAndTime));

	//
	// console output
	//
	cout << getHostName() << " " << getOSName() << " sharing " << (is64bitExe() ? "(64" : "(32") << "bit EXE)" << endl;
	#ifdef _DEBUG
		cout << " DEBUG";
	#else
		cout << " RELEASE";
	#endif
		cout << '\n' << " NCPUS=" << ncpu << endl << " RAM=" << (getPhysicalMemSz() + GB - 1) / GB << "GB " << endl << dateAndTime;
	#ifdef COUNTER64
		cout << " COUNTER64";
	#else
		cout << " COUNTER32" << endl;
	#endif
		cout << " NOPS=" << NOPS << endl << " NSECONDS=" << NSECONDS;
		cout << endl;
		cout << endl;
		cout << "Intel" << (cpu64bit() ? "64" : "32") << " family " << cpuFamily() << " model " << cpuModel() << " stepping " << cpuStepping() << " " << cpuBrandString() << endl;

	ALIGN(64) UINT64 cnt0;
	ALIGN(64) UINT64 cnt1;
	ALIGN(64) UINT64 cnt2;
	UINT64 cnt3;

	
	if ((&cnt3 >= &cnt0) && (&cnt3 < (&cnt0 + lineSz / sizeof(UINT64))))
		cout << "Warning: cnt3 shares cache line used by cnt0" << endl;
	if ((&cnt3 >= &cnt1) && (&cnt3 < (&cnt1 + lineSz / sizeof(UINT64))))
		cout << "Warning: cnt3 shares cache line used by cnt1" << endl;
	if ((&cnt3 >= &cnt2) && (&cnt3 < (&cnt2 + lineSz / sizeof(UINT64))))
		cout << "Warning: cnt2 shares cache line used by cnt1" << endl;

	cout << endl;
}