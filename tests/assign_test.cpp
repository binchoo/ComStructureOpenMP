#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int **rand_2d_array	(int size);
int assign		(int* arr, int size);
int assign_parallel	(int* arr, int size, int n_thread);

int main(int argc, char **argv)
{
	int size = atoi(argv[1]);
	int *arr = (int*)malloc(sizeof(int) * size);

	printf("%s length array. %s threads.\n", argv[1], argv[2]);

	double wtime;
	#pragma omp parallel num_threads(2) private(wtime)
	{
		if(omp_get_thread_num() == 0) {
			printf("Serial array assign..\n");
			wtime = omp_get_wtime();
			assign(arr, size);
			printf("SERIAL ASSIGN TIME = %lf\n", omp_get_wtime() - wtime);
		} else {
			omp_set_nested(1);
			printf("Parallel array assign..\n");
			wtime = omp_get_wtime();
			assign_parallel(arr, size, atoi(argv[2]));
			printf("PARALLEL ASSIGN TIME = %lf\n", omp_get_wtime() - wtime);
		}
	}
	free(arr);
	return 0;
}

int assign(int* arr, int size)
{
	for(int i = 0; i < size; i++)
		arr[i] = 1024;
	return 1;
}

int assign_parallel(int* arr, int size, int n_thread)
{
	omp_set_num_threads(n_thread);
	#pragma omp parallel for
	for(int i = 0; i < size; i++)
		arr[i] = 1024;
	return 1;
}
