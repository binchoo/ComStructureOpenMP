#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void random_init(int** ref, int len, int maxnum);
void print(int* array, int len);
int get_max(int* unsorted, int len, int num_threads);
int get_max_logarithm(int* unsorted, int start, int len);
int get_max_serial(int* unsorted, int len);

void countsort(int** sorted, int* unsorted, int ulen, int max);
void countsort_serial(int** sorted, int* unsorted, int ulen, int max);
int equalarray(int*a, int*b, int len);

int main()
{
	int* unsorted, *sorted;
	int max, size, maxnum;
	double t;
	
	printf("[데이터개수 최댓값] >> ");
	scanf("%d %d", &size, &maxnum);
	random_init(&unsorted, size, maxnum);
	max = get_max(unsorted, size, 8);
	
	printf("=unsorted==========\n");
	print(unsorted, size);

	countsort(&sorted, unsorted, size, max);
	
	printf("=sorted==========\n");
	print(sorted, size);
	
	free(unsorted);
	free(sorted);
	return 0;
}

void random_init(int** ref, int len, int maxnum)
{
	int i = 0;
	*ref = (int*)malloc(sizeof(int) * len);
	srand(time(NULL));
	
	#pragma omp parallel for
	for(i = 0; i < len; i++)
		(*ref)[i] = (rand() % maxnum) + 1;
}

void print(int* array, int len)
{

	for(int i = 0; i < len; i++)
		printf("%d ", array[i]);
	printf("\n");
}

int get_max(int* unsorted, int len, int num_threads)
{
	double t = omp_get_wtime();
	int max;
	int local_max[10] = {0};

	omp_set_num_threads(num_threads);
	#pragma omp parallel for
	for(int i = 0; i < len; ++i)
	{
		int myid = omp_get_thread_num();
		if(local_max[myid] < unsorted[i])
			local_max[myid] = unsorted[i];
	}
	max = get_max_serial(local_max, num_threads);
	return max;
}

int get_max_serial(int* unsorted, int len)
{
	double t = omp_get_wtime();
	int max = 0;
	for(int i = 0; i < len; i++)
	{
		if(max < unsorted[i])
			max = unsorted[i];
	}
	return max;
}

int get_max_logarithm(int* unsorted, int start, int len)
{
	if(len <= 1)
	{
		return unsorted[start];
	}
	else
	{	
		int m_front, m_back;
		#pragma omp parallel
		{
			#pragma omp sections
			{
				#pragma omp section
				m_front = get_max_logarithm(unsorted, start, len/2);
				#pragma omp section
				m_back = get_max_logarithm(unsorted, start+len/2, len - len/2);
			}
		}
		return m_front > m_back ? m_front : m_back;
	}
}

void countsort(int** _sorted, int* unsorted, int ulen, int max)
{
	int blen = max + 1;
	int* sorted = (int*)malloc(sizeof(int)*ulen);
	int* bucket = (int*)malloc(sizeof(int)*blen);
	double t = omp_get_wtime();
	int offset = 0;
	int* accum = (int*)malloc(sizeof(int)*blen);
	accum[0] = 0;
	memset(bucket, 0, sizeof(int) * blen);
	#pragma omp parallel for reduction(+:bucket[:blen])
	for(int i = 0; i < ulen; i++)
	{
		int found = unsorted[i];
		bucket[found] += 1;
	}
	
	for(int i = 1; i < blen; i++)
		accum[i] = accum[i-1]+bucket[i-1];
	#pragma omp parallel for
	for(int i = 0; i < blen; i++)
	{
		int repeat = bucket[i];
		int offset = accum[i];
		#pragma omp parallel for
		for(int j = 0; j < repeat; j++)
		{
			sorted[offset + j] = i;
		}
		//offset += repeat; 
	}		
	*_sorted = sorted;
	free(bucket);
	return ;
}

void countsort_serial(int** _sorted, int* unsorted, int ulen, int max)
{
	int blen = max + 1;
	int* sorted = (int*)malloc(sizeof(int) * ulen);
	int* bucket = (int*)malloc(sizeof(int) * blen);
	int offset = 0;
	memset(bucket, 0, sizeof(int) * blen);
	double t = omp_get_wtime();
	for(int i = 0; i < ulen; i++)
	{
		int found = unsorted[i];
		bucket[found] += 1;
	}

	for(int i = 0; i < blen; i++)
	{
		int repeat = bucket[i];
		for(int j = 0; j < repeat; j++)
		{
			sorted[offset + j] = i;
		}
		offset += repeat;
	}
	*_sorted = sorted;
	free(bucket);
	return ;
}

int equalarray(int* a, int* b, int len)
{
	for(int i = 0; i < len; i++)
		if(a[i] != b[i])
			return 0;
	return 1;
}
