#include "segel.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

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


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    int num_of_threads, queue_size, max_size;
    char* schedalg;

    getargs(&port, &num_of_threads, &queue_size, &schedalg, &max_size, argc, argv);

    // 
    // HW3: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	//

    // Create a stats object
    stats_t stats;

    // Populate the arrival time with sample values
    double arrival_time = 1634025964.121233; // Example seconds value

    // Populate the dispatch interval with sample values
    double dispatch_interval = 1634025970.123111; // Example seconds value

    // Populate the handler thread stats with sample values
    stats.handler_thread_id = 1;
    stats.handler_thread_req_count = 100;
    stats.handler_thread_static_req_count = 80;
    stats.handler_thread_dynamic_req_count = 20;
	requestHandle(connfd, &stats, arrival_time, dispatch_interval);

	Close(connfd);
    }

}


    


 
