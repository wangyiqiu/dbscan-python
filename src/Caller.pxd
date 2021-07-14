from libcpp cimport bool

cdef extern from "Caller.cpp":
    pass

# Declare the class with cdef
cdef extern from "Caller.h" namespace "Wrapper":
    cdef cppclass Caller:
        Caller() except +
        Caller(double *, int, int) except +
        int* computeDBSCAN(double, int, bool *, int *)
