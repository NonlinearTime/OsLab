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
	int sum = 0;
	for (int i = 0 ; i <= 1000; ++i) {
		sum += i;
		cout << sum << endl;
		sleep(1);
	}
	return 0;
}