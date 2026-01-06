#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef struct
{
    int* arena;
    int arena_length;
    int total_dog_count;
    int active_dog_count;
    int finished_dog_count;
    pthread_t* finished_dogs;
    pthread_t* tid_arr;
    pthread_mutex_t* arena_mtxes;
    pthread_mutex_t finish_line_mtx;
    int keep_running;
} SharedArgs;
void clean_this_shit_up(SharedArgs*);

void usage(char* name)
{
    printf("Usage %s\n", name);
    printf("\t-n gdzie n>20\n");
    printf("\t-m gdzie m>2\n");
}

void* thread_work(void* arg)
{
    SharedArgs* args = (SharedArgs*)arg;

    // wsadzasz psa
    pthread_mutex_lock(&args->arena_mtxes[0]);
    args->arena[0]++;
    pthread_mutex_unlock(&args->arena_mtxes[0]);

    int distance = 0;
    int position = 0;
    while (1)
    {
        int old_position = position;
        int time = 200 + (rand() % 1320);
        usleep(time);

        int jump = 1 + (rand() % 4);
        distance += jump;

        if (distance > args->arena_length)
            position = 2 * args->arena_length - distance;
        else
            position = distance;

        if (position <= 0)
        {
            pthread_mutex_lock(&args->finish_line_mtx);
            pthread_t self = pthread_self();
            args->finished_dogs[args->finished_dog_count] = self;
            args->finished_dog_count++;
            printf("[%lu] dotarlem do mety na pozycji: %d\n", self, args->finished_dog_count);
            pthread_mutex_unlock(&args->finish_line_mtx);
            break;
        }

        pthread_mutex_lock(&args->arena_mtxes[position]);
        if (args->arena[position] > 0)
            printf("waf waf waf\n");
        args->arena[position]++;
        pthread_mutex_unlock(&args->arena_mtxes[position]);
        pthread_mutex_lock(&args->arena_mtxes[old_position]);
        args->arena[old_position]--;
        pthread_mutex_unlock(&args->arena_mtxes[old_position]);
    }
    return NULL;
}

void* signal_thread_work(void* arg)
{
    SharedArgs* args = (SharedArgs*)arg;
    sigset_t new;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);

    int sig;

    sigwait(&new, &sig);
    switch (sig)
    {
        case SIGINT:
            printf("zawody nagle zakonczone, bo zaczelo padac\n");
            args->keep_running = 0;
            break;
    }
    return NULL;
}

void main_thread_work(SharedArgs* args, int n, int m)
{
    args->arena = (int*)malloc(sizeof(int) * (n + 1));
    args->arena_mtxes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * (n + 1));
    args->tid_arr = (pthread_t*)malloc(sizeof(pthread_t) * (m + 1));
    args->finished_dogs = (pthread_t*)malloc(sizeof(pthread_t) * m);
    args->total_dog_count = m;
    args->arena_length = n;
    args->keep_running = 1;
    args->finished_dog_count = 0;

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    for (int i = 0; i < n + 1; i++)
    {
        args->arena[i] = 0;
        pthread_mutex_t mtx;
        pthread_mutex_init(&mtx, NULL);
        args->arena_mtxes[i] = mtx;
    }
    pthread_mutex_init(&args->finish_line_mtx, NULL);

    pthread_t tid;
    for (int i = 0; i < m; i++)
    {
        pthread_create(&tid, NULL, thread_work, args);
        args->tid_arr[i] = tid;
    }
    // tworze na sygnaly thread
    pthread_create(&tid, NULL, signal_thread_work, args);
    args->tid_arr[m] = tid;

    struct timespec ts = {1, 0};
    while (args->keep_running)
    {
        nanosleep(&ts, NULL);
        pthread_mutex_lock(&args->finish_line_mtx);
        if (args->finished_dog_count == args->total_dog_count)
        {
            printf("Zawody zakonczone, top 3 psy:\n");
            for (int i = 0; i < 3; i++)
            {
                printf("%d   -   %lu\n", i + 1, args->finished_dogs[i]);
            }
            pthread_mutex_unlock(&args->finish_line_mtx);
            break;
        }
    }
    clean_this_shit_up(args);
}

void clean_this_shit_up(SharedArgs* args)
{
    pthread_cancel(args->tid_arr[args->total_dog_count]);

    for (int i = 0; i <= args->total_dog_count; i++)
    {
        pthread_join(args->tid_arr[i], NULL);
    }

    for (int i = 0; i <= args->arena_length; i++)
    {
        pthread_mutex_destroy(&args->arena_mtxes[i]);
    }

    free(args->arena);
    free(args->finished_dogs);
    free(args->arena_mtxes);
    free(args->tid_arr);
    pthread_mutex_destroy(&args->finish_line_mtx);
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    if (argc != 3)
        usage(argv[0]);

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    SharedArgs args;
    main_thread_work(&args, n, m);
    return EXIT_SUCCESS;
}
