# Parallelizing PageRank Algorithm with OpenMP
In this part, you will implement two graph processing algorithms: breadth-first search (BFS) and a simple implementation of page rank. A good implementation of this assignment will be able to run these algorithms on graphs containing hundreds of millions of edges on a multi-core machine in only seconds.

## 2.1 Background: Representing Graphs
The starter code operates on directed graphs, whose implementation you can find in common/graph.h and common/graph_internal.h. We recommend you begin by understanding the graph representation in these files. A graph is represented by an array of edges (both outgoing_edges and incoming_edges), where each edge is represented by an integer describing the id of the destination vertex. Edges are stored in the graph sorted by their source vertex, so the source vertex is implicit in the representation. This makes for a compact representation of the graph, and also allows it to be stored contiguously in memory. For example, to iterate over the outgoing edges for all nodes in the graph, you’d use the following code which makes use of convenient helper functions defined in common/graph.h (and implemented in common/graph_internal.h):

```
for (int i=0; i<num_nodes(g); i++) {
    // Vertex is typedef'ed to an int. Vertex* points into g.outgoing_edges[]
    const Vertex* start = outgoing_begin(g, i);
    const Vertex* end = outgoing_end(g, i);
    for (const Vertex* v=start; v!=end; v++)
        printf("Edge %u %u\n", i, *v);
}
```

## 2.2 Task 1: Implementing Page Rank
As a simple warm up exercise to get comfortable using the graph data structures, and to get acquainted with a few OpenMP basics, we’d like you to begin by implementing a basic version of the well-known page rank algorithm.

Please take a look at the pseudocode provided to you in the function pageRank(), in the file page_rank/page_rank.cpp. You should implement the function, parallelizing the code with OpenMP. Just like any other algorithm, first identify independent work and any necessary sychronization.

## 2.3 Task 2: Parallel Breadth-First Search (“Top Down”)
Breadth-first search (BFS) is a common algorithm that you’ve almost certainly seen in a prior algorithms class.

Please familiarize yourself with the function bfs_top_down() in breadth_first_search/bfs.cpp, which contains a sequential implementation of BFS. The code uses BFS to compute the distance to vertex 0 for all vertices in the graph. You may wish to familiarize yourself with the graph structure defined in common/graph.h as well as the simple array data structure vertex_set (breadth_first_search/bfs.h), which is an array of vertices used to represent the current frontier of BFS.

## 2.4 Task 3: “Bottom-Up” BFS
Think about what behavior might cause a performance problem in the BFS implementation from Part 2.3. An alternative implementation of a breadth-first search step may be more efficient in these situations. Instead of iterating over all vertices in the frontier and marking all vertices adjacent to the frontier, it is possible to implement BFS by having each vertex check whether it should be added to the frontier! Basic pseudocode for the algorithm is as follows:

```
for(each vertex v in graph)
    if(v has not been visited && 
       v shares an incoming edge with a vertex u on the frontier)
            add vertex v to frontier;
```

This algorithm is sometimes referred to as a “bottom-up” implementation of BFS, since each vertex looks “up the BFS tree” to find its ancestor. (As opposed to being found by its ancestor in a “top-down” fashion, as was done in Part 2.3.)

Please implement a bottom-up BFS to compute the shortest path to all the vertices in the graph from the root (see bfs_bottom_up() in breadth_first_search/bfs.cpp). Start by implementing a simple sequential version. Then parallelize your implementation.

## 2.5 Task 4: Hybrid BFS
Notice that in some steps of the BFS, the “bottom-up” BFS is significantly faster than the top-down version. In other steps, the top-down version is significantly faster. This suggests a major performance improvement in your implementation, if you could dynamically choose between your “top-down” and “bottom-up” formulations based on the size of the frontier or other properties of the graph! If you want a solution competitive with the reference one, your implementation will likely have to implement this dynamic optimization. Please provide your solution in bfs_hybrid() in breadth_first_search/bfs.cpp.