#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef MY_BUF_SIZE
#define MY_BUF_SIZE 256
#endif

int main() {
	int mydev, i, ret;
	char buf[MY_BUF_SIZE];

	/*open mydev*/
	mydev = open("/dev/mydev",O_RDWR | O_NONBLOCK);
	if (mydev < 0) {
		printf("open mydev fail !\n");
		return -1;
	}
	printf("open mydev success !\n");

	/*set buf*/
	printf("the chars to write.\n");
	for (i = 0 ; i < MY_BUF_SIZE ; ++i) {
		buf[i] = i + 'A';
		printf("%c", buf[i]);
	}

	printf("\n");

	/*write mydev*/
	ret = write(mydev,buf,MY_BUF_SIZE);
	if (ret < 0) printf("write mydev fail !\n");
	else printf("write mydev success !\n");

	// if ((ret = lseek(mydev,0,SEEK_SET)) == -1) printf("lseek mydev fail !\n");
	// else printf("lseek mydev success !\n");
	close(mydev);
	mydev = open("/dev/mydev",O_RDWR | O_NONBLOCK);

	/*read mydev*/
	ret = read(mydev,buf,MY_BUF_SIZE);
	if (ret < 0) printf("read my dev fail !\n");
	else printf("read mydev success !\n");

	for (i = 0 ; i < MY_BUF_SIZE ; ++i) {
		printf("buf[%d]:%c\n", i ,buf[i] );
	}

	close(mydev);

	return 0;
}