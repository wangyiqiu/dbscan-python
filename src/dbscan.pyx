# distutils: language = c++
# cython: language_level = 3

from libc.stdlib cimport malloc, free
from libcpp cimport bool
from Caller cimport DBSCAN as DBSCAN_cpp
import numpy as np

def DBSCAN(X, double eps=0.5, int min_samples=5):
    cdef int n = X.shape[0]
    dim = X.shape[1]

    if dim <= 1:
        print("Error: invalid input data dimensionality (has to >1).")
        return np.array([],dtype=np.int32), np.array([],dtype=np.bool_)

    if n <= 0:
        return np.array([],dtype=np.int32), np.array([],dtype=np.bool_)

    if n > 100000000:
        print("Warning: large n, the program behavior might be undefined due to overflow.")

    cdef double [:,:] X_view = X

    core_samples = np.empty(n, dtype=np.bool_)
    cdef bool [:] core_samples_view = core_samples

    labels = np.empty(n, dtype=np.int32)
    cdef int [:] labels_view = labels

    DBSCAN_cpp(&X_view[0,0], dim, n, eps, min_samples, &core_samples_view[0], &labels_view[0])

    return labels, core_samples
