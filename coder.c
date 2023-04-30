#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


# define dest_size 1024

struct Input {
    int order;
    char data[1024];
    void (*func)(char *, int);
    struct Input* next;
}typedef Input;

void initInput(Input* input,int order,char* data,void (*func)(char *, int)){
input->order = order;
strcpy(input->data,data);
input->func = func,
input->next = NULL;
}


void* tempFunc();


int key;
int numberOfThreads;
Input *firstInput=NULL;
Input *lastInput=NULL;
pthread_mutex_t lock;




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
    numberOfThreads=5 ;// לבנתיים sysconf(_SC_NPROCESSORS_CONF);
    pthread_t* thread_ids = malloc(numberOfThreads * sizeof(pthread_t));


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

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex initialization failed.\n");
        exit(EXIT_FAILURE);
    }

// make the threads in the thread pool

    for(int i=0; i< numberOfThreads; i++){
        pthread_create(&thread_ids[i], NULL, tempFunc, NULL);
        
        printf("create thread number %d\n", i);
    }


    while ((c = getchar()) != EOF)
	{
	  data[counter] = c;
	  counter++;

	  if (counter == 1024){

        pthread_mutex_lock(&lock);

        Input* input = malloc(sizeof(struct Input));
        initInput(input,order,data,func);
        if(firstInput==NULL){
            firstInput=input;
            lastInput=firstInput;
        }
        else{
            lastInput->next=input;
            lastInput=lastInput->next;
            }
        order++;

        pthread_mutex_unlock(&lock);


		printf("encripted data- in the main while: %s\n",data);  
		counter = 0;
	  }
	}
	//the rest of the data that small than 1024
	if (counter > 0)
	{
        struct Input* input = malloc(sizeof(struct Input));
        input->next==NULL;
        input->func=func;
        input->order=order;
        strncpy(input->data, data, counter); 
		printf("last data:\n %s\n",input->data);

        pthread_mutex_lock(&lock);

        if(firstInput==NULL){
            firstInput=input;
            lastInput=firstInput;
        }
        else{
            lastInput->next=input;
            lastInput=lastInput->next;
            }
        order++;
        pthread_mutex_unlock(&lock);

	}
//need to put here wait
    for (int i = 0; i < numberOfThreads; i++) {
        pthread_join(thread_ids[i], NULL);
    }
}


void* tempFunc(){

    while(firstInput==NULL); // need to use wait, for no busy wating
    printf("the first data:%s\n",firstInput->data);
    // כאן מפעילים כל טרד והוא בודק מהתור משימות אם יש משימה אם לא לא עושה כלום, בטח יש משהו שמעיר אותם.
    //הם מוציאים משימות ומעורב מיוטיקס, וצריך לשמור את הסר איך שהוא
    // TODO: save the data and print with the real order of encrypt
    return NULL;
}