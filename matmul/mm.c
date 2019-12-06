//%cflags:-fopenmp
#include <stdio.h>
#include <omp.h>

#define N_THREADS 8
#define LEN 1024

#define N LEN
#define M LEN
#define D LEN

typedef double WTIME;
/*스레드 별 실행시간*/
WTIME times[N_THREADS];

/*블록 별 워크로드*/
WTIME block_wise_AB(double (*A)[M], double (*B)[D]);

/*행 별 워크로드*/
WTIME row_wise_AB(double (*A)[M], double (*B)[D]);

/*가장 오래 걸린 실행시간을 의미합니다*/
WTIME get_max_time(WTIME times[N_THREADS]);

/*행렬을 콘솔에 출력합니다*/
void print_result(double result[N][D]);

double A[N][M] = {0};
double B[M][D] = {0};
double result[N][D] = {0};

int main()
{
	WTIME block_wise_t;
	WTIME row_wise_t;

	#pragma omp parallel for
	for(int i = 0; i < N; i++)
		for(int j = 0; j < M; j++)
			A[i][j] = i;

	#pragma omp parallel for
	for(int i = 0; i < M; i++)
		for(int j = 0; j < D; j++)
			B[i][j] = j;

	block_wise_t = block_wise_AB(A, B);
	//print_result(result);
	row_wise_t = row_wise_AB(A, B);
	//print_result(result);
	printf("%lf sec\n", block_wise_t);
	printf("%lf sec\n", row_wise_t);
	return 0;
}

WTIME block_wise_AB(double (*A)[M], double (*B)[D])
{
	#pragma omp parallel num_threads(N_THREADS)
	{
		#pragma omp barrier
        	WTIME my_start_time = omp_get_wtime();
		int offset = N / N_THREADS * omp_get_thread_num();
	
		for(int n = 0; n < N / N_THREADS; n++)
		{
			for(int d = 0; d < D; d++)
			{
				double sum = 0;
				for(int m = 0; m < M; m++)
					sum+= A[offset + n][m] * B[m][d];
				result[offset + n][d] = sum;
			}
		}
		times[omp_get_thread_num()] = omp_get_wtime() - my_start_time;
	}
	return get_max_time(times);
}

WTIME row_wise_AB(double (*A)[M], double (*B)[D])
{
	#pragma omp parallel num_threads(N_THREADS)
	{
		#pragma omp barrier
		WTIME my_start_time = omp_get_wtime();
		for(int n = omp_get_thread_num(); n < N; n += N_THREADS)
		{
			for(int d = 0; d < D; d++)
			{
				double sum = 0;
				for(int m = 0; m < M; m++)
					sum += A[n][m] * B[m][d];
				result[n][d] = sum;
			}
		}
		times[omp_get_thread_num()] = omp_get_wtime() - my_start_time;
	}
	return get_max_time(times);
}

WTIME get_max_time(WTIME times[N_THREADS])
{
	WTIME max = times[0];
	int i;
	for(i = 1; i < N_THREADS; i++)
	{
		if(max < times[i])
			max = times[i];
	}
	return max;
}

void print_result(double result[N][D])
{
	int i, j;
	for(i = 0; i < N; i++)
	{
		for(j = 0; j < D; j++)
			printf("%6.2f ", result[i][j]);
		printf("\n");
	}
}
