#include "cream.h"
#include "utils.h"
#include "csapp.h"
#include "server.h"
#include "debug.h"

queue_t *queue;
hashmap_t *hashmap;
sem_t fds;
pthread_mutex_t lock;

int main(int argc, char *argv[]) {
    args_t *args;
    int exit_code;

    signal(SIGPIPE, SIG_IGN); //Ignore SIGPIPE

    if (!(args = parse_args(argc, argv))) {
        app_error("invalid number of arguments.\n");
    }

    /* Init lock */
    if (sem_init(&(fds), 0, 0) == -1) //setting # of items to 0
    {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&(lock), NULL) != 0) //setting mutex to NULL
    {
        perror("pthread_mutex_init failed");
        exit(EXIT_FAILURE);
    }

    exit_code = server_init(args);

    free(args->port_number);
    free(args->num_workers);
    free(args->max_entries);
    free(args);

    /* Destroy queue & hashmap */
    // invalidate_queue(queue, queue_free_function);
    // invalidate_map(hashmap);

    /* Destroy lock */
    if (sem_destroy(&fds) == -1) //setting # of items to 0
    {
        perror("sem_destroy failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_destroy(&lock) != 0) //setting mutex to NULL
    {
        perror("pthread_mutex_destory failed");
        exit(EXIT_FAILURE);
    }
    exit(exit_code);
}

args_t *parse_args(int argc, char **argv) {
    if (argc != 4) {

        if (getopt(argc, argv, "h") != -1)
        {
            printf("Usage: %s [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES\n-h\tDisplays this help menu and returns EXIT_SUCCESS.\nNUM_WORKERS\tThe number of worker threads used to service requests.\nPORT_NUMBER\tPort number to listen on for incoming connections.\nMAX_ENTRIES\tThe maximum number of entries that can be stored in `cream`'s underlying data store.\n", argv[0]);
            exit(EXIT_SUCCESS);
        }
        return NULL;
    }

    args_t *args = Calloc(1, sizeof(args_t));
    args->num_workers = strdup(argv[1]);
    args->port_number = strdup(argv[2]);
    args->max_entries = strdup(argv[3]);
    return args;
}

int server_init(args_t *args)
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    uint32_t number_workers = (uint32_t) atoi(args->num_workers);
    uint32_t maximum_entries = (uint32_t) atoi(args->max_entries);
    pthread_t tid[number_workers];

    /* Init a queue */
    queue = create_queue();

    /* Init a hashmap */
    hashmap = create_map(maximum_entries, jenkins_one_at_a_time_hash, map_free_function);

    /* Create threads */
    for (uint32_t i = 0; i < number_workers; ++i)
    {
        Pthread_create(&tid[i], NULL, thread, NULL);
    }

    /* Bind & Listen */
    listenfd = Open_listenfd(args->port_number);

    while (1)
    {
        /* Accept */
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Calloc(1, sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        /* Enqueue */
        if(!enqueue(queue, connfdp))
        {
            perror("enqueue failed");
            exit(EXIT_FAILURE);
        }

        /* Increase # of fd */
        if (sem_post(&(fds)) != 0) //up semaphore
        {
            perror("sem_post failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

int handle_request(int connfd)
{
    request_header_t request_header;
    response_header_t response_header;

    void *key;
    void *value;

    Rio_readn(connfd, &request_header, sizeof(request_header));

    switch (request_header.request_code)
    {
        case PUT:
            if ((request_header.key_size <= 0) || (request_header.value_size <= 0))
            {
                return 0;
            }
            key = Calloc(1, request_header.key_size);
            value = Calloc(1, request_header.value_size);
            Rio_readn(connfd, key, request_header.key_size);
            Rio_readn(connfd, value, request_header.value_size);
            handle_put(connfd, key, value);
            break;
        case GET:
            if (request_header.key_size <= 0)
            {
                return 0;
            }
            key = Calloc(1, request_header.key_size);
            Rio_readn(connfd, key, request_header.key_size);
            handle_get(connfd, key);
            break;
        case EVICT:
            if (request_header.key_size <= 0)
            {
                return 0;
            }
            key = Calloc(1, request_header.key_size);
            Rio_readn(connfd, key, request_header.key_size);
            handle_evict(connfd, key);
            break;
        case CLEAR:
            handle_clear(connfd);
            break;
        default:
            response_header = (response_header_t) {UNSUPPORTED, 0};
            Rio_writen(connfd, &response_header, sizeof(response_header));
            return 0;
    }

    return 0;
}

int handle_put(int connfd, void *key, void *value)
{
    map_key_t map_key;
    map_val_t map_value;
    response_header_t response_header;

    map_key = (map_key_t) {key, strlen(key)};
    map_value = (map_val_t) {value, strlen(value)};
    debug("Map_Key's key: %s", (char *) map_key.key_base);

    if(!put(hashmap, map_key, map_value, true))
    {
        response_header = (response_header_t) {BAD_REQUEST, 0};
        Rio_writen(connfd, &response_header, sizeof(response_header));
        Free(map_key.key_base);
        Free(map_value.val_base);
        debug("this is put");
        return 0;
    }

    response_header = (response_header_t) {OK, strlen(value)};
    Rio_writen(connfd, &response_header, sizeof(response_header));
    debug("this is put");
    return 0;
}
int handle_get(int connfd, void *key)
{
    map_key_t map_key;
    map_val_t map_value;
    response_header_t response_header;

    map_key = (map_key_t) {key, strlen(key)};
    debug("Map_Key's key: %s", (char *) map_key.key_base);
    map_value = get(hashmap, map_key);

    if((map_value.val_base) == NULL)
    {
        response_header = (response_header_t) {NOT_FOUND, 0};
        Rio_writen(connfd, &response_header, sizeof(response_header));
        Free(map_key.key_base);
        Free(map_value.val_base);
        debug("this is get");
        return 0;
    }

    response_header = (response_header_t) {OK, strlen(map_value.val_base)};
    Rio_writen(connfd, &(response_header), sizeof(response_header));
    Rio_writen(connfd, map_value.val_base, sizeof(map_value.val_base));
    debug("this is get");
    return 0;
}
int handle_evict(int connfd, void *key)
{
    map_key_t map_key;
    response_header_t response_header;
    map_node_t deleted_node;

    map_key = (map_key_t) {key, strlen(key)};
    if (((deleted_node = delete(hashmap, map_key)).key.key_base) != NULL)
    {
        free(deleted_node.key.key_base);
        free(deleted_node.val.val_base);
    }

    response_header = (response_header_t) {OK, 0};
    Rio_writen(connfd, &(response_header), sizeof(response_header));
    debug("this is evict");
    return 0;
}
int handle_clear(int connfd)
{
    response_header_t response_header;

    if(!clear_map(hashmap))
    {
        response_header = (response_header_t) {BAD_REQUEST, 0};
        Rio_writen(connfd, &response_header, sizeof(response_header));
        debug("this is clear");
        return 0;
    }

    response_header = (response_header_t) {OK, 0};
    Rio_writen(connfd, &(response_header), sizeof(response_header));
    debug("this is clear");
    return 0;
}

void *thread(void *vargp)
{
    int connfd;
    int *connfdp;

    /* Make threads live for the lifetime of the program */
    Pthread_detach(pthread_self());

    while (1)
    {
        /* Decrease # of fd */
        if (sem_wait(&(fds)) != 0) //down semaphore
        {
            perror("sem_wait failed");
            exit(EXIT_FAILURE);
        }
        /* Get file descriptor */
        connfdp = (int *) dequeue(queue);

        if (connfdp == NULL)
        {
            perror("queued item is invalid nah..");
            exit(EXIT_FAILURE);
        }
        connfd = *connfdp;
        /* Response */
        handle_request(connfd);
        /* Free */
        Free(connfdp);
        /* Close */
        Close(connfd);
    }

    return NULL;
}

/* Used in item destruction */
void map_free_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

/* Used in item destruction */
void queue_free_function(void *item) {
    free(item);
}
