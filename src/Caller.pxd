from libcpp cimport bool

cdef extern from "Caller.cpp":
    pass

# Declare the class with cdef
cdef extern from "Caller.h":
    int* DBSCAN(double *, int, int, double, int, bool *, int *)
