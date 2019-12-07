#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

int **rand_1d_array	(int size);
int **rand_2d_array	(int size);
int get_max		(int** arr, int size);
int get_max_parallel	(int** arr, int size, int n_thread);

int main(int argc, char **argv)
{
	int size = atoi(argv[1]);
	int **arr = rand_2d_array(size);

	printf("%s * %s array. %s threads.\n", argv[1], argv[1], argv[2]);

	int max;
	double wtime;
	#pragma omp parallel num_threads(2) private(max, wtime)
	{
		if(omp_get_thread_num() == 0) {
			printf("Serial max calc..\n");
			wtime = omp_get_wtime();
			max = get_max(arr, size);
			printf("max = %d \t SERIAL MAX TIME = %lf\n", max, omp_get_wtime() - wtime);
		} else {
			omp_set_nested(1);
			printf("Parallel max calc..\n");
			wtime = omp_get_wtime();
			max = get_max_parallel(arr, size, atoi(argv[2]));
			printf("max = %d \t PARALLEL MAX TIME = %lf\n", max, omp_get_wtime() - wtime);
		}
	}
	free(arr);
	return 0;
}
int **rand_1d_array(int size)
{
	int* rtn = (int*)malloc(sizeof(int)*size);
	for(int i = 0; i < size; i++)
		rtn[i] = 1000 - i / 2 + i*i;
}

int **rand_2d_array(int size)
{	
	int** rtn = (int**)malloc(sizeof(int*)*size);
	#pragma omp parallel for
	for(int i = 0; i < size; i++) {
		rtn[i] = (int*)malloc(sizeof(int)*size);
		#pragma omp parallel for
		for(int j = 0; j < size; j++) {
			rtn[i][j] = 100*(i + j) - i*i + j*j;
		}
	}
	return rtn;
}

int get_max(int** arr, int size)
{
	int rtn = 0;
	for(int i = 0; i < size; i++) {
		for(int j = 0; j < size; j++) {
			if(rtn < arr[i][j]) {
				rtn = arr[i][j];
			}
		}
	}
	return rtn;
}

int get_max_parallel(int** arr, int size, int n_thread)
{
	int glob_max = 0;
	int *local_maxes = (int*)calloc(n_thread ,sizeof(int));
	omp_set_num_threads(n_thread);

	#pragma omp parallel for
	for(int i = 0; i < size; i++) {
		for(int j = 0; j < size; j++) {
			if(local_maxes[omp_get_thread_num()] < arr[i][j]) {
				local_maxes[omp_get_thread_num()] = arr[i][j];
			}
		}	
	}

	for(int i = 0; i < n_thread; i++) {
		if(glob_max < local_maxes[i]) {
			glob_max = local_maxes[i];
		}
	}

	free(local_maxes);
	return glob_max;
}
