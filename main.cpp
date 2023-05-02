#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <string.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <dlfcn.h>

void (*encrypt_func)(char *s, int key);
void (*decrypt_func)(char *s, int key);

struct task{
    char txt[1025];
    char flag[2];
    int key;
    int index;
};

int writeIndex;
std::queue<struct task> taskQ;
std::queue<pthread_t> threadQ;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockWrite = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t taskCond;
pthread_cond_t threadCond;
pthread_cond_t writeCond;

void *worker(void* arg){
    printf("worker\n");
    fflush(stdout);
    struct task currTask = *(struct task*)arg;
    if(strcmp(currTask.flag, "-e") == 0){
        encrypt_func(currTask.txt, currTask.key);
    }else{
        decrypt_func(currTask.txt, currTask.key);
    }
    printf("encrypt\n");
    fflush(stdout);
    printf("write index: %d\n", writeIndex);
    fflush(stdout);
    printf("task index: %d\n", currTask.index);
    fflush(stdout);
    pthread_mutex_lock(&lockWrite);
    while(writeIndex < currTask.index){
        pthread_cond_wait(&writeCond, &lockWrite);
    }
    printf("write\n");
    fflush(stdout);
    printf("%s", currTask.txt);
    fflush(stdout);
    writeIndex++;
    pthread_cond_broadcast(&writeCond);
    threadQ.push(pthread_self());
    pthread_cond_broadcast(&threadCond);
    pthread_mutex_unlock(&lockWrite);
}


//give jobs to the threads
void *manage(void* arg){
    // struct task currTask;
    // pthread_t currThread;
    printf("manage\n");
    fflush(stdout);
    while(1){
        // pthread_mutex_lock(&lock3);
        
        printf("enter\n");
        fflush(stdout);

        while(taskQ.empty()){
            pthread_mutex_lock(&lock);
            pthread_cond_wait(&taskCond, &lock);
        }
        // pthread_mutex_unlock(&lock);
        
        struct task currTask = taskQ.front();
        printf("struct index: %d\n", currTask.index);
        fflush(stdout);
        taskQ.pop();

        while(threadQ.empty()){
            pthread_mutex_lock(&lock2);
            pthread_cond_wait(&threadCond, &lock2);
        }
        // pthread_t currThread= threadQ.front();
        // threadQ.pop();
        // printf("struct index again: %d\n", currTask.index);
        pthread_create(&threadQ.front(), NULL, worker, (void*)&currTask);
        threadQ.pop();
        printf("struct index again: %d\n", currTask.index);
        // pthread_mutex_unlock(&lock3);
        // pthread_mutex_unlock(&lock2);
        // pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *argv[])
{
    int readIndex = 0;
    int cores = get_nprocs_conf();
    writeIndex = 0;

    void* handle = dlopen("./libCodec.so", RTLD_LAZY | RTLD_GLOBAL);
    if(!handle){
        fprintf(stderr, "dlerror: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    encrypt_func = (void (*)(char *s, int key))dlsym(handle,"encrypt");
    decrypt_func = (void (*)(char *s, int key))dlsym(handle,"decrypt");

	// if (argc != 3)
	// {
	//     printf("usage: key < file \n");
	//     printf("!! data more than 1024 char will be ignored !!\n");
	//     return 0;
	// }

    for(int i=0; i<cores; i++){ //cores-2?
        pthread_t curr_t;
        threadQ.push(curr_t);
    }

	int key = atoi(argv[1]);
	printf("key is %i \n",key);
    fflush(stdout);
    char flag[2];
    strcpy(flag, argv[2]);

    pthread_t manager; 

    pthread_create(&manager, NULL, manage, NULL);

	char c;
	int counter = 0;
	int dest_size = 1025;
	char data[dest_size];

    // printf("111\n");

	while ((c = getchar()) != EOF) 
	{
	  data[counter] = c;
	  counter++;

	  if (counter == 1024){
        data[1024] = '\0';
        struct task *t = (struct task*)malloc(sizeof(struct task));
        memset(t->txt, '\0', 1025);
        printf("read index: %d\n", readIndex);
        t->index = readIndex;
        t->key = key;
        strcpy(t->txt, data);
        strcpy(t->flag, flag);
        taskQ.push(*t);
        pthread_cond_broadcast(&taskCond);
		// encrypt(data,key);
		// printf("encripted data: %s\n",data);
		counter = 0;
        readIndex++;
	  }
	}
	
	if (counter > 0)
	{
		char lastData[counter];
		lastData[0] = '\0';
		strncat(lastData, data, counter);
        struct task *t = (struct task*)malloc(sizeof(struct task));
        memset(t->txt, '\0', 1025);
        t->index = readIndex;
        t->key = key;
        strcpy(t->txt, lastData);
        strcpy(t->flag, flag);
        taskQ.push(*t);
        pthread_cond_broadcast(&taskCond);
		// encrypt(lastData,key);
		// printf("encripted data:\n %s\n",lastData);
	}

    // for(int i=0; i<cores; i++){
    //     pthread_t* curr = threadQ.front();
    //     threadQ.pop();
    //     delete(curr);
    // }
    while(1){
        continue;
    }

	return 0;
}
