#include "queue.h"

queue_t *create_queue(void) {
    queue_t *queue;

    if (queue = calloc(1, sizeof(queue_t)) == NULL)
    {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    if ((sem_init(queue->items, 0, 0)) == -1)      //# of items 0
    {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(queue->lock, NULL) != 0)
    {
        perror("pthread_mutex_init failed");
        exit(EXIT_FAILURE);
    }
    // queue->invalid = false;

    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    return false;
}

bool enqueue(queue_t *self, void *item) {
    return false;
}

void *dequeue(queue_t *self) {
    return NULL;
}
