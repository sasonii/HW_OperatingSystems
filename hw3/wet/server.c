#include "segel.h"
#include "request.h"
#include "queue.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// declerations
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
int num_of_curr_working = 0;
void *thread_func(void* args);
int handle_overloading(int connfd, Queue* queue, char* schedalg, int* queue_size,int max_size);

// HW3: Parse the new arguments too
void getargs(int *port, int* num_of_threads, int* queue_size, char** schedalg, int* max_size, int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Wrong Usage for: %s\n", argv[0]); //"Usage: %s <port>\n"
	exit(1);
    }
    *port = atoi(argv[1]);
    *num_of_threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = argv[4];
    if(!strcmp(*schedalg, "dynamic")){
        if (argc < 6) {
            fprintf(stderr, "Wrong Usage for: %s\n", argv[0]); //"Usage: %s <port>\n"
            exit(1);
        }
        *max_size = atoi(argv[5]);
    }
}

typedef struct {
    int id;
    Queue* queue;
    // Add more fields for other types as needed
} ThreadData;

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    Queue* queue = createQueue();
    int num_of_threads, queue_size, max_size;
    char* schedalg;

    getargs(&port, &num_of_threads, &queue_size, &schedalg, &max_size, argc, argv);

    ThreadData threadData[num_of_threads];
    pthread_t threads_array[num_of_threads];
    for (int i = 0; i < num_of_threads; i++) //creating the thread pool
    {
        threadData[i].id = i;
        threadData[i].queue = queue;
        pthread_create(&threads_array[i], NULL, thread_func,&(threadData[i]));
    }

    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    double arrival_time = Time_GetSeconds();
    pthread_mutex_lock(&mutex2);
    int is_overloaded = queue->queue_size + num_of_curr_working >= queue_size;
    pthread_mutex_unlock(&mutex2);
    if(is_overloaded){
        if(handle_overloading(connfd, queue, schedalg, &queue_size, max_size)){
            continue; //in some cases we would like to close the socket, so no need to insert into queue
        }
    }

    pthread_mutex_lock(&mutex);
    enqueue(queue, connfd, arrival_time);
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cond);
    }
}

void *thread_func(void* args){
    ThreadData * threadData = (ThreadData*)args;
    int id = threadData->id;
    Queue* queue = threadData->queue;

    int connfd;
    int element;
    double* arrival_time = (double *) malloc(sizeof(double*));
    double* dispatch_interval = (double *) malloc(sizeof(double*));

    // Create a stats object for this thread
    stats_t stats;
    stats.handler_thread_id = id;
    stats.handler_thread_req_count = 0;
    stats.handler_thread_static_req_count = 0;
    stats.handler_thread_dynamic_req_count = 0;

    while(1){
        pthread_mutex_lock(&mutex);
        pthread_mutex_lock(&mutex2);
        element = dequeue(queue, arrival_time, dispatch_interval);
        if(element != -1){
            num_of_curr_working++;
        }
        pthread_mutex_unlock(&mutex2);

        while(element == -1){
            pthread_cond_wait(&cond, &mutex);
            pthread_mutex_lock(&mutex2);
            element = dequeue(queue, arrival_time, dispatch_interval);
            if(element != -1){
                num_of_curr_working++;
            }
            pthread_mutex_unlock(&mutex2);

        }
        pthread_mutex_unlock(&mutex);

        connfd = (int)element;
        requestHandle(connfd, &stats, arrival_time, dispatch_interval);

        pthread_mutex_lock(&mutex2); //lock cur_worker counter
        num_of_curr_working--;
        pthread_mutex_unlock(&mutex2); //unlock cur_worker counter
        pthread_cond_signal(&cond2);

        Close(connfd);
    }

    free(arrival_time);
    free(dispatch_interval);
}

int handle_overloading(int connfd, Queue* queue, char* schedalg, int* queue_size,int max_size){
    if(!strcmp(schedalg, "block")){
        pthread_mutex_lock(&mutex2);
        while(queue->queue_size + num_of_curr_working >= *queue_size){
            pthread_cond_wait(&cond2, &mutex2);//wait for queue to stop being full
        }
        pthread_mutex_unlock(&mutex2);
        return 0;
    }

    if(!strcmp(schedalg, "dt")){
        Close(connfd);
        return 1; //continue to next request
    }

    if(!strcmp(schedalg, "dh")){
        pthread_mutex_lock(&mutex);
        int connfd_to_close = dequeue(queue, NULL, NULL);
        if(connfd_to_close == -1){
            Close(connfd);
            pthread_mutex_unlock(&mutex);
            return 1;
        } else{
            Close(connfd_to_close);
            pthread_mutex_unlock(&mutex);
            return 0; //add this request to queue
        }
    }

    if(!strcmp(schedalg, "bf")){
        pthread_mutex_lock(&mutex2);
        while(queue->queue_size + num_of_curr_working != 0){
            pthread_cond_wait(&cond2, &mutex2);//wait for queue to stop being full
        }
        pthread_mutex_unlock(&mutex2);
        return 0;
    }

    if(!strcmp(schedalg, "dynamic")){
        Close(connfd);
        if(*queue_size < max_size){
            (*queue_size)++;
        }
        return 1; //continue to next request
    }

    if(!strcmp(schedalg, "random")){
        int random_to_del;
        pthread_mutex_lock(&mutex);
        if(queue->queue_size == 0){
            pthread_mutex_unlock(&mutex);
            Close(connfd);
            return 1;
        }
        int size_to_del = (queue->queue_size)/2;
        int connfd_to_close;
        for (int i = 0; i < size_to_del; i++)
        {
            random_to_del = rand()%((queue->queue_size)) + 1;
            printf("%d, %d\n", queue->queue_size, random_to_del);
            connfd_to_close = dequeue_i(queue, random_to_del);
            Close(connfd_to_close);
        }
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    // should not get here
    return 0;
}


    


 
