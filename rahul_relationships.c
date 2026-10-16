#include "stocklink.h"
#include <string.h>

void rel_init(ProductGraph *graph)
{
    if (graph) memset(graph, 0, sizeof(*graph));
}

static int vertex_index(ProductGraph *graph, int id, int create)
{
    int i;
    for (i = 0; i < graph->vertex_count; ++i)
        if (graph->product_ids[i] == id) return i;
    if (!create || graph->vertex_count >= MAX_VERTICES || id <= 0) return -1;
    graph->product_ids[graph->vertex_count] = id;
    graph->degree[graph->vertex_count] = 0;
    return graph->vertex_count++;
}

static int add_or_update_edge(ProductGraph *graph, int from, int to, int strength)
{
    int i;
    for (i = 0; i < graph->degree[from]; ++i) {
        if (graph->edges[from][i].product_id == graph->product_ids[to]) {
            graph->edges[from][i].strength = strength;
            return 1;
        }
    }
    if (graph->degree[from] >= MAX_DEGREE) return 0;
    graph->edges[from][graph->degree[from]].product_id = graph->product_ids[to];
    graph->edges[from][graph->degree[from]].strength = strength;
    ++graph->degree[from];
    return 1;
}

int rel_add_link(ProductGraph *graph, int product_a, int product_b, int strength)
{
    int a, b;
    if (!graph || product_a <= 0 || product_b <= 0 || product_a == product_b ||
        strength < 1 || strength > 10) return 0;
    a = vertex_index(graph, product_a, 1);
    b = vertex_index(graph, product_b, 1);
    if (a < 0 || b < 0) return 0;
    {
        int i, edge_ab = 0, edge_ba = 0;
        for (i = 0; i < graph->degree[a]; ++i)
            if (graph->edges[a][i].product_id == product_b) edge_ab = 1;
        for (i = 0; i < graph->degree[b]; ++i)
            if (graph->edges[b][i].product_id == product_a) edge_ba = 1;
        /* Check both directions first so a full vertex cannot leave a half-edge. */
        if ((!edge_ab && graph->degree[a] >= MAX_DEGREE) ||
            (!edge_ba && graph->degree[b] >= MAX_DEGREE)) return 0;
    }
    if (!add_or_update_edge(graph, a, b, strength)) return 0;
    if (!add_or_update_edge(graph, b, a, strength)) return 0;
    return 1;
}

int rel_recommend(const ProductGraph *graph, int start_product_id,
                  int max_depth, int out_ids[], int capacity)
{
    int queue[MAX_VERTICES], depth[MAX_VERTICES], visited[MAX_VERTICES] = {0};
    int start = -1, head = 0, tail = 0, count = 0, i;
    if (!graph || !out_ids || capacity <= 0 || max_depth < 1) return 0;
    for (i = 0; i < graph->vertex_count; ++i)
        if (graph->product_ids[i] == start_product_id) { start = i; break; }
    if (start < 0) return 0;
    queue[tail] = start;
    depth[tail++] = 0;
    visited[start] = 1;
    while (head < tail) {
        int current = queue[head];
        int current_depth = depth[head++];
        int edge;
        if (current_depth >= max_depth) continue;
        for (edge = 0; edge < graph->degree[current]; ++edge) {
            int target_id = graph->edges[current][edge].product_id;
            int target = -1, j;
            for (j = 0; j < graph->vertex_count; ++j)
                if (graph->product_ids[j] == target_id) { target = j; break; }
            if (target < 0 || visited[target]) continue;
            visited[target] = 1;
            if (target_id != start_product_id && count < capacity) out_ids[count++] = target_id;
            if (tail < MAX_VERTICES) {
                queue[tail] = target;
                depth[tail++] = current_depth + 1;
            }
        }
    }
    return count;
}

void rel_seed_demo(ProductGraph *graph)
{
    rel_add_link(graph, 101, 102, 8);
    rel_add_link(graph, 101, 103, 9);
    rel_add_link(graph, 102, 106, 6);
    rel_add_link(graph, 104, 107, 9);
    rel_add_link(graph, 103, 108, 5);
    rel_add_link(graph, 105, 106, 4);
}

/* DFS returns the other products in the same connected relationship group. */
int rel_bundle(const ProductGraph *graph, int start_product_id,
               int out_ids[], int capacity)
{
    int stack[MAX_VERTICES], visited[MAX_VERTICES] = {0};
    int start = -1, top = 0, count = 0, i;
    if (!graph || !out_ids || capacity <= 0) return 0;
    for (i = 0; i < graph->vertex_count; ++i)
        if (graph->product_ids[i] == start_product_id) { start = i; break; }
    if (start < 0) return 0;
    stack[top++] = start;
    visited[start] = 1;
    while (top > 0) {
        int current = stack[--top];
        int edge;
        for (edge = 0; edge < graph->degree[current]; ++edge) {
            int target_id = graph->edges[current][edge].product_id;
            int target = -1, j;
            for (j = 0; j < graph->vertex_count; ++j)
                if (graph->product_ids[j] == target_id) { target = j; break; }
            if (target < 0 || visited[target]) continue;
            visited[target] = 1;
            if (target_id != start_product_id && count < capacity) out_ids[count++] = target_id;
            if (top < MAX_VERTICES) stack[top++] = target;
        }
    }
    return count;
}
