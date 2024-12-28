#include <stdio.h>

void foo()
{
	printf("FOO\n");
}
int main(int argc, char** argv)
{
	char* a = argv[1];
	char* b = argv[2];
	char* c = argv[3];
	if(a[1]=='a')
		printf("%s\n", a);
	printf("%s\n", b);
	printf("%s\n", c);
	foo();
}
