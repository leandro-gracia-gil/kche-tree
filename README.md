# Kche-tree
Kche-tree is a set of C++ templates for generic cache-aware and non-mutable kd-trees. Its main purpose is to provide an easy to use but powerful implementation of the typical kd-tree structure functionality with very low latencies.

It provides the following basic operations:
* **Build**: create a kd-tree from a set of feature vectors. Median splitting is used to keep the tree balanced. Cost: O(n log² n).
* **K nearest neighbours**: retrieve the K nearest neighbours of a given feature vector. Estimated average cost: O(log K log n).
* **All neighbours within a range**: retrieve all the neighbours inside a maximum distance radius from a given feature vector. Estimated average cost: O(log m log n) with m the number of neighbours in the range.

The template has been designed to minimize the number of cache misses combined with many algorithmic techniques and ideas. Here are some of its features:
* Can dynamically define the metrics to use when exploring the tree: Euclidean, Mahalanobis, etc.
* Automatic SSE code generation and unrolling optimized for the requested number of dimensions.
* Incremental calculation of the hyperrectangle-hypersphere intersections.
* Internal data permutation to increase cache hits.
* Contiguous bucket data to reduce the leaf node size.
* Preorder memory allocation during the build to increase cache hits when traversing.
* Exploration/intersection recursive scheme to reduce the number of calculations performed.
* Use of specific k-neighbours optimized containers: k-vectors and k-heaps.
* Distance calculations with upper bounds allowing early returns.
* Endianness-safe binary file format and stream operators provide to easily save and load the kd-trees.

For more details about use, please check the documentation included in the release.
