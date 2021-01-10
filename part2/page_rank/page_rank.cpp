#include "page_rank.h"

#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double *solution, double damping, double convergence) {
	int numNodes = num_nodes(g);
	bool converged = false;
	double equal_prob = 1.0 / numNodes;
	double *score_new = (double*)malloc(numNodes * sizeof(double));
	
	#pragma omp parallel for
  	for (int i = 0; i < numNodes; ++i)
    	solution[i] = equal_prob;  // initialization

	while (!converged) {
		double score_old = 0;
		double global_diff = 0;
		#pragma omp parallel for reduction(+:score_old)
		for (int vi = 0; vi < numNodes; ++vi) {  // comput score_old[v]
			score_new[vi] = 0;  // initialize for new round
			if (!outgoing_size(g, vi))  // nodes v in graph with no outgoing edges
				score_old += solution[vi];
		}
		#pragma omp parallel for
		for (int vi = 0; vi < numNodes; ++vi) {  // compute score_new[vi] for all nodes vi
			const int *begin = incoming_begin(g, vi);
			const int *end = incoming_end(g, vi);
			const int *node = begin;
			for (const int *node = begin; node != end; node++) {  // sum over all nodes vj reachable from incoming edges
				int num = outgoing_size(g, *node);
				score_new[vi] += solution[*node] / num;  // score_old[vj] / number of edges leaving vj
			}
			score_new[vi] = (damping * score_new[vi]) + (1.0 - damping) / numNodes;
			score_new[vi] += damping * score_old / numNodes;
		}
		#pragma omp parallel for reduction(+:global_diff)
		for (int vi = 0; vi < numNodes; ++vi) {
			global_diff += abs(score_new[vi] - solution[vi]);
			solution[vi] = score_new[vi];
		}
		converged = global_diff < convergence;
	}
	free(score_new);
}
