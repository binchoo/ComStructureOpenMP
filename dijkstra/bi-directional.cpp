#include <iostream>
#include <queue>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*Config Variables*/
#define INT_MAX 10000000
#define DO_PARALLEL 14999
#define chunk_size 13
int   n_thread;
int   path_true;
FILE  *fout;

/*Print Utils*/
void  print_param	(FILE* fp, char** argv);
void _print_student_id	(FILE* fp);
void  print_path	(FILE* fp, int* updater_of, int target, int size);
void  print_dist_time	(FILE* fp, int dist, double time);

/*CSV Reader*/
int  _extract_str_int	(char* str);
int **csv_to_array	(char* file_name, int* get_size);

/*Array Generation Utils*/
int **random_2d_array	(int size);
int **symmetric_2d_array(int** arr, int size);

using namespace std;
/*Heap Node*/
struct Node {
	int id;
	int dist;
	Node(int id, int dist) : id(id), dist(dist) { }
};

/*Heap Comparator*/
struct cmp {
	bool operator()(Node& u, Node& v) {
		return u.dist > v.dist;
	}
};

/*Dijkstra Minimum Distance*/
/*여러 번 호출되기 때문에 inline으로 선언*/
inline 
int arg_min(int* dist, int* graphed, int size)
{
	register int i;
	register int min = INT_MAX;
	register int arg_min;
	for(i = 0; i < size; i++) {
		if(!graphed[i] && dist[i] < min) {
			min = dist[i];
			arg_min = i;
		}
	} return arg_min;
}

inline 
int arg_min_parallel(int* dist, int* graphed, int size)
{
	static int* local_arg_min = (int*)malloc(sizeof(int) * n_thread);
	register int glob_min = INT_MAX, glob_arg_min;
	register int my_arg;
	register int i;
	
	#pragma omp parallel
	{
		register int my_id = omp_get_thread_num();
		register int local_min = INT_MAX;
		local_arg_min[my_id] = -1;
		#pragma omp for schedule(static, chunk_size)
		for (i = 0; i < size; i++) {
			if (!graphed[i] && dist[i] < local_min) {
				local_min = dist[i];
				local_arg_min[my_id] = i;
			}
		}
	} for (i = 0; i < n_thread; i++) {
		my_arg = local_arg_min[i];
		if (my_arg != -1 && dist[my_arg] < glob_min) {
			glob_min = dist[my_arg];
			glob_arg_min = my_arg;
		}
	} return glob_arg_min;
}

/*Dijkstra 알고리즘*/
inline 
int dijkstra(int** cost, int size, int source, int target)
{
	int dist[size], updater_of[size]; //거리, 갱신자
	int graphed[size] = { 0 }; //그래프 포함여부
	register int i, start, d; //반복문 탐색자, 방금 그래프에 포함된 노드, 새로운 거리

	for(i = 0; i < size; i++) { 
		dist[i] = INT_MAX;
	} 
	dist[source] = 0, updater_of[source] = -1; 

	priority_queue<Node, vector<Node>, cmp> dist_q; 
	dist_q.push(Node(source, 0));
	while (!graphed[target]) {
		start = dist_q.top().id;
		dist_q.pop();
		graphed[start] = 1;
		for (i = 0; i < size; i++) {
			if (cost[start][i] && !graphed[i]) {
				d = dist[start] + cost[start][i];
				if (d < dist[i]) {
					dist_q.push(Node(i, d)); // 이 부분이 임계영역이라 병렬처리 하면 병목이 됨.
					dist[i] = d;
					updater_of[i] = start;
				}
			}
		}
	} if (path_true) {
		print_path(fout, updater_of, target, size);
	} return dist[target];
}

int compact_bi_dijkstra(int** cost, int size, int source, int target)
{
	int f_dist[size], f_updater_of[size]; //거리, 갱신자
	int b_dist[size], b_updater_of[size]; //거리, 갱신자
	int f_graphed[size] = { 0 }; //그래프 포함여부
	int b_graphed[size] = { 0 }; //그래프 포함여부
	register int i, f_start, b_start, d; //반복문 탐색자, 방금 그래프에 포함된 노드, 새로운 거리
	for(i = 0; i < size; i++) { 
		f_dist[i] = INT_MAX;
		b_dist[i] = INT_MAX;
	} 
	f_dist[source] = 0, f_updater_of[source] = -1; 
	b_dist[target] = 0, b_updater_of[target] = -1; 

	priority_queue<Node, vector<Node>, cmp> f_dist_q; 
	f_dist_q.push(Node(source, 0));
	priority_queue<Node, vector<Node>, cmp> b_dist_q; 
	b_dist_q.push(Node(target, 0));
	int x;
	while(1) {
		#pragma omp parallel sections private(i, d)
		{
		#pragma omp section
		{
			f_start = f_dist_q.top().id;
			f_graphed[f_start] = 1; 
			f_dist_q.pop();
			for (i = 0; i < size; i++) {
				if (cost[f_start][i] && !f_graphed[i]) {
					d = f_dist[f_start] + cost[f_start][i];
					if (d < f_dist[i]) {
						f_dist_q.push(Node(i, d));
						f_dist[i] = d;
						f_updater_of[i] = f_start;
					}
				}
			}
		
		}
		#pragma omp section
		{
			b_start = b_dist_q.top().id;
			b_graphed[b_start] = 1;
			b_dist_q.pop();
			for (i = 0; i < size; i++) {
				if (cost[b_start][i] && !b_graphed[i]) {
					d = b_dist[b_start] + cost[b_start][i];
					if (d < b_dist[i]) {
						b_dist_q.push(Node(i, d)); 
						b_dist[i] = d;
						b_updater_of[i] = b_start;
					}
				}
			}
		}//end section
		}//end sections
		if (b_graphed[f_start]) {
			x = f_start;
			break;
		} else if (f_graphed[b_start]) {
			x = b_start;
			break;
		}
	}
	return f_dist[x] + b_dist[x];

}

int bi_dijkstra(int** cost, int size, int source, int target)
{
	register int test, shortest = INT_MAX;
	register int x, f_dist, b_dist;
	int min_x;
	for (x = 0; x < size; x++) {
		#pragma omp parallel sections
		{
			#pragma omp section
			f_dist = dijkstra(cost, size, source, x);
			#pragma omp section
			b_dist = dijkstra(cost, size, target, x);

		} if ((test = f_dist + b_dist) < shortest) {
			shortest = test;
			min_x = x;
		}
	}
	printf("middle vertex is %d!\n", min_x);
	return shortest;
}

/*main*/
//mapDist1600.csv 4 0 200 1300
int main(int argc, char* argv[])
{
	fout = fopen("201511298 dijkstra.txt", "a");
	char* file_name = argv[1];
	n_thread = atoi(argv[2]);
	path_true = atoi(argv[3]);
	int src = atoi(argv[4]);
	int target = atoi(argv[5]);
	
	int size = 0; //그래프 노드의 수
	int **cost = csv_to_array(file_name, &size); //가중치 행렬 얻기

	omp_set_num_threads(n_thread); //스레드 수 설정
	
	print_param(fout, argv); //param 출력
	double time = omp_get_wtime(); 
	int dist = compact_bi_dijkstra(cost, size, src, target);
	time = omp_get_wtime() - time;

	print_dist_time(fout, dist, time); //거리와 실행시간 출력
	free(cost);
	fclose(fout);
	return 0;
}

/*Print Utils Impl.*/
void print_param(FILE* fp, char** argv)
{
	printf("param: %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
	fprintf(fp, "param: %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
}

void _print_student_id(FILE* fp)
{
	printf("201511298 ");
	fprintf(fp, "201511298 ");
}


void print_path(FILE* fp, int* updater_of, int target, int size)
{
	int buffer[size];
	register int buf_len = 0;

	do {
		buffer[buf_len++] = target;
	} while ((target = updater_of[target]) != -1);

	_print_student_id(fp);

	for(register int i = buf_len - 1; i >= 0; i--) {
		printf("n%04d ", buffer[i]);
		fprintf(fp, "n%04d ", buffer[i]);
	} 
	printf("\n");
	fprintf(fp, "\n");
}


void print_dist_time(FILE* fp, int dist, double wtime)
{
	_print_student_id(fp);
	printf("Shortest Path: %d Compute time: %.5lf msec\n", dist, wtime * 1000);
	fprintf(fp, "Shortest Path: %d Compute time: %.5lf msec\n", dist, wtime * 1000);
}

/*CSV Reader Impl.*/
int _extract_str_int(char* str)
{
	int i = 0, start = 0, end = strlen(str) - 1;
	char c;

	while((c = str[i]) != '\0') {
		if(isdigit(c) && start == 0) {
			start = i;
		} if(c == '.') {
			end = i;
		} i++;
	}
	
	int store = str[end];
	str[end] = '\0';
	str = str + start;
	
	int rtn = atoi(str);
	str = str - start;
	str[end] = store;
	return rtn;
}

int **csv_to_array(char* file_name, int* get_size)
{
	FILE* csv = fopen(file_name, "r");
	int size = _extract_str_int(file_name);
	int** data = (int**)malloc(size*sizeof(int*));
    
	if (!csv) {
		printf("%s is not a file.\n", file_name);    
		exit(-1);
	} for (int i = 0; i < size; i++) {
		data[i] = (int*)malloc(size*sizeof(int));
    	}

	char* row = (char*)malloc(size*10);
	int r = 0, c;
	while (fgets(row, size*10, csv)) {
		if (r != 0) {
			c = 0;
			char* col = strtok(row, ",\n");
			while (col != NULL) {
				if(c != 0) {
					if(strcmp(col, "MAX") == 0) {
						data[r-1][c-1] = 0;
					} else {
						data[r-1][c-1] = atoi(col);
					}
				} c++, col = strtok(NULL, ",\n");
			}
		} r++;
	} 
	fclose(csv);
    	*get_size = size;
	return data;
}

/*Array Generation Utils Impl.*/
int **random_2d_array(int size)
{
	int** array = (int**)malloc(size*sizeof(int*));
	#pragma omp parallel for
	for (int i = 0; i < size; i++) {
		array[i] = (int*)malloc(size*sizeof(int));
		for (int j = 0; j < size; j++) {
			if(i == j) {
				array[i][j] = 0;
			} else {
				array[i][j] = i + size - j;
			}
		}
	} return array;
}

int **symmetric_2d_array(int** arr, int size)
{
	for(int i = 0; i < size; i++) {
		for(int j = i; j < size; j++) {
			arr[j][i] = arr[i][j];
		}
	} return arr;
}
