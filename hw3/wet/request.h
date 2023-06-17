#ifndef __REQUEST_H__

typedef struct {
    int handler_thread_id;
    int handler_thread_req_count;
    int handler_thread_static_req_count;
    int handler_thread_dynamic_req_count;
} stats_t;

void requestHandle(int fd, stats_t* stats, double* arrival_time, double* dispatch_interval);

#endif
