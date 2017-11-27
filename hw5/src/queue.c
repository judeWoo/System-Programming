#include "queue.h"
#include <errno.h>
#include <stdio.h>

queue_t *create_queue(void) {
    queue_t *queue;

    if ((queue = (queue_t *) calloc(1, sizeof(queue_t))) == NULL)
    {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL; //setting front & rear to NULL
    if (sem_init(&(queue->items), 0, 0) == -1) //setting # of items to 0
    {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&(queue->lock), NULL) != 0) //setting mutex to NULL
    {
        perror("pthread_mutex_init failed");
        exit(EXIT_FAILURE);
    }
    queue->invalid = false;

    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    queue_node_t *indicator;

    if (self == NULL || destroy_function == NULL)
    {
        errno = EINVAL;
        return(false);
    }

    if (self->front == NULL || self->rear == NULL) //empty queue
    {
        self->invalid = true;
        return(true);
    }

    if (pthread_mutex_lock(&(self->lock)) != 0) //start critical region
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    indicator = self->front;

    if (indicator->next == NULL) //if only one node in queue
    {
        // if (sem_wait(&(self->items)) != 0) //down semaphore
        // {
        //     perror("sem_wait failed");
        //     exit(EXIT_FAILURE);
        // }
        destroy_function(indicator->item);
    }

    else
    {
        while (indicator->next != NULL) //if multiple nodes in queue
        {
            // if (sem_wait(&(self->items)) != 0) //down semaphore
            // {
            //     perror("sem_wait failed");
            //     exit(EXIT_FAILURE);
            // }
            destroy_function(indicator->item);

            indicator = indicator->next;

            if (indicator == NULL) //if last node in queue
            {
                // if (sem_wait(&(self->items)) != 0) //down semaphore
                // {
                //     perror("sem_wait failed");
                //     exit(EXIT_FAILURE);
                // }
                destroy_function(indicator->item);
                break;
            }
        }
    }

    self->invalid = true;

    if (pthread_mutex_unlock(&(self->lock)) != 0) //end critical region
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    return true;
}

bool enqueue(queue_t *self, void *item) {
    queue_node_t *indicator;
    queue_node_t *next_indicator;

    if (self == NULL || item == NULL)
    {
        errno = EINVAL;
        return(false);
    }

    if (pthread_mutex_lock(&(self->lock)) != 0) //start critical region
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    indicator = self->front;

    if (indicator == NULL) //nothing in the queue
    {
        if ((indicator = (queue_node_t *) calloc(1, sizeof(queue_node_t))) == NULL)
        {
            perror("calloc failed");
            exit(EXIT_FAILURE);
        }
        indicator->item = item;
        indicator->next = NULL;
        self->rear = indicator; //front == rear
        self->front = indicator; //front == rear
    }

    else //something in the queue
    {
        if (indicator->next == NULL) //if only one node in queue
        {
            next_indicator = indicator->next; //TODO
            if ((next_indicator = calloc(1, sizeof(queue_node_t))) == NULL)
            {
                perror("calloc failed");
                exit(EXIT_FAILURE);
            }
            next_indicator->item = item;
            next_indicator->next = NULL;
            self->rear = next_indicator; //update rear
        }

        else //if multiple node in queue
        {
            next_indicator = indicator->next;
            while (next_indicator != NULL)
            {
                next_indicator = next_indicator->next;
                if (next_indicator == NULL)
                {
                    if ((next_indicator = calloc(1, sizeof(queue_node_t))) == NULL)
                    {
                        perror("calloc failed");
                        exit(EXIT_FAILURE);
                    }
                    next_indicator->item = item;
                    next_indicator->next = NULL;
                    self->rear = next_indicator; //update rear
                    break;
                }
            }
        }
    }

    if (pthread_mutex_unlock(&(self->lock)) != 0) //end critical region
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    if (sem_post(&(self->items)) != 0) //up semaphore
    {
        perror("sem_post failed");
        exit(EXIT_FAILURE);
    }

    return true;
}

void *dequeue(queue_t *self) {
    queue_node_t *indicator;
    void *item;

    if (self == NULL)
    {
        errno = EINVAL;
        return(NULL);
    }

    if (self->front == NULL || self->rear == NULL) //empty queue
    {
        errno = EINVAL;
        return(NULL);
    }

    if (sem_wait(&(self->items)) != 0) //down semaphore
    {
        perror("sem_wait failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_lock(&(self->lock)) != 0) //start critical region
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    indicator = self->front;
    item = indicator->item;
    self->front = indicator->next; //TODO
    free(indicator);

    if (pthread_mutex_unlock(&(self->lock)) != 0) //end critical region
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }

    return item;
}
