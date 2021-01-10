#include "bfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>
#include <omp.h>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

#define ROOT_NODE_ID 0
#define NOT_VISITED_MARKER -1
#define SWITCH_RATIO 0.0001

void vertex_set_clear(vertex_set *list) {
    list->count = 0;
}

void vertex_set_init(vertex_set *list, int count) {
    list->max_vertices = count;
    list->vertices = (int *)malloc(sizeof(int) * list->max_vertices);
    vertex_set_clear(list);
}

void bfs_init(Graph g, vertex_set *frontier, solution *sol) {
    #pragma omp parallel for
    for (int i = 0; i < g->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    frontier->vertices[0] = ROOT_NODE_ID;
    frontier->count++;
    sol->distances[ROOT_NODE_ID] = 0;
}

void frontier_swap(vertex_set **frontier, vertex_set **new_frontier) {
    vertex_set* tmp = *frontier;
    *frontier= *new_frontier;
    *new_frontier= tmp;
}

void top_down_step(
    Graph g,
    vertex_set *frontier,
    vertex_set *new_frontier,
    int *distances) {

    #pragma omp parallel for
    for (int i = 0; i < frontier->count; i++) {

        int node = frontier->vertices[i];
        int start_edge = g->outgoing_starts[node];
        int end_edge = (node == g->num_nodes - 1)
                           ? g->num_edges
                           : g->outgoing_starts[node + 1];

        int *local_new_frontier = (int *)malloc((end_edge - start_edge) * sizeof(int));
        int local_count = 0;
        
        for (int neighbor = start_edge; neighbor < end_edge; neighbor++) {
            int outgoing = g->outgoing_edges[neighbor];
            if (distances[outgoing] != NOT_VISITED_MARKER) continue;
            if (__sync_bool_compare_and_swap(&distances[outgoing], NOT_VISITED_MARKER, distances[node] + 1)) 
                local_new_frontier[local_count++] = outgoing;
        }

        int index = __sync_fetch_and_add(&new_frontier->count, local_count);
        for (int j = 0; j < local_count; j++)
            new_frontier->vertices[index + j] = local_new_frontier[j];
            
        free(local_new_frontier);
    }
}

void bottom_up_step(
    Graph g,
    vertex_set *frontier,
    vertex_set *new_frontier,
    int *distances) {

    int *status = (int *) malloc(g->num_nodes * sizeof(int)); // record distances's old status
    #pragma omp parallel for
    for (int i = 0; i < g->num_nodes; i++)
        status[i] = distances[i];

    #pragma omp parallel for
    for (int i = 0; i < g->num_nodes; i++) {
        if (status[i] == NOT_VISITED_MARKER) {
            int start_edge = g->incoming_starts[i];
            int end_edge = (i == g->num_nodes - 1)
                            ? g->num_edges
                            : g->incoming_starts[i + 1];
            for (int neighbor = start_edge; neighbor < end_edge; neighbor++) {
                int incoming = g->incoming_edges[neighbor];
                if (status[incoming] != NOT_VISITED_MARKER) {
                    new_frontier->vertices[new_frontier->count++] = i;
                    distances[i] = distances[frontier->vertices[0]] + 1;
                    break;
                }
            }
        }
    }
    free(status);
}

void bfs_top_down(Graph graph, solution *sol) {
    vertex_set list1;
    vertex_set list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);
    vertex_set *frontier = &list1;
    vertex_set *new_frontier = &list2;
    bfs_init(graph, frontier, sol);

    while (frontier->count) {
        vertex_set_clear(new_frontier);
        top_down_step(graph, frontier, new_frontier, sol->distances);
        frontier_swap(&frontier, &new_frontier);
    }
}

void bfs_bottom_up(Graph graph, solution *sol) {
    vertex_set list1;
    vertex_set list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);
    vertex_set *frontier = &list1;
    vertex_set *new_frontier = &list2;
    bfs_init(graph, frontier, sol);

    while (frontier->count) {
        vertex_set_clear(new_frontier);
        bottom_up_step(graph, frontier, new_frontier, sol->distances);
        frontier_swap(&frontier, &new_frontier);
    }
}

void bfs_hybrid(Graph graph, solution *sol) {
    vertex_set list1;
    vertex_set list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);
    vertex_set *frontier = &list1;
    vertex_set *new_frontier = &list2;
    bfs_init(graph, frontier, sol);

    while (frontier->count) {
        vertex_set_clear(new_frontier);
        if (frontier->count > SWITCH_RATIO * graph->num_nodes)
            bottom_up_step(graph, frontier, new_frontier, sol->distances);
        else
            top_down_step(graph, frontier, new_frontier, sol->distances);
        frontier_swap(&frontier, &new_frontier);
    }
}
