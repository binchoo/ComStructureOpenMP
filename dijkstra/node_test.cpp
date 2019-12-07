#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define INT_MAX 10000000
#define DO_PARALLEL 9999
int   n_thread;
int   path_true;
/*Test Utils*/
void _print_student_id	();
void  print_path	(char*);
void  print_dist_time	(int, double);
/*CSV Reader*/
int  _extract_str_int	(char* str);
int **csv_to_array	(char* file_name, int* get_size);

/*Test Array Utils*/
int **random2DArray	(int size);
int **symetricArray	(int** arr, int size);

/*Dijkstra uitls*/
/*속도 감소의 핵심*/
inline int arg_min_parallel(int* dist, int* graphed, int size)
{
	static int* local_arg_min = (int*)malloc(sizeof(int) * n_thread);
	register int i;
	#pragma omp parallel
	{
		register int my_id = omp_get_thread_num();
		register int local_min = INT_MAX;
		local_arg_min[my_id] = -1;
		#pragma omp for schedule(static, 13)
		for(i = 0; i < size; i++) {
			if(!graphed[i] && dist[i] < local_min) {
				local_min = dist[i];
				local_arg_min[my_id] = i;
			}
		}
	}

	register int glob_min = INT_MAX;
	register int arg_min = 0;
	register int my_arg;
	for(i = 0; i < n_thread; i++) {
		my_arg = local_arg_min[i];
		if(my_arg != -1 && dist[my_arg] < glob_min) {
			glob_min = dist[my_arg];
			arg_min = my_arg;
		}
	}
	return arg_min;
}

inline int arg_min(int* dist, int* graphed, int size)
{
	register int i;
	register int min = INT_MAX;
	register int arg_min;
	for(i = 0; i < size; i++) {
		if(!graphed[i] && dist[i] < min) {
			min = dist[i];
			arg_min = i;
		}
	}
	return arg_min;
}


/*Algorithm*/
int dijkstra(int** cost, int size, int source, int target)
{
	int dist[size];
	int graphed[size] = { 0 };
	int updater_of[size];
	register int i, start, d;

	for(i = 0; i < size; i++) {
		dist[i] = INT_MAX;
	}

	dist[source] = 0;
	updater_of[source] = -1;
	while(!graphed[target]) {

		if(size > DO_PARALLEL) {
			start = arg_min_parallel(dist, graphed, size);
			graphed[start] = 1;

			#pragma omp parallel for schedule(static, 13)
			for (i = 0; i < size; i++) {
				if(cost[start][i] && !graphed[i]) {
					d = dist[start] + cost[start][i];
					if (d < dist[i]) {
						dist[i] = d;
						updater_of[i] = start;
					}
				}
			}

		} else {
			start = arg_min(dist, graphed, size);
			graphed[start] = 1;

			for (i = 0; i < size; i++) {
				if(cost[start][i] && !graphed[i]) {
					d = dist[start] + cost[start][i];
					if (d < dist[i]) {
						dist[i] = d;
						updater_of[i] = start;
					}
				}
			}
		}
	}

	if(path_true) {

		int buffer[size];
		register int buf_len = 0;
		start = target;
		do {
			buffer[buf_len++] = start;
		} while ((start = updater_of[start]) != -1);

		_print_student_id();
		for(i = buf_len - 1; i >= 0; i--) {
			printf("n%04d ", buffer[i]);
		}
		printf("\n");
	}
	return dist[target];
}

/*main*/
//./serial mapDist1600.csv 4 0 200 1300
int main(int argc, char* argv[])
{
	char *file_name = argv[1];
	n_thread = atoi(argv[2]);
	path_true = atoi(argv[3]);
	int src = atoi(argv[4]);
	int target = atoi(argv[5]);

	int size = 0;
	int **cost = csv_to_array(file_name, &size);
	omp_set_num_threads(n_thread);
	double total_time = 0;
	int total = 0;
	for(int i = 0; i < size; i++) {
		for(int j = i + 1; j < size; j++) {
			double time = omp_get_wtime();
			int dist = dijkstra(cost, size, src, i);
			time = omp_get_wtime() - time;
//			print_dist_time(dist, time);
			total_time += time;
			total++;
		}
		printf("average time = %lf\n", total_time/total);
	}
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
						data[r-1][c-1] = 0;
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
