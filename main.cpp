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
    char txt[1024];
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
    struct task *currTask = (struct task*)arg;
    if(strcmp(currTask->flag, "-e") == 0){
        encrypt_func(currTask->txt, currTask->key);
    }else{
        decrypt_func(currTask->txt, currTask->key);
    }
    pthread_mutex_lock(&lockWrite);
    while(writeIndex < currTask->index){
        pthread_cond_wait(&writeCond, &lockWrite);
    }
    printf("%s", currTask->txt);
    fflush(stdout);
    writeIndex++;
    pthread_cond_broadcast(&writeCond);
    threadQ.push(pthread_self());
    pthread_cond_broadcast(&taskCond);
    pthread_mutex_unlock(&lockWrite);
}


//give jobs to the threads
void *manage(void* arg){
    while(1){
        pthread_mutex_lock(&lock3);
        pthread_mutex_lock(&lock);

        while(taskQ.empty()||threadQ.empty()){
            pthread_cond_wait(&taskCond, &lock);
        }
        
        struct task *currTask = (struct task*)malloc(sizeof(struct task));
        *currTask = taskQ.front();
        pthread_create(&threadQ.front(), NULL, worker, (void*)currTask);
        threadQ.pop();
        taskQ.pop();
        pthread_mutex_unlock(&lock3);
        pthread_mutex_unlock(&lock);
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

    for(int i=0; i<cores; i++){ 
        pthread_t curr_t;
        threadQ.push(curr_t);
    }

	int key = atoi(argv[1]);
    char flag[2];
    strcpy(flag, argv[2]);

    pthread_t manager; 

    pthread_create(&manager, NULL, manage, NULL);

	char c;
	int counter = 0;
	int dest_size = 1024;
	char data[dest_size];

	while ((c = getchar()) != EOF) 
	{
	  data[counter] = c;
	  counter++;

	  if (counter == 1024){
        struct task *t = (struct task*)malloc(sizeof(struct task));
        memset(t->txt, '\0', 1024);
        t->index = readIndex;
        t->key = key;
        strcpy(t->txt, data);
        strcpy(t->flag, flag);
        taskQ.push(*t);
        pthread_cond_broadcast(&taskCond);
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
        memset(t->txt, '\0', 1024);
        t->index = readIndex;
        t->key = key;
        strcpy(t->txt, lastData);
        strcpy(t->flag, flag);
        taskQ.push(*t);
        pthread_cond_broadcast(&taskCond);
	}
    while(1){
        continue;
    }

	return 0;
}
