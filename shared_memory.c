#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void shared_memory_init(shared_memory* shm)
{
#ifdef _WIN32
    shm->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHM_SIZE, SHM_NAME);

    if (shm->hMapFile == NULL)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        exit(1);
    }

    shm->pBuf = (LPTSTR)MapViewOfFile(shm->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHM_SIZE);

    if (shm->pBuf == NULL)
    {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(shm->hMapFile);
        exit(1);
    }

    shm->hEvent = CreateEvent(NULL, FALSE, FALSE, SEM_NAME);

    if (shm->hEvent == NULL)
    {
        printf("CreateEvent error: %d\n", GetLastError());
        UnmapViewOfFile(shm->pBuf);
        CloseHandle(shm->hMapFile);
        exit(1);
    }
#else
    shm->shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (shm->shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm->shm_fd, SHM_SIZE) == -1)
    {
        perror("ftruncate");
        close(shm->shm_fd);
        exit(1);
    }

    shm->shm_base = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm->shm_fd, 0);

    if (shm->shm_base == MAP_FAILED)
    {
        perror("mmap");
        close(shm->shm_fd);
        exit(1);
    }

    shm->sem = sem_open(SEM_NAME, O_CREAT, 0644, 0);

    if (shm->sem == SEM_FAILED)
    {
        perror("sem_open");
        munmap(shm->shm_base, SHM_SIZE);
        close(shm->shm_fd);
        exit(1);
    }
#endif
}

void shared_memory_write(shared_memory* shm, const char* message)
{
#ifdef _WIN32
    strncpy((char*)shm->pBuf, message, SHM_SIZE);
    SetEvent(shm->hEvent);
#else
    strncpy(shm->shm_base, message, SHM_SIZE);
    sem_post(shm->sem);
#endif
}

void shared_memory_read(shared_memory* shm, char* buffer)
{
#ifdef _WIN32
    WaitForSingleObject(shm->hEvent, INFINITE);
    strncpy(buffer, (char*)shm->pBuf, SHM_SIZE);
#else
    sem_wait(shm->sem);
    strncpy(buffer, shm->shm_base, SHM_SIZE);
#endif
}

void shared_memory_release(shared_memory* shm)
{
#ifdef _WIN32
    UnmapViewOfFile(shm->pBuf);
    CloseHandle(shm->hMapFile);
    CloseHandle(shm->hEvent);
#else
    munmap(shm->shm_base, SHM_SIZE);
    close(shm->shm_fd);
    sem_close(shm->sem);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);
#endif
}
