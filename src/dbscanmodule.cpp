#include "Python.h"
#include "numpy/arrayobject.h"
#include "dbscan/capi.h"


static PyObject* DBSCAN_py(PyObject* self, PyObject* args, PyObject *kwargs)
{
    PyObject *Xobj;
    PyArrayObject *X = NULL;
    double eps = 0.5;
    int min_samples = 5;

    static const char *kwlist[] = {"X", "eps", "min_samples", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|di:DBSCAN", (char**)kwlist,
                                     &Xobj, &eps, &min_samples))
    {
        return NULL;
    }

    // Check the number of dimensions and that we actually recieved an np.ndarray
    X = (PyArrayObject*)PyArray_FROMANY(
        Xobj,
        NPY_DOUBLE,
        2,
        2,
        NPY_ARRAY_CARRAY_RO
    );

    if (X == NULL)
    {
        return NULL;
    }

    npy_intp *dims = PyArray_SHAPE(X);

    npy_intp n = dims[0];
    npy_intp dim = dims[1];

    if (dim < DBSCAN_MIN_DIMS)
    {
        PyErr_SetString(PyExc_ValueError, "DBSCAN: invalid input data dimensionality (has to >=" Py_STRINGIFY(DBSCAN_MIN_DIMS) ")");
        return NULL;
    }

    if (dim > DBSCAN_MAX_DIMS)
    {
        PyErr_SetString(PyExc_ValueError, "DBSCAN: dimension >" Py_STRINGIFY(DBSCAN_MAX_DIMS) " is not supported");
        return NULL;
    }

    if (n > 100000000)
    {
        PyErr_WarnEx(PyExc_RuntimeWarning, "DBSCAN: large n, the program behavior might be undefined due to overflow", 1);
    }

    PyArrayObject* core_samples = (PyArrayObject*)PyArray_SimpleNew(1, &n, NPY_BOOL);
    PyArrayObject* labels = (PyArrayObject*)PyArray_SimpleNew(1, &n, NPY_INT);

    DBSCAN(
        dim,
        n,
        (double*)PyArray_DATA(X),
        eps,
        min_samples,
        (bool*)PyArray_DATA(core_samples),
        (int*)PyArray_DATA(labels)
    );

    return PyTuple_Pack(2, labels, core_samples);
}

PyDoc_STRVAR(doc_DBSCAN,
"DBSCAN(X, eps=0.5, min_samples=5)\n--\n\n\
Run DBSCAN on a set of n samples of dimension dim with a minimum seperation\n\
between the clusters (which must include at least min_samples) of eps. Points\n\
that do not fit in any cluster are labeled as noise (-1).\n\
\n\
This function returns a tuple consisting of an int array of length n containing\n\
the labels after clustering and a bool array of length n which differentiates\n\
whether or not the sample is the core sample of its cluster.\n\
\n\
Parameters\n\
----------\n\
X : np.ndarray[tuple[n, dim], np.float64]\n\
    2-D array representing the samples.\n\
eps : float\n\
    minimum seperation between the clusters.\n\
min_samples : int\n\
    minimum number of samples in the clusters.\n\
\n\
Returns\n\
-------\n\
labels : np.ndarray[tuple[n], np.int_]\n\
    the labels after clustering\n\
core_samples : np.ndarray[tuple[n], np.bool_]\n\
    is each sample the core sample of its cluster\n\
\n");

static struct PyMethodDef methods[] = {
    {"DBSCAN", (PyCFunction)(void*)(PyCFunctionWithKeywords) DBSCAN_py, METH_VARARGS | METH_KEYWORDS, doc_DBSCAN},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef dbscanModule =
{
    PyModuleDef_HEAD_INIT,
    "_dbscan",
    "",
    0,
    methods
};

PyMODINIT_FUNC
PyInit__dbscan(void)
{
    import_array();
    PyObject *module = PyModule_Create(&dbscanModule);
#ifdef DBSCAN_VERSION
    PyModule_AddStringConstant(module, "__version__", DBSCAN_VERSION);
#endif
    PyModule_AddIntMacro(module, DBSCAN_MIN_DIMS);
    PyModule_AddIntMacro(module, DBSCAN_MAX_DIMS);

    return module;
}
