#include <queue>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*Config Variables*/
#define INT_MAX 10000000
int DO_PARALLEL;
int   N_THREAD;
int   PATH_TRUE;
FILE  *F_OUT;
/*Print Utils*/
void  print_param	(char** argv);
void _print_student_id	();
void  print_path	(int* updater_of, int target, int size); //src -> target
void  print_path_reverse(int* updater_of, int target, int size); //target -> src
void  print_dist_time	(int dist, double time);

/*CSV Reader*/
int  _extract_str_int	(char* str); //str에서 숫자를 추출
int **csv_to_array	(char* file_name, int* get_size); //csv에서 행렬을 불러옴

/*Array Generation Utils*/
int **random_2d_array	(int size);
int **symmetric_2d_array(int** arr, int size);

using namespace std;
/*Heap Node*/
struct Node {
	int id;
	int dist; //key
	Node(int id, int dist) : id(id), dist(dist) { }
};

/*Heap Comparator*/
struct cmp {
	bool operator()(Node u, Node v) {
		return u.dist > v.dist;
	}
};

int uni_dijkstra(int** cost, int size, int source, int target)
{
	int dist[size], updater_of[size], graphed[size] = {0}; //거리, 갱신자, 그래프 포함여부
	register int i, start, d; //탐색자, 그래프에 편입된 노드, 새로운 거리

	for(i = 0; i < size; i++) { 
		dist[i] = INT_MAX;
	} dist[source] = 0, updater_of[source] = -1; 

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
	}
	if (PATH_TRUE) {
		print_path(updater_of, target, size);
	} return dist[target];
}

int bidirect_dijkstra(int** cost, int size, int source, int target)
{
	int f_dist[size], f_updater_of[size], f_graphed[size] = {0}; //거리, 갱신자
	int b_dist[size], b_updater_of[size], b_graphed[size] = {0}; //거리, 갱신자

	register int i, d, x; //반복문 탐색자, 거리, 중간 버텍스
	register int f_start, b_start; //반복문 탐색자, 방금 그래프에 포함된 노드, 새로운 거리

	for(i = 0; i < size; i++) { 
		f_dist[i] = INT_MAX;
		b_dist[i] = INT_MAX;
	} 

	priority_queue<Node, vector<Node>, cmp> f_dist_q; 
	f_dist_q.push(Node(source, 0));
	f_dist[source]= 0; 

	priority_queue<Node, vector<Node>, cmp> b_dist_q; 
	b_dist_q.push(Node(target, 0));
	b_dist[target]= 0; 

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
						f_dist[i]= d;
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
					d = b_dist[b_start]+ cost[b_start][i];
					if (d < b_dist[i]) {
						b_dist_q.push(Node(i, d)); 
						b_dist[i]= d;
						b_updater_of[i] = b_start;
					}
				}
			}
		}//end section
		}//end sections
		if (b_graphed[f_start] || f_graphed[b_start]) {
			break;
		}
	} 
	//최적해 x를 찾는다
	register int rtn = INT_MAX;
	for (i = 0; i < size; i++) {
		register int d = f_dist[i] + b_dist[i];
		if ( d < rtn ) {
			rtn = d;
			x = i;
		}
	} 
	if (PATH_TRUE) {
		f_updater_of[source] = -1; //print source -> x
		print_path(f_updater_of, x, size);

		b_updater_of[target] = -1;//print x -> target
		print_path_reverse(b_updater_of, x, size); 
	} return rtn;
}

int dijkstra(int** cost, int size, int source, int target)
{
	if(size > DO_PARALLEL)
		return bidirect_dijkstra(cost, size, source, target);
	else
		return uni_dijkstra(cost, size, source, target);
}

/*main*/
int main(int argc, char* argv[])
{
	/*Create File Output Stream*/
	F_OUT = fopen("201511298 dijkstra.txt", "a");
	print_param(argv);

	/*Get Parsed Parameters*/
	char* file_name = argv[1];
	N_THREAD = atoi(argv[2]);
	PATH_TRUE = atoi(argv[3]);
	int src = atoi(argv[4]);
	int target = atoi(argv[5]);
	DO_PARALLEL = atoi(argv[6]);
	/*Get Parsed Weight Array*/	
	int size = 0; 
	int **cost = csv_to_array(file_name, &size); //가중치 행렬 얻기
	omp_set_num_threads(N_THREAD); //스레드 수 설정

	/*Find Shortest Path*/	
	double time,total = 0;
	int dist, count = 0;
	for(int i = 3; i < 20; i++) 
		for(int j = i; j < i + 1000; j++){
			time = omp_get_wtime(); 
			dist = dijkstra(cost, size, i, j);
			time = omp_get_wtime() - time;
			total += time;
			count++;
			print_dist_time(dist, time); //거리와 실행시간 출력
			printf("avg time : %lf\n", 1000*total/count);
		}
	if(PATH_TRUE) {
		printf("\n");
		fprintf(F_OUT, "\n");
	}
	/*Print Out Computation Time*/

	/*Free Resources*/
	fclose(F_OUT);
	return 0;
}

/*Print Utils Impl.*/
void print_param(char** argv)
{
	printf("param: %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
	fprintf(F_OUT, "param: %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
}

void _print_student_id()
{
	printf("201511298 ");
	fprintf(F_OUT, "201511298 ");
}

void print_path(int* updater_of, int target, int size)
{
	int path[size];
	register int len = 0;

	do {
		path[len++] = target;
	} while ((target = updater_of[target]) != -1);

	_print_student_id();

	for(register int i = len - 1; i >= 0; i--) {
		printf("n%04d ", path[i]);
		fprintf(F_OUT, "n%04d ", path[i]);
	} 
}

void print_path_reverse(int* updater_of, int target, int size)
{
	while ((target = updater_of[target]) != -1) {
		printf("n%04d ", target);
		fprintf(F_OUT, "n%04d ", target);
	}
}

void print_dist_time(int dist, double wtime)
{
	_print_student_id();
	printf("Shortest Path: %d Compute time: %.5lf ms\n\n", dist, wtime * 1000);
	fprintf(F_OUT, "Shortest Path: %d Compute time: %.5lf ms\n\n", dist, wtime * 1000);
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
	#pragma omp parallel for
	for(int i = 0; i < size; i++) {
		for(int j = i; j < size; j++) {
			arr[j][i] = arr[i][j];
		}
	} return arr;
}
