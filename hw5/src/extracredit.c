#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    hashmap_t *myhashmap;

    if ((capacity <= 0) || (hash_function == NULL) || (destroy_function == NULL))
    {
        errno = EINVAL;
        return NULL;
    }

    if ((myhashmap = (hashmap_t *) calloc(1, sizeof(hashmap_t))) == NULL)
    {
        return NULL;
    }
    if ((myhashmap->nodes = (map_node_t *) calloc(capacity, sizeof(map_node_t))) == NULL)
    {
        return NULL;
    }

    myhashmap->capacity = capacity;
    myhashmap->size = 0;
    myhashmap->hash_function = hash_function;
    myhashmap->destroy_function = destroy_function;
    myhashmap->num_readers = 0;
    if (pthread_mutex_init(&(myhashmap->write_lock), NULL) != 0) //setting mutex to NULL
    {
        errno = EINVAL;
        return NULL;
    }
    if (pthread_mutex_init(&(myhashmap->fields_lock), NULL) != 0) //setting mutex to NULL
    {
        errno = EINVAL;
        return NULL;
    }
    myhashmap->invalid = false;

    return myhashmap;
}
/* This is Writer */
bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    time_t current_time;
    int index; //mapped index
    int index_finder;
    int old_index; //least used node
    int new_index; //traverser
    int minimum; //least used node's number

    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    if ((key.key_base == NULL) || (val.val_base == NULL) || (key.key_len <= 0) || (val.val_len <= 0))
    {
        errno = EINVAL;
        return false;
    }

    if ((self->capacity <= 0) || (self->size < 0) ||(self->nodes == NULL) || (self->hash_function == NULL))
    {
        errno = EINVAL;
        return false;
    }

    if ((self->num_readers < 0) || (self->invalid == true) || (self->destroy_function == NULL))
    {
        errno = EINVAL;
        return false;
    }

    if ((self->size == self->capacity) && force == false) //map is full & force is false
    {
        errno = ENOMEM;
        return false;
    }

    if (pthread_mutex_lock(&(self->write_lock)) != 0)
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    /* Critical section starts */
    /* Writing starts */
    index = get_index(self, key);
    if (self->size == 0) //if map is empty
    {
        self->nodes[index].key = key; //put elements
        self->nodes[index].val = val;
        self->nodes[index].tombstone = false;
        self->nodes[index].num_used = 1; //lru updated to 1 (default)
        if(time(&(self->nodes[index].created_time)) == -1) //ttl updated to current time (default)
        {
            perror("time() failed");
            exit(EXIT_FAILURE);
        }
        self->size++;
        /* Writing ends */
        /* Critical section ends */
        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        return true;
    }

    else if ((self->size == self->capacity) && force == true) //if map is full but forced
    {
        /* Init index */
        if(time(&current_time) == -1) //ttl updated to current time (default)
        {
            perror("time() failed");
            exit(EXIT_FAILURE);
        }
        index_finder = index;
        while (1)
        {
            if (difftime(current_time, self->nodes[index_finder].created_time) >= TTL)
            {
                old_index = index_finder; //if found, insert
                goto insert;
            }

            index_finder++;

            if (index_finder == self->capacity)
            {
                index_finder = 0; //go back from the start
            }

            if (index_finder == index)
            {
                index = index_finder;
                break;
            }
        }
        /* Init others */
        new_index = index;
        old_index = index;
        minimum = self->nodes[new_index].num_used;
        /* Traverse the entire hashmap */
        while (1)
        {
            if (self->nodes[new_index].num_used < minimum) //minimum found
            {
                minimum = self->nodes[new_index].num_used;
                old_index = new_index;
            }

            new_index++; //minimum not found

            if (new_index >= self->capacity)
            {
                new_index = 0; //start from beginning
            }

            if (new_index == index) //if came back to start
            {
                insert:
                self->nodes[old_index].key = key; //put elements
                self->nodes[old_index].val = val;
                self->nodes[old_index].tombstone = false;
                self->nodes[old_index].num_used = 1; //lru updated to 1 (default)
                if(time(&(self->nodes[index].created_time)) == -1) //ttl updated to current time (default)
                {
                    perror("time() failed");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
        /* Writing ends */
        /* Critical section ends */
        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        return true;
    }

    else //map is not full
    {
        if (self->nodes[index].key.key_base == NULL) //target node is empty
        {
            if (self->nodes[index].tombstone == true) //if tombstoned
            {
                self->destroy_function(self->nodes[index].key, self->nodes[index].val);
            }
            self->nodes[index].key = key; //put elements
            self->nodes[index].val = val;
            self->nodes[index].tombstone = false;
            self->nodes[index].num_used = 1; //lru updated to 1 (default)
            if(time(&(self->nodes[index].created_time)) == -1) //ttl updated to current time (default)
            {
                perror("time() failed");
                exit(EXIT_FAILURE);
            }
            self->size++;
            if (pthread_mutex_unlock(&(self->write_lock)) != 0)
            {
                perror("pthread_mutex_unlock failed");
                exit(EXIT_FAILURE);
            }
            /* Writing ends */
            /* Critical section ends */
            return true;
        }

        while (self->nodes[index].key.key_base != NULL) //target node is occupied
        {
            if (key.key_len == self->nodes[index].key.key_len) //compare keys
            {
                if (memcmp(key.key_base, self->nodes[index].key.key_base, key.key_len) == 0) //key exists
                {
                    if (self->nodes[index].tombstone == true) //if tombstoned
                    {
                        self->destroy_function(self->nodes[index].key, self->nodes[index].val);
                    }
                    self->nodes[index].key = key; //put elements
                    self->nodes[index].val = val;
                    self->nodes[index].tombstone = false;
                    self->nodes[index].num_used = 1; //lru updated to 1 (default)
                    if(time(&(self->nodes[index].created_time)) == -1) //ttl updated to current time (default)
                    {
                        perror("time() failed");
                        exit(EXIT_FAILURE);
                    }
                    if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                    /* Writing ends */
                    /* Critical section ends */
                    return true;
                }
            }
            index++;
            if (index == self->capacity)
            {
                index = 0; //go back from the start
            }

            if (self->nodes[index].key.key_base == NULL) //found empty node
            {
                if (self->nodes[index].tombstone == true) //if tombstoned
                {
                    self->destroy_function(self->nodes[index].key, self->nodes[index].val);
                }
                self->nodes[index].key = key; //put elements
                self->nodes[index].val = val;
                self->nodes[index].tombstone = false;
                self->nodes[index].num_used = 1; //lru updated to 1 (default)
                if(time(&(self->nodes[index].created_time)) == -1) //ttl updated to current time (default)
                {
                    perror("time() failed");
                    exit(EXIT_FAILURE);
                }
                self->size++;
                if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                {
                    perror("pthread_mutex_unlock failed");
                    exit(EXIT_FAILURE);
                }
                /* Writing ends */
                /* Critical section ends */
                return true;
            }
        }
    }
    /* Cannot Come Here */
    return false;
}
/* This is Reader */
map_val_t get(hashmap_t *self, map_key_t key) {
    time_t current_time;
    int index;
    int new_index;

    if ((self == NULL) || (key.key_base == NULL) || (key.key_len <= 0))
    {
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    if ((self->capacity <= 0) || (self->size < 0) ||(self->nodes == NULL) || (self->hash_function == NULL))
    {
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    if ((self->num_readers < 0) || (self->invalid == true) || (self->destroy_function == NULL))
    {
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    if (pthread_mutex_lock(&(self->fields_lock)) != 0)
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    self->num_readers++; //Up the readers
    if (self->num_readers == 1)
    {
        if (pthread_mutex_lock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_lock failed");
            exit(EXIT_FAILURE);
        }
    }
    if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
    {
        perror("pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }
    /* Critical section starts */
    /* Reading starts */
    index = get_index(self, key);

    if ((self->nodes[index].key.key_base == NULL) || (self->nodes[index].key.key_len <= 0)) //key not found
    {
        if (self->nodes[index].tombstone == true) //if tombstoned
        {
            goto search;
        }

        if (pthread_mutex_lock(&(self->fields_lock)) != 0)
        {
            perror("pthread_mutex_lock failed");
            exit(EXIT_FAILURE);
        }
        self->num_readers--; //Down the readers
        if (self->num_readers == 0)
        {
            if (pthread_mutex_unlock(&(self->write_lock)) != 0)
            {
                perror("pthread_mutex_unlock failed");
                exit(EXIT_FAILURE);
            }
        }
        if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        /* Reading ends */
        /* Critical section ends */
        return MAP_VAL(NULL, 0);
    }

    else //target node is occupied
    {
        if (key.key_len == self->nodes[index].key.key_len) //if keys have same length
        {
            if (memcmp(key.key_base, self->nodes[index].key.key_base, key.key_len) == 0) //key found
            {
                if (self->nodes[index].tombstone == true) //if tombstoned
                {
                    goto search;
                }

                if (pthread_mutex_lock(&(self->fields_lock)) != 0)
                {
                    perror("pthread_mutex_lock failed");
                    exit(EXIT_FAILURE);
                }
                if(time(&current_time) == -1) //ttl updated to current time (default)
                {
                    perror("time() failed");
                    exit(EXIT_FAILURE);
                }
                if (difftime(current_time, self->nodes[index].created_time) >= TTL) //if expired
                {
                    self->nodes[index].key.key_base = NULL; //delete key
                    self->nodes[index].key.key_len = 0;
                    self->nodes[index].val.val_base = NULL;
                    self->nodes[index].val.val_len = 0;
                    self->nodes[index].tombstone = true;
                    self->size--;
                    self->num_readers--; //Down the readers
                    if (self->num_readers == 0)
                    {
                        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                        {
                            perror("pthread_mutex_unlock failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                    /* Reading ends */
                    /* Critical section ends */
                    return MAP_VAL(NULL, 0);
                }
                self->nodes[index].num_used++; //increase usage
                self->num_readers--; //Down the readers
                if (self->num_readers == 0)
                {
                    if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                }
                if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
                {
                    perror("pthread_mutex_unlock failed");
                    exit(EXIT_FAILURE);
                }
                /* Reading ends */
                /* Critical section ends */
                return self->nodes[index].val;
            }

            else
            {
                goto search;
            }
        }

        else //key is different
        {
            /* linear probing */
            search:
            new_index = index;
            while (1)
            {
                new_index++;
                if (new_index == self->capacity)
                {
                    new_index = 0; //go back from the start
                }
                if (new_index == index) //key not found
                {
                    if (pthread_mutex_lock(&(self->fields_lock)) != 0)
                    {
                        perror("pthread_mutex_lock failed");
                        exit(EXIT_FAILURE);
                    }
                    self->num_readers--; //Down the readers
                    if (self->num_readers == 0)
                    {
                        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                        {
                            perror("pthread_mutex_unlock failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                    /* Reading ends */
                    /* Critical section ends */
                    return MAP_VAL(NULL, 0);
                }
                if (self->nodes[new_index].key.key_base == NULL) //key not found
                {
                    if (self->nodes[new_index].tombstone == true) //if tombstoned
                    {
                        continue;
                    }
                    if (pthread_mutex_lock(&(self->fields_lock)) != 0)
                    {
                        perror("pthread_mutex_lock failed");
                        exit(EXIT_FAILURE);
                    }
                    self->num_readers--; //Down the readers
                    if (self->num_readers == 0)
                    {
                        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                        {
                            perror("pthread_mutex_unlock failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                    /* Reading ends */
                    /* Critical section ends */
                    return MAP_VAL(NULL, 0);
                }
                if (key.key_len == self->nodes[new_index].key.key_len) //if keys have same length
                {
                    if (memcmp(key.key_base, self->nodes[new_index].key.key_base, key.key_len) == 0) //key found
                    {
                        if (self->nodes[new_index].tombstone == true) //if tombstoned
                        {
                            continue;
                        }
                        if (pthread_mutex_lock(&(self->fields_lock)) != 0)
                        {
                            perror("pthread_mutex_lock failed");
                            exit(EXIT_FAILURE);
                        }
                        if (time(&current_time) == -1) //get the current time
                        {
                            perror("time() failed");
                            exit(EXIT_FAILURE);
                        }
                        if (difftime(current_time, self->nodes[index].created_time) >= TTL) //if expired
                        {
                            self->nodes[index].key.key_base = NULL; //delete key
                            self->nodes[index].key.key_len = 0;
                            self->nodes[index].val.val_base = NULL;
                            self->nodes[index].val.val_len = 0;
                            self->nodes[index].tombstone = true;
                            self->size--;
                            self->num_readers--; //Down the readers
                            if (self->num_readers == 0)
                            {
                                if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                                {
                                    perror("pthread_mutex_unlock failed");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
                            {
                                perror("pthread_mutex_unlock failed");
                                exit(EXIT_FAILURE);
                            }
                            /* Reading ends */
                            /* Critical section ends */
                            return MAP_VAL(NULL, 0);
                        }
                        self->nodes[index].num_used++; //increase usage
                        self->num_readers--; //Down the readers
                        if (self->num_readers == 0)
                        {
                            if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                            {
                                perror("pthread_mutex_unlock failed");
                                exit(EXIT_FAILURE);
                            }
                        }
                        if (pthread_mutex_unlock(&(self->fields_lock)) != 0)
                        {
                            perror("pthread_mutex_unlock failed");
                            exit(EXIT_FAILURE);
                        }
                        /* Reading ends */
                        /* Critical section ends */
                        return self->nodes[new_index].val;
                    }
                }
            }
        }
    }
    /* Cannot Come Here */
    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    int index;
    int new_index;

    if ((self == NULL) || (key.key_base == NULL) || (key.key_len  <= 0))
    {
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    if ((self->capacity <= 0) || (self->size < 0) ||(self->nodes == NULL) || (self->hash_function == NULL))
    {
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    if ((self->num_readers < 0) || (self->invalid == true) || (self->destroy_function == NULL))
    {
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    if (pthread_mutex_lock(&(self->write_lock)) != 0)
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    /* Critical section starts */
    /* Writing starts */
    index = get_index(self, key);

    if ((self->nodes[index].key.key_base == NULL) || (self->nodes[index].key.key_len <= 0)) //key not found
    {
        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        /* Writing ends */
        /* Critical section ends */
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    else //target node is occupied
    {
        if (key.key_len == self->nodes[index].key.key_len) //if keys have same length
        {
            if (memcmp(key.key_base, self->nodes[index].key.key_base, key.key_len) == 0) //key exists
            {
                if (self->nodes[index].tombstone == true) //if tombstoned
                {
                    goto search;
                }
                self->nodes[index].key.key_base = NULL; //delete key
                self->nodes[index].key.key_len = 0;
                self->nodes[index].val.val_base = NULL;
                self->nodes[index].val.val_len = 0;
                self->nodes[index].tombstone = true;
                self->size--;
                if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                {
                    perror("pthread_mutex_unlock failed");
                    exit(EXIT_FAILURE);
                }
                /* Writing ends */
                /* Critical section ends */
                return self->nodes[index];
            }

            else
            {
                goto search;
            }
        }

        else //key is different
        {
            /* linear probing */
            search:
            new_index = index;
            while (1)
            {
                new_index++;
                if (new_index == self->capacity)
                {
                    new_index = 0; //go back from the start
                }
                if (new_index == index) //key not found
                {
                    if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                    /* Writing ends */
                    /* Critical section ends */
                    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
                }
                if (self->nodes[new_index].key.key_base == NULL) //key not found
                {
                    if (self->nodes[new_index].tombstone == true) //if tombstoned
                    {
                        continue;
                    }
                    if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                    {
                        perror("pthread_mutex_unlock failed");
                        exit(EXIT_FAILURE);
                    }
                    /* Writing ends */
                    /* Critical section ends */
                    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
                }
                if (key.key_len == self->nodes[new_index].key.key_len) //if keys have same length
                {
                    if (memcmp(key.key_base, self->nodes[new_index].key.key_base, key.key_len) == 0) //key found
                    {
                        if (self->nodes[new_index].tombstone == true) //if tombstoned
                        {
                            continue;
                        }
                        self->nodes[index].key.key_base = NULL; //delete key
                        self->nodes[index].key.key_len = 0;
                        self->nodes[index].val.val_base = NULL;
                        self->nodes[index].val.val_len = 0;
                        self->nodes[index].tombstone = true;
                        self->size--;
                        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
                        {
                            perror("pthread_mutex_unlock failed");
                            exit(EXIT_FAILURE);
                        }
                        /* Reading ends */
                        /* Critical section ends */
                        return self->nodes[new_index];
                    }
                }
            }
        }
    }
    /* Cannot Get Here */
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}
/* This is Writer */
bool clear_map(hashmap_t *self) {
    int indicator;

    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    if ((self->capacity <= 0) || (self->size < 0) ||(self->nodes == NULL) || (self->hash_function == NULL))
    {
        errno = EINVAL;
        return false;
    }

    if ((self->num_readers < 0) || (self->invalid == true) || (self->destroy_function == NULL))
    {
        errno = EINVAL;
        return false;
    }

    if (self->size == 0)
    {
        return true;
    }

    if (pthread_mutex_lock(&(self->write_lock)) != 0)
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    /* Critical section starts */
    /* Writing starts */
    indicator = 0;
    for (int i = 0; i < self->capacity; ++i)
    {
        /* Clear */
        if ((self->nodes[i].key.key_base != NULL) || (self->nodes[i].key.key_len > 0)) //not empty node
        {
            self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            indicator++;
        }
    }

    if (indicator == self->size)
    {
        self->size = 0;
        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        /* Writing ends */
        /* Critical section ends */
        return true;
    }
    if (pthread_mutex_unlock(&(self->write_lock)) != 0)
    {
        perror("pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }
    /* Writing ends */
    /* Critical section ends */
    return false;
}
/* This is Writer */
bool invalidate_map(hashmap_t *self) {
    int indicator;

    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    if ((self->capacity <= 0) || (self->size < 0) ||(self->nodes == NULL) || (self->hash_function == NULL))
    {
        errno = EINVAL;
        return false;
    }

    if ((self->num_readers < 0) || (self->invalid == true) || (self->destroy_function == NULL))
    {
        errno = EINVAL;
        return false;
    }

    if (pthread_mutex_lock(&(self->write_lock)) != 0)
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    /* Critical section starts */
    /* Writing starts */
    if (self->size == 0)
    {
        free(self->nodes);
        self->invalid = true;
        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        /* Writing ends */
        /* Critical section ends */
        return true;
    }

    indicator = 0;
    for (int i = 0; i < self->capacity; ++i)
    {
        /* Clear */
        if ((self->nodes[i].key.key_base != NULL) || (self->nodes[i].key.key_len > 0)) //not empty node
        {
            self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            indicator++;
        }
    }

    if (indicator == self->size)
    {
        self->size = 0;
        free(self->nodes);
        self->invalid = true;
        if (pthread_mutex_unlock(&(self->write_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
        /* Writing ends */
        /* Critical section ends */
        return true;
    }
    if (pthread_mutex_unlock(&(self->write_lock)) != 0)
    {
        perror("pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }
    /* Writing ends */
    /* Critical section ends */
    return false;
}
