#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <signal.h>

#include <unistd.h>

#include <sys/wait.h>

#include <sys/types.h>

#include <iostream>

using namespace std;

int main (int argc, char ** argv) {
	int counter = 0;
	while (1) {
		cout << counter << endl;
		if (++counter == 10) counter = 0;	
		sleep(1);
	}
	return 0;
}