#include <stdio.h>
#include <stdlib.h>

#define INF 1000000000
#define NUM_NODES 3000

typedef struct Edge {
    int to;
    int weight;
    struct Edge *next;
} Edge;

typedef struct {
    int node;
    int dist;
} HeapNode;

typedef struct {
    HeapNode *data;
    int size;
    int capacity;
} MinHeap;

static MinHeap* create_heap(int capacity) {
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    h->data = (HeapNode *)malloc(sizeof(HeapNode) * capacity);
    h->size = 0;
    h->capacity = capacity;
    return h;
}

static void heap_push(MinHeap *h, int node, int dist) {
    int i = h->size++;
    h->data[i].node = node;
    h->data[i].dist = dist;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->data[parent].dist <= h->data[i].dist) break;
        HeapNode tmp = h->data[parent];
        h->data[parent] = h->data[i];
        h->data[i] = tmp;
        i = parent;
    }
}

static HeapNode heap_pop(MinHeap *h) {
    HeapNode top = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (2 * i + 1 < h->size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = left;
        if (right < h->size && h->data[right].dist < h->data[left].dist) {
            smallest = right;
        }
        if (h->data[i].dist <= h->data[smallest].dist) break;
        HeapNode tmp = h->data[i];
        h->data[i] = h->data[smallest];
        h->data[smallest] = tmp;
        i = smallest;
    }
    return top;
}

static void add_edge(Edge **adj, int u, int v, int w) {
    Edge *e = (Edge *)malloc(sizeof(Edge));
    e->to = v;
    e->weight = w;
    e->next = adj[u];
    adj[u] = e;
}

int main(void) {
    Edge **adj = (Edge **)calloc(NUM_NODES, sizeof(Edge *));
    
    /* Construct reproducible pseudo-random graph */
    unsigned int seed = 42;
    for (int i = 0; i < NUM_NODES; i++) {
        for (int k = 0; k < 12; k++) {
            seed = seed * 1103515245 + 12345;
            int target = (int)(seed % NUM_NODES);
            seed = seed * 1103515245 + 12345;
            int weight = (int)(seed % 100) + 1;
            if (target != i) {
                add_edge(adj, i, target, weight);
            }
        }
    }

    int *dist = (int *)malloc(sizeof(int) * NUM_NODES);
    MinHeap *heap = create_heap(NUM_NODES * 15);
    long long total_dist_sum = 0;

    /* Run Dijkstra from 8 different sources */
    int num_sources = 8;
    for (int src_idx = 0; src_idx < num_sources; src_idx++) {
        int src = src_idx * (NUM_NODES / num_sources);
        for (int i = 0; i < NUM_NODES; i++) dist[i] = INF;
        dist[src] = 0;
        heap->size = 0;
        heap_push(heap, src, 0);

        while (heap->size > 0) {
            HeapNode top = heap_pop(heap);
            int u = top.node;
            int d = top.dist;
            if (d > dist[u]) continue;

            for (Edge *e = adj[u]; e != NULL; e = e->next) {
                int v = e->to;
                int w = e->weight;
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    heap_push(heap, v, dist[v]);
                }
            }
        }

        for (int i = 0; i < NUM_NODES; i++) {
            if (dist[i] < INF) total_dist_sum += dist[i];
        }
    }

    printf("Dijkstra: Nodes=%d Sources=%d Checksum=%lld\n", NUM_NODES, num_sources, total_dist_sum);

    /* Cleanup */
    for (int i = 0; i < NUM_NODES; i++) {
        Edge *curr = adj[i];
        while (curr) {
            Edge *next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(adj);
    free(dist);
    free(heap->data);
    free(heap);
    return 0;
}
