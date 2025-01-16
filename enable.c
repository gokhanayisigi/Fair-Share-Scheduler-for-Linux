#include <linux/enableFair.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

int main (int argc, char * argv[]) 
{
    if (argc != 2) {
		printf("Input an argument. 0 for default, 1 for fair.\n");
		return 1;
	}
	int flag = atoi(argv[1]);
    enableFair(flag);
	return 0;
}