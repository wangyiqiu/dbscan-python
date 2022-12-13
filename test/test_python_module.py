"""
This file is almost identical to "example.py", except that it compares it
to the correct answer to create a test case.
"""

import numpy as np

from sklearn.datasets import make_blobs
from sklearn.preprocessing import StandardScaler

from sklearn.cluster import DBSCAN as goldDBSCAN
from dbscan import sklDBSCAN as ourDBSCAN

import unittest

class ExampleTest(unittest.TestCase):

    def verify_output(self, X):
        # ######################################################################
        # Compute DBSCAN

        labels = goldDBSCAN(eps=0.3, min_samples=10).fit(X).labels_
        true_labels = ourDBSCAN(eps=0.3, min_samples=10).fit(X).labels_

        # ######################################################################
        # Test case

        mapping = {-1: -1}

        for i, (label, true_label) in enumerate(zip(labels, true_labels)):
            if true_label in mapping:
                self.assertTrue(
                    mapping[true_label] == label,
                    "Contradiction at slot {}. mapping={}. true={}. ours={}." \
                        .format(i, mapping, true_label, label)
                )
            else:
                mapping[true_label] = label

    def test_example(self):
        # ######################################################################
        # Generate sample data
        centers = [[1, 1], [-1, -1], [1, -1]]
        X, labels_true = make_blobs(n_samples=750, centers=centers,
                                    cluster_std=0.4, random_state=0)
        X = StandardScaler().fit_transform(X)
        self.verify_output(X)
