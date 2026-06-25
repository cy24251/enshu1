#include <stdio.h>

extern void f();
extern void g();

int main()
{
	printf("function main is called\n");
	f();
	g();
	return 0;
}

