#ifndef THREAD_H
#define THREAD_H

#ifdef WINDOWS
#include <Windows.h>
typedef void * ThreadT;
#else
#include <pthread.h>
typedef pthread_t ThreadT;
#endif
namespace Thread
{
    ThreadT create(void * prg, void * args);
    void join(ThreadT handle);
    void close(ThreadT handle);
    void sleep(unsigned int milliseconds);
}

#endif // THREAD_H
