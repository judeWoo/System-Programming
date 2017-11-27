#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include "queue.h"

typedef struct args_t {
    char *num_workers;
    char *port_number;
    char *max_entries;
} args_t;

args_t *parse_args(int argc, char **argv);
int server_init(args_t *args);
int handle_request(int connfd);
int handle_put(int connfd, void *key, void *value);
int handle_get(int connfd, void *key);
int handle_evict(int connfd, void *key);
int handle_clear(int connfd);
void *thread(void *vargp);
void map_free_function(map_key_t key, map_val_t val);
void queue_free_function(void *item);

#endif