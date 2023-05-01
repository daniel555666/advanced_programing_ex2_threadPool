#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stddef.h>
#include <strings.h>

#define dest_size 1024

struct Input
{
    int order;
    char data[1024];
    void (*func)(char *, int);
    struct Input *next;
} typedef Input;

void initInput(Input *input, int order, char *data, void (*func)(char *, int))
{
    input->order = order;
    strcpy(input->data, data);
    input->func = func,
    input->next = NULL;
}

// This function will insert the struct into a list sorted ASC by the order property

void insert(Input **head, Input *input, pthread_mutex_t *mutex_lock)
{
    pthread_mutex_lock(mutex_lock);
    if(*head == NULL){
        *head = input;
        pthread_mutex_unlock(mutex_lock);
        return;
    }
    if(*head != NULL && (*head)->order == 0){
        *head = input;
        pthread_mutex_unlock(mutex_lock);
        return;
    }
    if ((*head)->order > input->order)
    {
        input->next = *head;
        *head = input;
        pthread_mutex_unlock(mutex_lock);
        return;
    }

    struct Input *curr = *head;
    while (curr->next != NULL && curr->next->order < input->order)
    {
        curr = curr->next;
    }
    input->next = curr->next;
    curr->next = input;
    pthread_mutex_unlock(mutex_lock);
}


// This function will return the value of the first input in the list

Input *getFirst(Input **input, pthread_mutex_t *mutex_lock)
{
    pthread_mutex_lock(mutex_lock);
    if (input == NULL || *input == NULL) 
    {
        pthread_mutex_unlock(mutex_lock);
        return NULL;
    }
    Input *result = *input;
    pthread_mutex_unlock(mutex_lock);
    return result;
}


// This function will remove the first input in the list

void removeFirst(Input **input, pthread_mutex_t *mutex_lock)
{
    pthread_mutex_lock(mutex_lock);
    if (*input == NULL)
    {
        pthread_mutex_unlock(mutex_lock);
        return;
    }
    struct Input *temp = *input;
    *input = (*input)->next;
    free(temp);
    pthread_mutex_unlock(mutex_lock);
}

// This function will remove and free the first input in the list

void freeFirst(Input **input, pthread_mutex_t *mutex_lock)
{
    pthread_mutex_lock(mutex_lock);
    if (*input == NULL)
    {
        pthread_mutex_unlock(mutex_lock);
        return;
    }
    struct Input *temp = *input;
    *input = (*input)->next;
    free(temp);
    pthread_mutex_unlock(mutex_lock);
}

// This function will remove and return the first input in the list

Input *popFirst(Input **input, pthread_mutex_t *mutex_lock)
{
    pthread_mutex_lock(mutex_lock);
    if (*input == NULL)
    {
        pthread_mutex_unlock(mutex_lock);
        return NULL;
    }
    struct Input *temp = *input;
    *input = (*input)->next;
    pthread_mutex_unlock(mutex_lock);
    return temp;
}


void *tempFunc();

int key;
int numberOfThreads;
// Input *firstInput=NULL;
// Input *lastInput=NULL;
pthread_mutex_t lock;
Input *input_list=NULL;
Input *output_list=NULL;
pthread_mutex_t task_lock;
pthread_mutex_t output_lock;
int output_order = 1;
int is_finished = 0;
int first_input = 0;
int first_output = 0;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: key < file \n");
        printf("!! data more than 1024 char will be ignored !!\n");
        return 0;
    }

    //for debugging only
    FILE *fp;
    fp = freopen("text.txt", "r", stdin); // Open "input.txt" and redirect it as stdin
    if (fp == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    //end for debugging only

    void (*func)(char *, int);
    char c;
    int counter = 0;
    int order = 1;
    // char data[dest_size];
    char * data;
    data = (char *) malloc (dest_size * sizeof(char));
    bzero(data, dest_size);
    numberOfThreads = 2; // לבנתיים sysconf(_SC_NPROCESSORS_CONF);
    pthread_t *thread_ids = malloc(numberOfThreads * sizeof(pthread_t));

    //save key
    // system("text.txt < cat");
    // if(getchar() == 'c'){
    //     printf(getchar());
    // }
    key = atoi(argv[1]);
    printf("key is %i \n", key);

    // check if enc or dec
    if (!strcmp(argv[2], "-e"))
    {
        func = &encrypt;
    }
    else if (!strcmp(argv[2], "-d"))
    {
        func = &decrypt;
    }
    else
    {
        printf("error: enter -e or -d on the third argument");
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("Mutex initialization failed.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&task_lock, NULL) != 0)
    {
        printf("Mutex initialization failed.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&output_lock, NULL) != 0)
    {
        printf("Mutex initialization failed.\n");
        exit(EXIT_FAILURE);
    }

    // make the threads in the thread pool

    for (int i = 0; i < numberOfThreads; i++)
    {
        pthread_create(&thread_ids[i], NULL, tempFunc, NULL);
        printf("create thread number %d\n", i);
    }

    while ((c = getchar()) != EOF)
    {
        data[counter] = c;
        counter++;
        if (counter == 1024)
        {

            pthread_mutex_lock(&lock);

            Input *input = malloc(sizeof(struct Input));
            initInput(input, order, data, func);
            insert(&input_list, input, &task_lock);

            order++;

            pthread_mutex_unlock(&lock);

            Input *first = getFirst(output_list, &output_lock);
            if (first != NULL && first->order == output_order)
            {
                printf("%s", first->data);
                freeFirst(&output_list, &output_lock);
            }

            printf("encripted data- in the main while: %s\n", data);
            counter = 0;
        }
    }
    //the rest of the data that small than 1024
    if (counter > 0)
    {
        struct Input *input = malloc(sizeof(struct Input));
        input->next == NULL;
        input->func = func;
        input->order = order;
        strncpy(input->data, data, counter);
        // initInput(input, order, data, func);
        // printf("last data:\n %s\n", input->data);

        pthread_mutex_lock(&lock);

        insert(&input_list, input, &task_lock);
        bzero(data, dest_size);
        order++;
        is_finished = 1;
        pthread_mutex_unlock(&lock);
    }
    while (getFirst(&output_list, &output_lock) != NULL)
    {
        Input *first = getFirst(&output_list, &output_lock);
        if (first->order == output_order)
        {
            printf("%s", first->data);
            freeFirst(&output_list, &output_lock);
            output_order++;
        }
    }
    //need to put here wait
    for (int i = 0; i < numberOfThreads; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

    // Print the rest of the data
    while (getFirst(&output_list, &output_lock) != NULL)
    {
        Input *first = getFirst(&output_list, &output_lock);
        if (first->order == output_order)
        {
            printf("%s", first->data);
            freeFirst(&output_list, &output_lock);
            output_order++;
        }
    }
    printf("\n");
}

void *tempFunc()
{
    while (1)
    {
        Input *task;
        task = popFirst(&input_list, &task_lock);
        if (task == NULL && is_finished != 0)
        {
            break;
        }
        while (task == NULL)
        {
            if(is_finished != 0){
                return;
            }
            task = popFirst(&input_list, &task_lock);
        }
        // We do not need to wait since we use mutex to allow the thread to wait until they get the input;
        // while(task==NULL){
        //     continue; // wait till you get a task
        // } // need to use wait, for no busy wating

        // activate the encrypt or decrypt function
        (task->func)(task->data, key);
        // insert the task into the output list
        insert(&output_list, task, &output_lock);

        // printf("the first data:%s\n",firstInput->data);
        // כאן מפעילים כל טרד והוא בודק מהתור משימות אם יש משימה אם לא לא עושה כלום, בטח יש משהו שמעיר אותם.
        //הם מוציאים משימות ומעורב מיוטיקס, וצריך לשמור את הסר איך שהוא
        // TODO: save the data and print with the real order of encrypt
    }
    // return NULL;
}