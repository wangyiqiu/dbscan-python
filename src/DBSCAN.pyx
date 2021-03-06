# distutils: language = c++

from libc.stdlib cimport malloc, free
from libcpp cimport bool
from Caller cimport Caller
import numpy as np

def DBSCAN(X, double eps=0.5, int min_samples=5):
    cdef int n = X.shape[0]
    dim = X.shape[1]

    if dim <= 1:
        print("Error: invalid input data dimensionality (has to >1).")
        return np.array([],dtype=np.int32), np.array([],dtype=np.bool)

    if n <= 0:
        return np.array([],dtype=np.int32), np.array([],dtype=np.bool)

    if n > 100000000:
        print("Warning: large n, the program behavior might be undefined due to overflow.")

    cdef double [:,:] X_view = X

    my_instance = new Caller(&X_view[0,0], dim, n)

    core_samples = np.empty(n, dtype=np.bool)
    cdef bool [:] core_samples_view = core_samples

    labels = np.empty(n, dtype=np.int32)
    cdef int [:] labels_view = labels

    try:
        my_instance.computeDBSCAN(eps, min_samples, &core_samples_view[0], &labels_view[0])
    finally:
        del my_instance

    return labels, core_samples
