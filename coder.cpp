#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <unistd.h>

using namespace std;

# define dest_size 1024

struct Input
{
    int order;
    char* data;
    void (*func)(char *, int);

    Input(int order,char* data,void (*func)(char *, int)){
        this->order=order;
        this->data=data;
        this->func=func;
    }
    bool operator<(const Input& other) const {
        return order > other.order;
    }
};

queue<pthread_t> threadQueue;
queue<Input*> exQueue;
priority_queue<Input*> outPQ;
void* tempFunc(void* input);


int key;
int numberOfThreads;



int main(int argc, char *argv[])
{
	if (argc != 3)
	{
	    printf("usage: key < file \n");
	    printf("!! data more than 1024 char will be ignored !!\n");
	    return 0;
	}

    void (*func)(char *, int);
	char c;
	int counter = 0;
    int order=0;  
	char data[dest_size];
    numberOfThreads=sysconf(_SC_NPROCESSORS_CONF);
    pthread_t* thread_ids=new pthread_t [numberOfThreads];


    //save key
	key = atoi(argv[1]);
	printf("key is %i \n",key);

    // check if enc or dec
    if(! strcmp(argv[2],"-e")){
        func=encrypt;
    }
    else
    if(! strcmp(argv[2],"-d")){
         func=decrypt;
    }
    else
    {printf("error: enter -e or -d on the third argument");}


// make the threads in the thread pool

    for(int i=0; i< numberOfThreads; i++){
        pthread_create(&(thread_ids[i]), NULL, tempFunc, nullptr);
        // std::cout<<thread_ids[i]<<std::endl;
        threadQueue.push(thread_ids[i]);
        printf("Hello from thread %lu!\n", thread_ids[i]);
    }


    while ((c = getchar()) != EOF)
	{
	  data[counter] = c;
	  counter++;

	  if (counter == 1024){
        Input* input=new Input(order,data,func);
        exQueue.push(input);
		printf("encripted data- in the main while: %s\n",data);  
		counter = 0;
	  }
	}
	//the rest of the data that small than 1024
	if (counter > 0)
	{
		char lastData[counter];
		lastData[0] = '\0';
		strncat(lastData, data, counter);
		func(lastData,key);
		printf("encripted data:\n %s\n",lastData);
	}
     

}
void* tempFunc(void* input){
    Input* inp=(Input*) input;
    printf("%s",inp->data);
    // כאן מפעילים כל טרד והוא בודק מהתור משימות אם יש משימה אם לא לא עושה כלום, בטח יש משהו שמעיר אותם.
    //הם מוציאים משימות ומעורב מיוטיקס, וצריך לשמור את הסר איך שהוא
    // TODO: save the data and print with the real order of encrypt
    return nullptr;
}