#include<stdio.h>

int main()
{
	int a[] = {1, 20, 999, 7777};
	for(int i = 0; i < sizeof(a)/sizeof(int); i++)
		printf("n%05d\n", a[i]);
	return 0;
}
