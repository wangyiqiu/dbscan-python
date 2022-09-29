#pragma once

// intT* DBSCAN(floatT* PF, int dim, intT n, floatT epsilon, intT minPts, bool* coreFlag, intT* cluster);

// replace names from "dbscan/pbbs/parallel.h" with actual types

int DBSCAN(double* PF, int dim, int n, double epsilon, int minPts, bool* coreFlag, int* cluster);
