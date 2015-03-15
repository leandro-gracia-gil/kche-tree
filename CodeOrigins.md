# Where this code comes from #

All the code and ideas that are part of the project are completely of public domain. As it seems that some people like to think that some parts are based on proprietary code and _stolen_ ideas, this page has been created to clearly show that it is not to anyone who cares on reading.


# Implemented ideas #

Some featured ideas of the kche-tree templates are the incremental hypersphere-hyperrectangle distance calculations, the use of data permutation vectors to make bucket data contiguous and leaf nodes quite small, or the use of an epsilon value for approximate searches. All those, and many others tested but finally not included (some may be found on the historical non-released versions) are simply already published ideas.

If you had some of those ideas and you thought they are yours only you're just wrong. And here's the proof:
  * **Incremental distance calculations**: quite described back in 1993 by Sunil Arya and David M. Mount in [Algorithms for Fast Vector Quantization](http://www.cs.umd.edu/~mount/Papers/DCC.pdf).
  * **Use of permutation vectors and two indices to encode bucket data at leaf nodes**: described back in 1990 by Jon Louis Bentley in [K-d trees for semidynamic point sets](http://portal.acm.org/citation.cfm?id=98564), a reference of the previous paper.
  * **Approximate k-nearest neighbour searches**: currently implemented by just adding a fixed distance _epsilon_ to the hyperrectangle intersections in a quite simple way. However the idea of approximate searches was already published and a lot more developed back in 1993 by Sunil Arya and David M. Mount (the authors of the first paper) in [Approximate Nearest Neighbor Queries in Fixed Dimensions](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.77.185&rep=rep1&type=pdf). In this case, they even developed an open source library for approximate k-neighbour searches: [ANN](http://www.cs.umd.edu/~mount/ANN/).
  * **Depth-first branch and bound, path ordering and others**: some of these were tested in previous non-released versions and some were just adapted as part of the template design. In any case, these and other optimizations were also published in 2001 by Neal Sample,  Matthew Haines, Mark Arnold, and Timothy Purcell in [Optimizing Search Strategies in k-d Trees](http://ilpubs.stanford.edu:8090/723/1/2001-17.pdf).

If still not clear after this, please do not hesitate to contact [me](http://code.google.com/u/leandro.gracia.gil/) at gmail.

<sub>Updated on 25th May 2011 to remove dead links to articles.</sub>