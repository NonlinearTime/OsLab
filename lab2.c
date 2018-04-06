#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>

int main (int argc, char ** argv) {
	printf("%s %s\n", argv[1], argv[2]);
	syscall(383,argv[1],argv[2]);
	return 0;
}