from ._dbscan import *

# Load version from _version.py if available
from . import _dbscan
__all__ = tuple(v for v in dir(_dbscan) if v.startswith('_'))
try:
    from ._version import version as __version__
    __all__ += ('__version__',)
except:
    pass
del _dbscan

try:
    # Create scikit-learn wrapper if possible
    import sklearn.base
    import sklearn.cluster

    class sklDBSCAN(sklearn.base.BaseEstimator):
        __slots__ = ('core_sample_indices_', 'components_', 'labels_', 'n_features_in_', 'feature_names_in_',
                     'eps', 'min_samples', 'metric', 'metric_params', 'algorithm', 'leaf_size', 'p', 'n_jobs')
        def __init__(self, eps=0.5, *, min_samples=5, metric='euclidean', metric_params=None, algorithm='auto', leaf_size=30, p=None, n_jobs=None):
            if metric != 'euclidean': raise NotImplementedError('Only euclidean metric supported')
            if algorithm not in ('auto', 'kd_tree'): raise NotImplementedError('Only kd tree is supported')
            if leaf_size != 30: raise NotImplementedError('Leaf size not supported')
            if p not in (None, 2): raise NotImplementedError('Only Euclidean distance supported')
            if n_jobs is not None: raise NotImplementedError('Parallelization is done automatically')

            self.eps = eps
            self.min_samples = min_samples
            self.metric = metric
            self.metric_params = metric_params
            self.algorithm = algorithm
            self.leaf_size = leaf_size
            self.p = p
            self.n_jobs = n_jobs
        def fit_predict(self, X, y=None, sample_weight=None):
            if sample_weight is not None: raise NotImplementedError('sample_weight not supported')
            labels, core_samples = DBSCAN(X, self.eps, min_samples=self.min_samples)
            self.core_sample_indices_ = core_sample_indices = core_samples.nonzero()[0]
            self.components_ = X[core_sample_indices]
            self.labels_ = labels
            self.n_features_in_ = X.shape[1]
            return labels
        def fit(self, X, y=None, sample_weight=None):
            self.fit_predict(X, y=y, sample_weight=sample_weight)
            return self

    sklDBSCAN.__doc__ = \
    """A faster version of DBSCAN (with more limited functionality than sklearn's
    version). sklearn's documentation is below. If you do not need 100% API
    compatibility with scikit learn, use the low-level numpy function DBSCAN.
    This class is just a Python wrapper around that function.

    """ + sklearn.cluster.DBSCAN.__doc__

    del sklearn
    __all__ += ('sklDBSCAN',)
except Exception as e:
    # scikit-learn might have not been installed correctly
    pass
