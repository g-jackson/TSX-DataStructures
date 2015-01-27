#pragma once

#include <iostream>                             // cout
#include <iomanip>                              // setprecision

#ifndef HELPER_H
#define HELPER_H
#include "helper.h" 
#endif

#ifndef CONFIG_H
#define CONFIG_H
#include "config.h" 
#endif

#define K           1024                        
#define GB          (K*K*K)                     

typedef struct {
	int sharing;                                // sharing
	int nt;                                     // # threads
	UINT64 rt;                                  // run time (ms)
	UINT64 ops;                                 // ops
	UINT64 incs;                                // should be equal ops
	UINT64 aborts;                              //
} Result;

void outputConfig(int ncpu, int maxThread, char dateAndTime[256], int lineSz);
void outputResult(Result* r, int indx, int range);
void outputHeader();
void endResultOutput(Result* r, int indx);