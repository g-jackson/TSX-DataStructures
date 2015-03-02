#include "output.h"                             //
#include <iostream>
#include <fstream>

using namespace std;                            // cout

void outputConfig(int ncpu, int maxThread, int lineSz){
	// get date
	char dateAndTime[256];
	getDateAndTime(dateAndTime, sizeof(dateAndTime));

	//
	// console output
	//
	cout << getHostName() << " " << getOSName() << " sharing " << (is64bitExe() ? "(64" : "(32") << "bit EXE)" << endl;
	cout << dateAndTime << endl;
	cout << " NCPUS=" << ncpu << endl << " RAM=" << (getPhysicalMemSz() + GB - 1) / GB << "GB " << endl;
#ifdef COUNTER64
	cout << " COUNTER64";
#else
	cout << " COUNTER32" << endl;
#endif
	cout << " NOPS=" << NOPS << endl;
	cout << " NSECONDS=" << NSECONDS << endl;
#if  MODIFY == 0
	cout << " MODIFY=" << MODIFY << " (modifying OPs = 0%)" << endl;
#else
	cout << " MODIFY=" << MODIFY << " (modifying OPs = " << ((float)(NOPS) / (float)(MODIFY)) << "%)" << endl;
#endif

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

void outputHeader(){
	// use thousands comma separator
	setCommaLocale();

	// header
	cout << setw(4) << "nt";
	cout << setw(6) << "rt";
	cout << setw(16) << "ops";
	cout << setw(6) << "rel";
	cout << setw(8) << "commit";
	cout << setw(10) << "range";
	cout << endl;
	cout << setw(4) << "--";        // nt
	cout << setw(6) << "--";        // rt
	cout << setw(16) << "---";      // ops
	cout << setw(6) << "---";       // rel
	cout << setw(8) << "------";
	cout << setw(10) << "--------";
	cout << endl;
}

void outputResult(Result* r, int indx, int range){
	cout << setw(4) << r[indx].nt;
	cout << setw(6) << fixed << setprecision(2) << (double)r[indx].rt / 1000;
	cout << setw(16) << r[indx].ops;
	cout << setw(6) << fixed << setprecision(2) << (double)r[indx].ops / r[indx].ops;
	cout << setw(7) << fixed << setprecision(0) << 100.0 * (r[indx].ops - r[indx].aborts) / r[indx].ops << "%    ";
	cout << setw(5) << range;
	cout << endl;
}

void endResultOutput(Result* r, int indx){
	setLocale();
	if (LOG){
		ofstream logfile;
		logfile.open("log.txt");
		logfile << "nt/rt/ops/incs/aborts";
		logfile << endl;
		cout << "Writing to file 'log.txt'.";
		for (int i = 0; i < indx; i++) {
			logfile << r[i].nt << "/" << r[i].rt << "/" << r[i].ops << "/" << r[i].incs << "/" << r[i].aborts << endl;
		}
		logfile.close();
	}
	cout << endl;
}
