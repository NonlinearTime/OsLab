#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <signal.h>

#include <unistd.h>

#include <sys/wait.h>

#include <sys/types.h>

#include <time.h>

#include <iostream>

int main () {
	time_t now;
	struct tm * tm_now;
	while (1) {
		time(&now);
		tm_now = localtime(&now);
		std::cout 
		<< tm_now->tm_year+1900 << "." 
		<< tm_now->tm_mon+1 << "." 
		<< tm_now->tm_mday << "." 
		<< tm_now->tm_hour << "." 
		<<tm_now->tm_min  << "."
		<< tm_now->tm_sec 
		<< std::endl ;
		sleep(1);
	}
	return 0;
}