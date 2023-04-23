#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <string.h>
#include <sys/sysinfo.h>
#include <pthread.h>

struct task{
    char* txt;
    int key;
    int index;
}Task;

int writeIndex;
std::queue<struct task> taskQ;
std::queue<pthread_t*> threadQ;

int main(int argc, char *argv[])
{
    int readIndex = 0;
    int cores = get_nprocs_conf();
    writeIndex = 0;

	if (argc != 2)
	{
	    printf("usage: key < file \n");
	    printf("!! data more than 1024 char will be ignored !!\n");
	    return 0;
	}

    for(int i=0; i<cores; i++){
        threadQ.push(new pthread_t);
    }

	int key = atoi(argv[1]);
	printf("key is %i \n",key);

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
        t->txt = data;
        t->key = key;
        t->txt = data;
        taskQ.push(*t);
		// encrypt(data,key);
		// printf("encripted data: %s\n",data);
		counter = 0;
	  }
      readIndex++;
	}
	
	if (counter > 0)
	{
		char lastData[counter];
		lastData[0] = '\0';
		strncat(lastData, data, counter);
        struct task *t = (struct task*)malloc(sizeof(struct task));
        t->txt = lastData;
        t->key = key;
        t->txt = data;
        taskQ.push(*t);
		// encrypt(lastData,key);
		// printf("encripted data:\n %s\n",lastData);
        readIndex++;
	}

    for(int i=0; i<cores; i++){
        pthread_t* curr = threadQ.front();
        threadQ.pop();
        delete(curr);
    }

	return 0;
}
