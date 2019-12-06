#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define INT_MAX 10000000
int n_thread;
/*Test Utils*/
void _print_student_id	();
void  print_path	(char*);
void  print_dist_time	(int, double);
/*CSV Reader*/
int  _extract_str_int	(char* str);
int **csv_to_array	(char* file_name, int* get_size);

/*Testing Array Utils*/
int **random2DArray	(int size);
int **symetricArray	(int** arr, int size);

/*Algorithm*/
int dijkstra(struct Graph* graph, int source, int target);

// A structure to represent a node in adjacency list 
struct AdjListNode 
{ 
    int dest; 
    int weight; 
    struct AdjListNode* next; 
}; 
  
// A structure to represent an adjacency list 
struct AdjList 
{
    int len; 
    struct AdjListNode* head;  // pointer to head node of list 
    struct AdjListNode** entry;
}; 
  
// A structure to represent a graph. A graph is an array of adjacency lists. 
// Size of array will be V (number of vertices in graph) 
struct Graph 
{ 
    int V; 
    struct AdjList* array; 
}; 
  
// A utility function to create a new adjacency list node 
struct AdjListNode* newAdjListNode(int dest, int weight) 
{ 
    struct AdjListNode* newNode = 
            (struct AdjListNode*) malloc(sizeof(struct AdjListNode)); 
    newNode->dest = dest; 
    newNode->weight = weight; 
    newNode->next = NULL; 
    return newNode; 
} 
  
// A utility function that creates a graph of V vertices 
struct Graph* createGraph(int V) 
{ 
    struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph)); 
    graph->V = V; 
  
    // Create an array of adjacency lists.  Size of array will be V 
    graph->array = (struct AdjList*) malloc(V * sizeof(struct AdjList)); 
  
     // Initialize each adjacency list as empty by making head as NULL 
    for (int i = 0; i < V; ++i) 
    {
        graph->array[i].entry = (struct AdjListNode**)malloc(V * sizeof(struct AdjList*));
        graph->array[i].len = 0;
        graph->array[i].head = NULL; 
    }
    return graph; 
} 
  
// Adds an edge to an undirected graph 
void addEdge(struct Graph* graph, int src, int dest, int weight) 
{ 
    // Add an edge from src to dest.  A new node is added to the adjacency 
    // list of src.  The node is added at the beginning 
    struct AdjListNode* newNode = newAdjListNode(dest, weight); 
    newNode->next = graph->array[src].head; 
    graph->array[src].head = newNode; 
    graph->array[src].entry[graph->array[src].len++] = newNode;
    // Since graph is undirected, add an edge from dest to src also 
    newNode = newAdjListNode(src, weight); 
    newNode->next = graph->array[dest].head; 
    graph->array[dest].head = newNode; 
    graph->array[dest].entry[graph->array[dest].len++] = newNode;
} 
  
// Structure to represent a min heap node 
struct MinHeapNode 
{ 
    int  v; 
    int dist; 
}; 
  
// Structure to represent a min heap 
struct MinHeap 
{ 
    int size;      // Number of heap nodes present currently 
    int capacity;  // Capacity of min heap 
    int *pos;     // This is needed for decreaseKey() 
    struct MinHeapNode **array; 
}; 
  
// A utility function to create a new Min Heap Node 
struct MinHeapNode* newMinHeapNode(int v, int dist) 
{ 
    struct MinHeapNode* minHeapNode = 
           (struct MinHeapNode*) malloc(sizeof(struct MinHeapNode)); 
    minHeapNode->v = v; 
    minHeapNode->dist = dist; 
    return minHeapNode; 
} 
  
// A utility function to create a Min Heap 
struct MinHeap* createMinHeap(int capacity) 
{ 
    struct MinHeap* minHeap = 
         (struct MinHeap*) malloc(sizeof(struct MinHeap)); 
    minHeap->pos = (int *)malloc(capacity * sizeof(int)); 
    minHeap->size = 0; 
    minHeap->capacity = capacity; 
    minHeap->array = 
         (struct MinHeapNode**) malloc(capacity * sizeof(struct MinHeapNode*)); 
    return minHeap; 
} 
  
// A utility function to swap two nodes of min heap. Needed for min heapify 
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) 
{ 
    struct MinHeapNode* t = *a; 
    *a = *b; 
    *b = t; 
} 
  
// A standard function to heapify at given idx 
// This function also updates position of nodes when they are swapped. 
// Position is needed for decreaseKey() 
void minHeapify(struct MinHeap* minHeap, int idx) 
{ 
    int smallest, left, right; 
    smallest = idx; 
    left = 2 * idx + 1; 
    right = 2 * idx + 2; 
  
    if (left < minHeap->size && 
        minHeap->array[left]->dist < minHeap->array[smallest]->dist ) 
      smallest = left; 
  
    if (right < minHeap->size && 
        minHeap->array[right]->dist < minHeap->array[smallest]->dist ) 
      smallest = right; 
  
    if (smallest != idx) 
    { 
        // The nodes to be swapped in min heap 
        MinHeapNode *smallestNode = minHeap->array[smallest]; 
        MinHeapNode *idxNode = minHeap->array[idx]; 
  
        // Swap positions 
        minHeap->pos[smallestNode->v] = idx; 
        minHeap->pos[idxNode->v] = smallest; 
  
        // Swap nodes 
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]); 
  
        minHeapify(minHeap, smallest); 
    } 
} 
  
// A utility function to check if the given minHeap is ampty or not 
int isEmpty(struct MinHeap* minHeap) 
{ 
    return minHeap->size == 0; 
} 
  
// Standard function to extract minimum node from heap 
struct MinHeapNode* popMin(struct MinHeap* minHeap) 
{ 
    if (isEmpty(minHeap)) 
        return NULL; 
  
    // Store the root node 
    struct MinHeapNode* root = minHeap->array[0]; 
  
    // Replace root node with last node 
    struct MinHeapNode* lastNode = minHeap->array[minHeap->size - 1]; 
    minHeap->array[0] = lastNode; 
  
    // Update position of last node 
    minHeap->pos[root->v] = minHeap->size-1; 
    minHeap->pos[lastNode->v] = 0; 
  
    // Reduce heap size and heapify root 
    --minHeap->size; 
    minHeapify(minHeap, 0); 
  
    return root; 
} 
  
// Function to decreasy dist value of a given vertex v. This function 
// uses pos[] of min heap to get the current index of node in min heap 
void decreaseKey(struct MinHeap* minHeap, int v, int dist) 
{ 
    // Get the index of v in  heap array 
    int i = minHeap->pos[v]; 
  
    // Get the node and update its dist value 
    minHeap->array[i]->dist = dist; 
  
    // Travel up while the complete tree is not hepified. 
    // This is a O(Logn) loop 
    while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist) 
    { 
        // Swap this node with its parent 
        minHeap->pos[minHeap->array[i]->v] = (i-1)/2; 
        minHeap->pos[minHeap->array[(i-1)/2]->v] = i; 
        swapMinHeapNode(&minHeap->array[i],  &minHeap->array[(i - 1) / 2]); 
  
        // move to parent index 
        i = (i - 1) / 2; 
    } 
} 
  
// A utility function to check if a given vertex 
// 'v' is in min heap or not 
bool isInMinHeap(struct MinHeap *minHeap, int v) 
{ 
   if (minHeap->pos[v] < minHeap->size) 
     return true; 
   return false; 
} 
  
// A utility function used to print the solution 
void printArr(int dist[], int n) 
{ 
    printf("Vertex   Distance from Source\n"); 
    for (int i = 0; i < n; ++i) 
        printf("%d \t\t %d\n", i, dist[i]); 
} 

void printConnections(Graph* graph)
{
    int v = graph->V;
    int i = 0;
    while(i < v)
    {
    	printf("vertex %d has %d neighbors.\n", i, graph->array[i].len);
	i++;
    }
}
  
// The main function that calulates distances of shortest paths from src to all 
// vertices. It is a O(ELogV) function 
int dijkstra(struct Graph* graph, int src, int target) 
{ 
    int V = graph->V;// Get the number of vertices in graph 
    int* dist = (int*)malloc(sizeof(int)*V);
    struct MinHeap* minHeap = createMinHeap(V); 
  
    #pragma omp parallel for
    for (int v = 0; v < V; ++v) 
    { 
        dist[v] = INT_MAX; 
        minHeap->array[v] = newMinHeapNode(v, dist[v]); 
        minHeap->pos[v] = v; 
    } 
  
    minHeap->array[src] = newMinHeapNode(src, dist[src]); 
    minHeap->pos[src]   = src; 
    dist[src] = 0; 
    decreaseKey(minHeap, src, dist[src]); 
  
    minHeap->size = V; 
    double wtime = omp_get_wtime();
    while (!isEmpty(minHeap)) 
    { 
        struct MinHeapNode* minHeapNode = popMin(minHeap); 
        int u = minHeapNode->v; // Store the extracted vertex number 
	//printf(" n%04d ", u);
	if(u == target)
		break;
	#pragma omp parallel for
	for(int i = 0; i < graph->array[u].len; i++)
        { 
            struct AdjListNode* pCrawl = graph->array[u].entry[i];
	    int v = pCrawl->dest; 
  
            if (isInMinHeap(minHeap, v) && dist[u] != INT_MAX &&  
                                          pCrawl->weight + dist[u] < dist[v]) 
            { 
            	dist[v] = dist[u] + pCrawl->weight; 
            	#pragma omp critical
            	{
                	decreaseKey(minHeap, v, dist[v]);	
            	}
	    } 
        } 
    } 
    return dist[target];
}

struct Graph* arrayToGraph(int** weight, int size);
  
/*main*/
//./serial mapDist1600.csv 4 0 200 1300
int main(int argc, char* argv[])
{
	char *file_name = argv[1];
	n_thread = atoi(argv[2]);
	int path_true = atoi(argv[3]);
	int src = atoi(argv[4]);
	int target = atoi(argv[5]);

	int size = 0;
	int **cost = csv_to_array(file_name, &size);
	struct Graph* graph = arrayToGraph(cost, size);
	omp_set_num_threads(n_thread);

	double time = omp_get_wtime();
	int dist = dijkstra(graph, src, target);
	time = omp_get_wtime() - time;

	print_dist_time(dist, time);
	free(cost);
	return 0;
}
/*Test Utils*/
void _print_student_id()
{
	printf("201511298 ");
}

void print_path(char* path)
{
	_print_student_id();
	printf("%s\n", path);	
}

void print_dist_time(int dist, double wtime)
{
	_print_student_id();
	printf("Shortest Path: %d Compute time: %lf msec\n", dist, wtime);
}

/*CSV Reader Impl.*/
int _extract_str_int(char* str)
{
	int i = 0, start = 0, end = strlen(str) - 1;
	char* copy, c;

	while((c = str[i])!='\0')
	{
		if(isdigit(c) && start == 0)
			start = i;
		if(c == '.')
			end = i;
		i++;
	}
	
	int store = str[end];
	str[end] = '\0';
	copy = str + start;

	int rtn = atoi(copy);
	str[end] = store;
	return rtn;
}

int** csv_to_array(char* file_name, int* get_size)
{
	FILE* csv = fopen(file_name, "r");
	int size = _extract_str_int(file_name);
	int** data = (int**)malloc(size*sizeof(int*));
    
	if(!csv) {
		printf("%s is not a file.\n", file_name);    
		exit(-1);
	}
    
	for(int i = 0; i < size; i++)
		data[i] = (int*)malloc(size*sizeof(int));
    
	char* row = (char*)malloc(size*10);
	int r = 0, c;
	while(fgets(row, size*10, csv))
	{
		if(r != 0)
		{
			char* col = strtok(row, ",\n");
			c = 0;
			while(col != NULL)
			{
				if(c != 0)
				{
					if(strcmp(col, "MAX") == 0)
						data[r-1][c-1] = INT_MAX;
					else
						data[r-1][c-1] = atoi(col);
				}
				col = strtok(NULL, ",\n");
				c++;
			}
		}
		r++;
	}
    	*get_size = size;
	return data;
}

/*Testing Array Utils Impl.*/
int **random2DArray(int size)
{
	int** array = (int**)malloc(size*sizeof(int*));
	#pragma omp parallel for
	for(int i = 0; i < size; i++)
	{
		array[i] = (int*)malloc(size*sizeof(int));
		for(int j = 0; j < size; j++)
		{
			if(i == j)
				array[i][j] = 0;
			else
				array[i][j] = i + size - j;
		}
	}
	return array;
}

int **symetricArray	(int** arr, int size)
{
	for(int i = 0; i < size; i++) {
		for(int j = i; j < size; j++) {
			arr[j][i] = arr[i][j];
		}
	}
	return arr;
}

/*Graph Generator Impl.*/
struct Graph* arrayToGraph(int** weight, int size)
{
	struct Graph* graph = createGraph(size);
	for(int i = 0; i < size - 1; i++)
		for(int j = i + 1; j < size; j++) 
			if(weight[i][j] != 0)
				addEdge(graph, i, j, weight[i][j]);
	return graph;
}
