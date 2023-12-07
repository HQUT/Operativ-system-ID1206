#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define MAX 5

struct SharedData {
    int var;
    int read_count;
    sem_t rw_mutex;
    sem_t mutex;
};

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void reader(int shmid, int reader_id) {
    struct SharedData *shared = (struct SharedData *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) error_exit("shmat");

    for (int i = 0; i < MAX; ++i) {
        sem_wait(&shared->mutex);
        shared->read_count++;
        if (shared->read_count == 1) {
            sem_wait(&shared->rw_mutex);
            printf("Den första läsaren skaffar låset.\n");
        }
        sem_post(&shared->mutex);

        printf("Läsare (%d) läser värdet %d\n", reader_id, shared->var);

        usleep(100000);

        sem_wait(&shared->mutex);
        shared->read_count--;
        if (shared->read_count == 0) {
            printf("Den sista läsaren släpper låset.\n");
            sem_post(&shared->rw_mutex);
        }
        sem_post(&shared->mutex);

        sleep(1);
    }

    shmdt(shared);
}

void writer(int shmid) {
    struct SharedData *shared = (struct SharedData *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) error_exit("shmat");

    for (int i = 0; i < MAX; ++i) {
        sem_wait(&shared->rw_mutex);
        printf("Skrivaren skaffar låset.\n");
        shared->var++;
        printf("Skrivaren (%d) skriver värdet %d\n", getpid(), shared->var);
        printf("Skrivaren släpper låset.\n");
        sem_post(&shared->rw_mutex);

        sleep(1);
    }

    shmdt(shared);
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (shmid == -1) error_exit("shmget");

    struct SharedData *shared = (struct SharedData *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) error_exit("shmat");

    shared->var = 0;
    shared->read_count = 0;
    sem_init(&shared->rw_mutex, 1, 1);
    sem_init(&shared->mutex, 1, 1);

    pid_t pid = fork();
    if (pid == -1) error_exit("fork");
    if (pid == 0) {
        reader(shmid, 124);
        exit(0);
    }

    pid = fork();
    if (pid == -1) error_exit("fork");
    if (pid == 0) {
        reader(shmid, 125);
        exit(0);
    }

    writer(shmid);

    while (wait(NULL) > 0);

    sem_destroy(&shared->mutex);
    sem_destroy(&shared->rw_mutex);

    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
