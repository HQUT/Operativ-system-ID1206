#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 3
#define TOTAL_MODIFICATIONS 15

int buffer = 0;
pthread_mutex_t buffer_mutex;

void *threadFunction(void *arg) {
    int count = 0;
    while (1) {
        pthread_mutex_lock(&buffer_mutex);

        if (buffer >= TOTAL_MODIFICATIONS) {
            pthread_mutex_unlock(&buffer_mutex);
            break;
        }

        printf("TID: %lu, PID: %d, Buffer: %d\n", (unsigned long)pthread_self(), getpid(), buffer);
        buffer++;
        count++;

        pthread_mutex_unlock(&buffer_mutex);
        usleep(100000); 
    }
    return (void *)(intptr_t)count;
}

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&buffer_mutex, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, threadFunction, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    int totalAccesses = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        void *count;
        pthread_join(threads[i], &count);
        printf("TID %lu worked on the buffer %ld times\n", (unsigned long)threads[i], (intptr_t)count);
        totalAccesses += (intptr_t)count;
    }

    printf("Total buffer accesses: %d\n", totalAccesses);

    pthread_mutex_destroy(&buffer_mutex);
    return 0;
}
