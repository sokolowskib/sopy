#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
volatile sig_atomic_t keep_running = 1;
typedef struct
{
    int l;
    pthread_t* tid_arr;
    bool* tid_checked;
    int count;
    pthread_mutex_t mtx;
    int run;
} ProgramArgs;

int find_tid_in_array(ProgramArgs* args, pthread_t tid)
{
    for (int i = 0; i < args->count; i++)
    {
        if (args->tid_arr[i] == tid)
            return i;
    }
    return -1;
}

void* thread_work(void* void_args)
{
    ProgramArgs* args = (ProgramArgs*)void_args;
    int M = 2 + (rand() % 98);
    pthread_t tid = pthread_self();
    while (args->run)
    {
        pthread_mutex_lock(&args->mtx);

        int remainder = args->l % M;
        bool is_zero = false;
        if (remainder == 0)
            is_zero = true;
        int index = find_tid_in_array(args, tid);
        if (index == -1)
            ERR("thread not in tid array");

        if (args->tid_checked[index] == false)
        {
            printf("[%lu] czy moje M(%d) dzieli L(%d): %d\n", pthread_self(), M, args->l, is_zero);
            args->tid_checked[index] = true;
        }

        pthread_mutex_unlock(&args->mtx);
    }

    return NULL;
}

void create_threads(ProgramArgs* args, int n)
{
    args->tid_arr = (pthread_t*)malloc(sizeof(pthread_t) * n);
    args->tid_checked = (bool*)malloc(sizeof(bool) * n);
    args->run = 1;
    args->count = n;
    pthread_t tid;
    for (int i = 0; i < n; i++)
    {
        if (pthread_create(&tid, NULL, thread_work, args) != 0)
            ERR("pthread create");
        args->tid_arr[i] = tid;
        args->tid_checked[i] = false;
    }
}

void handle_sigint(int n) { keep_running = 0; }

void main_loop(ProgramArgs* args)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    if (-1 == sigaction(SIGINT, &sa, NULL))
    {
        ERR("sigaction");
    }

    struct timespec ts = {0, 100000000};
    args->l = 0;
    while (keep_running)
    {
        nanosleep(&ts, NULL);
        pthread_mutex_lock(&args->mtx);

        args->l++;
        for (int i = 0; i < args->count; i++)
        {
            args->tid_checked[i] = false;
        }
        pthread_mutex_unlock(&args->mtx);

        bool should_increment = false;
        while (!should_increment && keep_running)
        {
            should_increment = true;
            pthread_mutex_lock(&args->mtx);
            for (int i = 0; i < args->count; i++)
            {
                if (args->tid_checked[i] == false)
                    should_increment = false;
            }
            pthread_mutex_unlock(&args->mtx);
        }
    }
    args->run = 0;
}

void join_threads(ProgramArgs* args, int n)
{
    for (int i = 0; i < n; i++)
    {
        pthread_join(args->tid_arr[i], NULL);
    }
    free(args->tid_arr);
    free(args->tid_checked);
}

void usage(char* name)
{
    printf("Usage %s:\n", name);
    printf("\t -n ilosc watkow do utworzenia");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        usage(argv[0]);
    }
    int n = atoi(argv[1]);
    ProgramArgs args;
    pthread_mutex_init(&args.mtx, NULL);
    create_threads(&args, n);
    main_loop(&args);
    join_threads(&args, n);

    pthread_mutex_destroy(&args.mtx);

    return EXIT_SUCCESS;
}
