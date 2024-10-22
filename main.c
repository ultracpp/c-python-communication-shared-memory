#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "shared_memory.h"

void test_write(shared_memory* shm);
void test_read(shared_memory* shm);
#ifndef _WIN32
void sig_handler(int signo);
#endif

shared_memory shm;

int main(int argc, char* argv[])
{
    //char* mode = "write";
    char* mode = "write";

/*#ifndef _WIN32
    if (strcmp(mode, "read") == 0)
    {
        signal(SIGINT, sig_handler);
    }
#endif*/

    shared_memory_init(&shm);

    if (strcmp(mode, "write") == 0)
    {
        test_write(&shm);
    }
    else if (strcmp(mode, "read") == 0)
    {
        test_read(&shm);
    }
    else
    {
        printf("Invalid mode. Use 'write' or 'read'.\n");
        shared_memory_release(&shm);
        return 1;
    }

    shared_memory_release(&shm);
    return 0;
}

void test_write(shared_memory* shm)
{
    char message[SHM_SIZE];
    int idx = 0;

    while (1)
    {
        snprintf(message, SHM_SIZE, "Hello from C! %d", idx++);

        shared_memory_write(shm, message);

#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }
}

void test_read(shared_memory* shm)
{
    char buffer[SHM_SIZE];

    while (1)
    {
        shared_memory_read(shm, buffer);
        printf("Received message from shared memory: %s\n", buffer);
    }
}

#ifndef _WIN32
void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("\nReceived SIGINT signal, releasing shared memory...\n");
        shared_memory_release(&shm);
        exit(1);
    }
}
#endif
