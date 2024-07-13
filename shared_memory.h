#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#endif

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 1024
#define SEM_NAME "/my_semaphore"

#ifdef _WIN32
typedef struct _tag_shared_memory
{
    HANDLE hMapFile;
    HANDLE hEvent;
    LPCTSTR pBuf;
} shared_memory;
#else
typedef struct _tag_shared_memory
{
    int shm_fd;
    char* shm_base;
    sem_t* sem;
} shared_memory;
#endif

void shared_memory_init(shared_memory* shm);
void shared_memory_write(shared_memory* shm, const char* message);
void shared_memory_read(shared_memory* shm, char* buffer);
void shared_memory_release(shared_memory* shm);

#endif
