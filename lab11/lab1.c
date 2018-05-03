#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_SIZE 1024
#define MAX_NUM 64

void P(int semid) {
    struct sembuf sops = {0,-1,0};
   	semop(semid,&sops,1);
}

void V(int semid) {
	struct sembuf sops = {0,1,0};
	semop(semid,&sops,1);
}

union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

struct bufItem{
    char buff[MAX_SIZE];
    int size;
};

struct bufItem *shared_buf[MAX_NUM];

int full,empty,mutex;


void ReadBuf(int fp) {
    int i = 0;
    while (1) {
        P(empty);
        P(mutex);
        int ef = read(fp,shared_buf[i % MAX_NUM]->buff,MAX_SIZE);
        shared_buf[i % MAX_NUM]->size = ef;
        i++;
                
        V(mutex);
        V(full);
        printf("read : %d %d\n",ef,i);
        if (ef != MAX_SIZE) { 
            break;
        }
    }
}

void WriteBuf(int fp) {
    int i = 0;
    while (1) {    
        P(full);
        P(mutex);

        int ef = write(fp,shared_buf[i % MAX_NUM]->buff,shared_buf[i % MAX_NUM]->size);
        i++;
                    
        V(mutex);
        V(empty);
                    
        printf("write : %d %d\n",ef,i);
        if (ef != MAX_SIZE) {
            break;
        }
    }
}

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("please input 2 filename.");
        return 0;
    }
    void *shm = NULL;
    int shmid;
    for(int i = 0 ; i != MAX_NUM ; ++i) {
        //int k = ftok("/tmp/null",1);
        shmid = shmget(IPC_PRIVATE,sizeof(struct bufItem),IPC_CREAT | 0666);
        shm = shmat(shmid,NULL,0);
        shared_buf[i] = (struct bufItem *)shm;
    }   
    
    int key;

    union semun arg_1,arg_2,arg_3;


    key = ftok("/tmp",0x66);

    if (key < 0) {
    	perror("ftok key error.");
    	return -1;
    }

    full = semget(IPC_PRIVATE,1,IPC_CREAT | 0666);
    empty = semget(IPC_PRIVATE,1,IPC_CREAT | 0666);
    mutex = semget(IPC_PRIVATE,1,IPC_CREAT | 0666);
    if (full == -1 || empty == -1|| mutex == -1) {
    	perror("create semget error.");
    	return -1;
    }

    arg_1.val = 0;
    arg_2.val = MAX_NUM;
    arg_3.val = 1;

    if (semctl(full,0,SETVAL,arg_1) < 0) {
    	perror("Ctl sem1 error.");
		semctl(full,0,IPC_RMID,arg_1);
    	return -1;
    }

    if (semctl(empty,0,SETVAL,arg_2) < 0) {
    	perror("Ctl sem error.");
		semctl(empty,1,IPC_RMID,arg_2);
    	return -1;
    }

    if (semctl(mutex,0,SETVAL,arg_3) < 0) {
    	perror("Ctl sem error.");
		semctl(mutex,1,IPC_RMID,arg_2);
    	return -1;
    }

    pid_t read,write;

    read = fork();

    if (read == 0) {
        int fp = open(argv[1],O_RDONLY);
        if (fp == -1) {
            printf("Open file read.txt failed!\n");
            //getch();
            exit(1);
        }
        ReadBuf(fp);
        close(fp);
        printf("read finished!\n");
        exit(0);
    } else if (read > 0) {
        write = fork();
        if (write == 0) {
            int fp = open(argv[2],O_WRONLY | O_CREAT | O_APPEND, 777);
            if (fp == -1 ) {
                printf("Open file write.txt failed!\n");
                exit(1);
            }
            WriteBuf(fp);
            close(fp);
            printf("write finished!\n");
            exit(0);
        } else {
            wait(NULL);
            wait(NULL);
            printf("finished!\n");
            semctl(full,0,IPC_RMID,arg_1);
            semctl(empty,0,IPC_RMID,arg_2);
            semctl(mutex,0,IPC_RMID,arg_3);
            shmctl(shmid,IPC_RMID,0);

            exit(0);
        }
    } 
    return 0;
}