#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline short host_to_network_short(short host)
{
	return ((host & 0xFF) << 8) | ((host >> 8) & 0xFF);
}

static inline short network_to_host_short(short network)
{
	return ((network & 0xFF) << 8) | ((network >> 8) & 0xFF);
}

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

	/*shm->hSemRead = CreateSemaphore(NULL, 1, 1, SEM_READ_NAME);
	shm->hSemWrite = CreateSemaphore(NULL, 0, 1, SEM_WRITE_NAME);

	if (shm->hSemRead == NULL || shm->hSemWrite == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
		UnmapViewOfFile(shm->pBuf);
		CloseHandle(shm->hMapFile);
		exit(1);
	}*/

	shm->hSemRead = CreateEvent(NULL, FALSE, TRUE, SEM_READ_NAME);
	shm->hSemWrite = CreateEvent(NULL, FALSE, FALSE, SEM_WRITE_NAME);

	if (shm->hSemRead == NULL || shm->hSemWrite == NULL)
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

	shm->sem_read = sem_open(SEM_READ_NAME, O_CREAT, 0644, 1);
	shm->sem_write = sem_open(SEM_WRITE_NAME, O_CREAT, 0644, 0);

	if (shm->sem_read == SEM_FAILED || shm->sem_write == SEM_FAILED)
	{
		perror("sem_open");
		munmap(shm->shm_base, SHM_SIZE);
		close(shm->shm_fd);
		exit(1);
	}
#endif
}

void shared_memory_read(shared_memory* shm, char* buffer)
{
#ifdef _WIN32
	/*DWORD dwWaitResult = WaitForSingleObject(shm->hSemWrite, INFINITE);
	if (dwWaitResult != WAIT_OBJECT_0)
	{
		printf("WaitForSingleObject error: %d\n", GetLastError());
		return;
	}

	strncpy(buffer, (char*)shm->pBuf, SHM_SIZE);

	if (!ReleaseSemaphore(shm->hSemRead, 1, NULL))
	{
		printf("ReleaseSemaphore error: %d\n", GetLastError());
	}*/

	WaitForSingleObject(shm->hSemWrite, INFINITE);
	strncpy(buffer, (char*)shm->pBuf, SHM_SIZE);
	SetEvent(shm->hSemRead);
#else
	sem_wait(shm->sem_write);
	strncpy(buffer, shm->shm_base, SHM_SIZE);
	sem_post(shm->sem_read);
#endif
}

void shared_memory_write(shared_memory* shm, const char* message)
{
#ifdef _WIN32
	/*DWORD dwWaitResult = WaitForSingleObject(shm->hSemRead, INFINITE);
	if (dwWaitResult != WAIT_OBJECT_0)
	{
		printf("WaitForSingleObject error: %d\n", GetLastError());
		return;
	}

	strncpy((char*)shm->pBuf, message, SHM_SIZE);

	if (!ReleaseSemaphore(shm->hSemWrite, 1, NULL))
	{
		printf("ReleaseSemaphore error: %d\n", GetLastError());
	}*/

	WaitForSingleObject(shm->hSemRead, INFINITE);
	strncpy((char*)shm->pBuf, message, SHM_SIZE);
	SetEvent(shm->hSemWrite);
#else
	sem_wait(shm->sem_read);
	strncpy(shm->shm_base, message, SHM_SIZE);
	sem_post(shm->sem_write);
#endif
}

void shared_memory_read_binary(shared_memory* shm, char* buffer, short* length)
{
#ifdef _WIN32
	WaitForSingleObject(shm->hSemWrite, INFINITE);
	*length = network_to_host_short(*(short*)shm->pBuf);
	memcpy(buffer, (char*)shm->pBuf + 2, *length);
	SetEvent(shm->hSemRead);
#else
	sem_wait(shm->sem_write);
	*length = network_to_host_short(*(short*)shm->shm_base);
	memcpy(buffer, shm->shm_base + 2, *length);
	sem_post(shm->sem_read);
#endif
}

void shared_memory_write_binary(shared_memory* shm, const char* data, short length)
{
#ifdef _WIN32
	WaitForSingleObject(shm->hSemRead, INFINITE);
	*(short*)shm->pBuf = host_to_network_short(length);
	memcpy((char*)shm->pBuf + 2, data, length);
	SetEvent(shm->hSemWrite);
#else
	sem_wait(shm->sem_read);
	*(short*)shm->shm_base = host_to_network_short(length);
	memcpy(shm->shm_base + 2, data, length);
	sem_post(shm->sem_write);
#endif
}

void shared_memory_release(shared_memory* shm)
{
#ifdef _WIN32
	CloseHandle(shm->hSemRead);
	CloseHandle(shm->hSemWrite);
	UnmapViewOfFile(shm->pBuf);
	CloseHandle(shm->hMapFile);
#else
	munmap(shm->shm_base, SHM_SIZE);
	close(shm->shm_fd);
	sem_close(shm->sem_read);
	sem_close(shm->sem_write);
	sem_unlink(SEM_READ_NAME);
	sem_unlink(SEM_WRITE_NAME);
	shm_unlink(SHM_NAME);
#endif
}
