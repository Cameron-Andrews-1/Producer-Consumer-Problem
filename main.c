//main.c
//main file for producer/consumer problem
//to run file, main parameters are p(# producers), c(# consumers), i(# of items produced by each producer), and d(delay option)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER; //condition for buffer that isn't full
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER; //condition for buffer that isn't empty
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //mutex lock

int buffer[8]; //buffer for products
int front = 0; //number of products currently in buffer
int rear = 0;

struct producer_args
{
    int i; //number of items produced by each producer
    int id; //id of producer
    int d;
};

struct consumer_args
{
    int i; //number of items produced by each producer
    int c; //number of consumers
    int p; //number of producers
    int id; //id of consumer
    int d;
};

//producer thread
void* producer_thread(void* input) //input is pointer to producer_args struct)
{
    int i = ((struct producer_args*)input)->i;
    int id = ((struct producer_args*)input)->id;
    int d = ((struct producer_args*)input)->d;
    int index;
    for (index = 0; index < i; ++index)
    {
        pthread_mutex_lock(&lock); //locks mutex
        while ((rear+1)%9 == front%9)
        {
            pthread_cond_wait(&buffer_not_full, &lock);   
        }
        int product = (id * i) + index; //unique product
        buffer[rear++] = product; //adds product to buffer
        printf("producer_%d produced item %d\n", id, product);
        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&lock);
        if (d == 0)
        {
            usleep(.5);
        }
    }
    free(input);
}

//consumer thread
void* consumer_thread(void* input)
{
    int i = ((struct consumer_args*)input)->i;
    int c = ((struct consumer_args*)input)->c;
    int p = ((struct consumer_args*)input)->p;
    int d = ((struct consumer_args*)input)->d;
    int id = ((struct consumer_args*)input)->id;
    int consume_num = (p*i)/c; //number of items each consumer consumes
    int consume;
    for (consume = 0; consume < consume_num; ++consume)
    {
        pthread_mutex_lock(&lock); //locks mutex
        while (rear%9 == front%9)
        {
            pthread_cond_wait(&buffer_not_empty, &lock);
        }
        int product = buffer[front++];
        printf("consumer_%d consumed item %d\n", id, product);
        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&lock);
        if (d == 1)
        {
            usleep(.5);
        }
    }
    free(input);
}

int main(int argc, char** argv)
{
    
    int p = atoi(argv[1]); //number of producers
    int c = atoi(argv[2]); //number of consumers
    int i = atoi(argv[3]); //number of items produced by each producer
    int d = atoi(argv[4]); //delay option (0 -> producer delay, 1 -> consumer delay)

    pthread_t pthreads[p]; //array of producer threads
    pthread_t cthreads[c]; //array of consumer threads  

    int producer;
    //struct producer_args* pargs[p];
    for (producer = 0; producer < p; ++producer) //creating producer threads
    {
        struct producer_args* pargs = (struct producer_args*)malloc(sizeof(struct producer_args));
        pargs->i = i;
        pargs->id = producer;
        pargs->d = d;
        pthread_create(&pthreads[producer], NULL, producer_thread, pargs);
    }

    int consumer;
    for (consumer = 0; consumer < c; ++consumer) //creating consumer threads
    {
        struct consumer_args* cargs = (struct consumer_args*)malloc(sizeof(struct consumer_args));
        //struct consumer_args cargs;
        cargs->i = i;
        cargs->c = c;
        cargs->p = p;
        cargs->d = d;
        cargs->id = consumer;
        pthread_create(&cthreads[consumer], NULL, consumer_thread, cargs);
    }

    for (producer = 0; producer < p; ++producer) //joining producer threads
    {
        pthread_join(pthreads[producer], NULL);
    }

    for (consumer = 0; consumer < c; ++consumer)
    {
        pthread_join(cthreads[consumer], NULL);
    }
    return 0;
}

