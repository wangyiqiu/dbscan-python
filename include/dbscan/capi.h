/*
A C API that instantiates templates for the C++ DBSCAN function. This API is
also useful in C++ because it allows for the selection of the number of
dimensions in the data.
*/

#pragma once

#ifndef DBSCAN_MIN_DIMS
#define DBSCAN_MIN_DIMS 2
#endif

#ifndef DBSCAN_MAX_DIMS
#define DBSCAN_MAX_DIMS 20
#endif


#ifdef __cplusplus

static_assert(DBSCAN_MIN_DIMS > 1);
static_assert(DBSCAN_MAX_DIMS >= DBSCAN_MIN_DIMS);

// intT* DBSCAN(int dim, intT n, floatT* PF, floatT epsilon, intT minPts, bool* coreFlag, intT* cluster);

// replace names from "dbscan/pbbs/parallel.h" with actual types

extern "C" int DBSCAN(int dim, int n, double* PF, double epsilon, int minPts, bool* coreFlag, int* cluster);

#else

#if DBSCAN_MIN_DIMS <= 1
#error DBSCAN_MIN_DIMS <= 1
#endif

#if DBSCAN_MAX_DIMS < DBSCAN_MIN_DIMS
#error DBSCAN_MAX_DIMS < DBSCAN_MIN_DIMS
#endif

int DBSCAN(int dim, int n, double* PF, double epsilon, int minPts, bool* coreFlag, int* cluster);

#ifndef __STDC_NO_VLA__
inline int DBSCAN_vla(int dim, int n, double PF[n][dim], double epsilon, int minPts, bool coreFlag[n], int cluster[n]) {
  return DBSCAN(dim, n, (void*)PF, epsilon, minPts, coreFlag, cluster);
}
#endif

#endif
