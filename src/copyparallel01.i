%module copyparallel01

%{
#define SWIG_FILE_WITH_INIT
#include "iostream"
#include "fstream"
#include "vector"
#include "sstream"
#include "cstdlib"
#include "cstring"
#include "sys/time.h"
#include "omp.h"
#include "cmath"

void callFunction();


%}

extern void callFunction();