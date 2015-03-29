#ifndef WINDOWS
#include <unistd.h>
#endif
#include "thread.h"

ThreadT Thread::create(void * prg, void * args)
{
#ifdef WINDOWS
    DWORD thr;
    return ((ThreadT)CreateThread(0, 0, (LPTHREAD_START_ROUTINE)prg, args, 0, &thr));
#else
    pthread_t t;
    pthread_create(&t, NULL, prg, args);
    return (ThreadT)t;
#endif
}
void Thread::join(ThreadT handle)
{
#ifdef WINDOWS
    WaitForSingleObject((HANDLE)handle, INFINITE);
#else
    int *p;
    pthread_join((pthread_t)handle, (void**)&p);
#endif
}
void Thread::close(ThreadT handle)
{
#ifdef WINDOWS
    CloseHandle((HANDLE)handle);
#else
    pthread_exit(handle);
#endif
}
void Thread::sleep(unsigned int milliseconds)
{
#ifdef WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds);
#endif
}
