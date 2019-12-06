#include <stdio.h>
#include <omp.h>
#define N_THREADS 8
#define WTIME double

#define LEN 1024
#define N LEN
#define M LEN 
#define D LEN

WTIME block_wise_AB(double (*A)[M], double (*B)[D]);
WTIME block_wise_AB2(double (*A)[M], double (*B)[D]);
WTIME row_wise_AB(double (*A)[M], double (*B)[D]);
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

        row_wise_t = row_wise_AB(A, B);
        printf("%lf sec", row_wise_t);
        return 0;
}

WTIME block_wise_AB(double (*A)[M], double (*B)[D])
{
        double time = omp_get_wtime();
        #pragma omp parallel num_threads(N_THREADS)
        {
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
        }
        return omp_get_wtime() - time;
}

WTIME row_wise_AB(double (*A)[M], double (*B)[D])
{
        double time = omp_get_wtime();
        #pragma omp parallel num_threads(N_THREADS)
        {
                for(int n = omp_get_thread_num(); n < N; n+=8)
                {
                        for(int d = 0; d < D; d++)
                        {
                                double sum = 0;
                                for(int m = 0; m < M; m++)
                                        sum += A[n][m] * B[m][d];
                                result[n][d] = sum;
                        }
                }
        }
        return omp_get_wtime() - time;
}

WTIME block_wise_AB2(double (*A)[M], double (*B)[D])
{
        double time = omp_get_wtime();
        #pragma omp parallel num_threads(N_THREADS)
        {
		#pragma omp for
                for(int n = 0; n < N; n++)
                {
                        for(int d = 0; d < D; d++)
                        {
                                double sum = 0;
                                for(int m = 0; m < M; m++)
                                        sum += A[n][m] * B[m][d];
                                result[n][d] = sum;
                        }
                }
        }
        return omp_get_wtime() - time;
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

