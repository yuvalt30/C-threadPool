// Yuval Tal 311127120

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include "pthread.h"
#include <stdlib.h>
#include <stdio.h>

typedef void (*func) (void *);

typedef struct Task
{
    void (*computeFunc) (void *);
    void* param;
}Task;

typedef struct thread_pool
{
    pthread_t* tid;
    int n;
    OSQueue* q;
    int isBeingDestroyed;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int shouldStop;
}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
