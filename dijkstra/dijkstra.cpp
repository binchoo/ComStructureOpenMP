// dijkstra.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

//#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define MAX 100000
#define NODE 4

int dijsktra(int cost[NODE][NODE], int source, int target)
{
	int dist[NODE], prev[NODE], selected[NODE] = { 0 }, i, m, min, start, d, j=0;
	char path[NODE + 1][6];

	for (i = 0; i< NODE; i++)
	{
		dist[i] = MAX;
		prev[i] = -1; 
	}

	start = source;
	selected[start] = 1;
	dist[start] = 0;

	while (selected[target] == 0)
	{
		min = MAX;
		m = 0;

		for (i = 0; i< NODE; i++)
		{
			d = dist[start] + cost[start][i];
			if (d< dist[i] && selected[i] == 0)
			{
				dist[i] = d;
				prev[i] = start;
			}
			if (min>dist[i] && selected[i] == 0)
			{
				min = dist[i];
				m = i;
			}
		}
		start = m;
		selected[start] = 1;
	}
	start = target;
	
	while (start != -1)
	{
		char temp[6];
		_itoa_s(start, temp,5,10);
		//itoa(start, temp,5,10);

		if (start < 10){
			strcpy(path[j], "n000");
		}
		else if (start < 100){
			strcpy(path[j], "n00");
		}
		else if (start < 1000){
			strcpy(path[j], "n0");
		}
		else{
			strcpy(path[j], "n");
		}

		strcat(path[j++], temp);
		start = prev[start];
	}

	for (int i = j - 1; i >= 0; i--){
		printf("%s ", path[i]);
	}

	return dist[target];
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
	int i, j, w, ch, co;
	int source, target, x, y;

	//각 노드들 사이의 거리
	int cost[NODE][NODE] = { { 0,   30,   9,  MAX },
							 { 30,   0,   6,   12 },
							 { 9,    6,   0,  MAX },
							 { MAX, 12, MAX,    0 } };

	printf("\t The Shortest Path Algorithm ( DIJKSTRA'S ALGORITHM) \n\n");

	printf("Enter the source : ");
	//	scanf_s("%d", &source);
	scanf("%d", &source);
	printf("Enter the target : ");
	//scanf_s("%d", &target);
	scanf("%d", &target);
	co = dijsktra(cost, source, target);
	printf("\nThe Shortest Path: %d\n", co);
}
 
