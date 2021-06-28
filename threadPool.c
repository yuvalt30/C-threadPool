// Yuval Tal 311127120
#define DESTROY_NO_WAIT 2
#define DESTROY_WAIT 1
#include "threadPool.h"

void* runTasks(void* tpp){
    ThreadPool* tp = (ThreadPool*)tpp;
    Task* task;
    while(1){
        pthread_mutex_lock(&tp->mutex); // lock
        while (osIsQueueEmpty(tp->q))
        {
            pthread_cond_wait(&tp->cond, &tp->mutex);
        }
        task = (Task *)osDequeue(tp->q);
        pthread_mutex_unlock(&tp->mutex); // unlock
        task->computeFunc(task->param); // execute task
        if(task)
            free(task);
    }
}

void destruction(void *arg)
{
    ThreadPool *tp = (ThreadPool*)arg;
    pthread_cond_signal(&tp->cond);
    // printf("thread's destruction\n");
    pthread_exit(NULL);
}

void insertNDestructions(ThreadPool *tp, Task* destruction){
    int i=0;
    for(i=0; i<tp->n; ++i){
        osEnqueue(tp->q, destruction);
    }
}

void emptyQueue(ThreadPool* tp){
    Task* temp;
    while(!osIsQueueEmpty(tp->q)){
        temp = (Task*)osDequeue(tp->q);
        if(temp)
            free(temp);
    }
}

void sigAll(ThreadPool *tp){
    int i=0;
    for(i=0; i<tp->n; ++i){
        pthread_cond_signal(&tp->cond);
    }
}

void joinAll(ThreadPool* tp){
    int i=0;
    for (i = 0; i < tp->n; ++i) {
        pthread_join(tp->tid[i], NULL);
    }
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks)
{
    threadPool->isBeingDestroyed = 1;
    Task* dt = (Task*)malloc(sizeof(Task));
    dt->computeFunc = destruction;
    dt->param = (void*)threadPool;

    pthread_mutex_lock(&threadPool->mutex); // lock
    if (!shouldWaitForTasks)
        emptyQueue(threadPool);

    insertNDestructions(threadPool, dt);

    pthread_mutex_unlock(&threadPool->mutex); // unlock
    pthread_cond_signal(&threadPool->cond);

    while (!osIsQueueEmpty(threadPool->q)){
        pthread_cond_wait(&threadPool->cond, &threadPool->mutex);
    }
    joinAll(threadPool);
    osDestroyQueue(threadPool->q);
    pthread_mutex_destroy(&threadPool->mutex); // mutex
    pthread_cond_destroy(&threadPool->cond); // cond
    free(threadPool->tid); // p_thread[]
    free(threadPool);
    free(dt); // destruction task
}

ThreadPool *tpCreate(int numOfThreads)
{
    ThreadPool *tp = (ThreadPool *)malloc(sizeof(ThreadPool));
    tp->shouldStop = 0;
    tp->q = osCreateQueue();
    tp->n = numOfThreads;
    tp->isBeingDestroyed = 0;
    tp->tid = (pthread_t *)malloc(numOfThreads * sizeof(pthread_t));
    int i;
    pthread_cond_init(&tp->cond, NULL);
    pthread_mutex_init(&tp->mutex, NULL);
    for (i = 0; i < numOfThreads; ++i)
        if (pthread_create(&tp->tid[i], NULL, runTasks, (void *)tp))
            perror("pthread_create fail\n");

    return tp;
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param)
{

    if (threadPool->isBeingDestroyed)
        return -1;
    Task *t = (Task *)malloc(sizeof(Task));
    t->computeFunc = computeFunc;
    t->param = param;

    pthread_mutex_lock(&threadPool->mutex);
    osEnqueue(threadPool->q, t);
    pthread_mutex_unlock(&threadPool->mutex);
    pthread_cond_signal(&threadPool->cond);
    return 0;
}