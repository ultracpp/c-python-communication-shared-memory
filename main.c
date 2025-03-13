/*
 * c-python-communication-shared-memory - IPC between C and Python
 * Copyright (c) 2024 Eungsuk Jeon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

#include "shared_memory.h"

#ifdef _DEBUG
#include <vld.h>
#endif

void test_write(shared_memory* shm);
void test_read(shared_memory* shm);
void test_write_binary(shared_memory* shm);
void test_read_binary(shared_memory* shm);
void batch_write(shared_memory* shm);
void batch_read(shared_memory* shm);
#ifndef _WIN32
void sig_handler(int signo);
#endif

shared_memory shm;
char global_buffer[SHM_SIZE];
size_t buffer_offset = 0;
time_t last_flush_time;

static void flush_buffer(shared_memory* shm)
{
	if (buffer_offset > 0)
	{
		global_buffer[buffer_offset] = '\0';
		shared_memory_write(shm, global_buffer);
		memset(global_buffer, 0, SHM_SIZE);
		buffer_offset = 0;
		last_flush_time = time(NULL);
	}
}

int main(int argc, char* argv[])
{
	char* mode = argc > 1 ? argv[1] : "batch_write";

#ifndef _WIN32
	if (strcmp(mode, "read") == 0 || strcmp(mode, "batch_read") == 0)
	{
		signal(SIGINT, sig_handler);
	}
#endif

	shared_memory_init(&shm);

	memset(global_buffer, 0, SHM_SIZE);
	last_flush_time = time(NULL);

	if (strcmp(mode, "write") == 0)
	{
		test_write(&shm);
	}
	else if (strcmp(mode, "read") == 0)
	{
		test_read(&shm);
	}
	else if (strcmp(mode, "bin_write") == 0)
	{
		test_write_binary(&shm);
	}
	else if (strcmp(mode, "bin_read") == 0)
	{
		test_read_binary(&shm);
	}
	else if (strcmp(mode, "batch_write") == 0)
	{
		batch_write(&shm);
	}
	else if (strcmp(mode, "batch_read") == 0)
	{
		batch_read(&shm);
	}
	else
	{
		printf("Invalid mode. Use 'write', 'read', 'batch_write', or 'batch_read'.\n");
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

	for (int i = 0; i < 10000; i++)
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

	for (int i = 0; i < 10000; i++)
	{
		shared_memory_read(shm, buffer);
		printf("%s\n", buffer);
	}
}

void test_write_binary(shared_memory* shm)
{
	char data[SHM_SIZE - 2];
	int idx = 0;

	for (int i = 0; i < 10000; i++)
	{
		data[0] = idx % 256;
		data[1] = 0xAA;
		data[2] = 0xBB;
		data[3] = 0xCC;
		short length = 4;

		shared_memory_write_binary(shm, data, length);
		idx++;

#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}

void test_read_binary(shared_memory* shm)
{
	char buffer[SHM_SIZE - 2];
	short length;

	for (int i = 0; i < 10000; i++)
	{
		shared_memory_read_binary(shm, buffer, &length);

		if (length > 0)
		{
			printf("Received binary data (length=%d): ", length);
			for (int j = 0; j < length; j++)
			{
				printf("%02x ", (unsigned char)buffer[j]);
			}
			printf("\n");
		}
	}
}

void batch_write(shared_memory* shm)
{
	char message[SHM_SIZE];
	int idx = 0;

	for (int i = 0; i < 100000; i++)
	{
		time_t current_time = time(NULL);
		snprintf(message, SHM_SIZE, "Hello from C! %d", idx++);
		size_t msg_len = strlen(message);

		if (buffer_offset + msg_len + 1 >= SHM_SIZE || difftime(current_time, last_flush_time) >= 1.0)
		{
			//printf("%d\n", buffer_offset + msg_len + 1);
			flush_buffer(shm);
		}

		memcpy(global_buffer + buffer_offset, message, msg_len);
		buffer_offset += msg_len;
		global_buffer[buffer_offset++] = '\n';
	}

	if (buffer_offset > 0)
	{
		flush_buffer(shm);
	}
}

void batch_read(shared_memory* shm)
{
	char buffer[SHM_SIZE];

	while (1)
	//for (int i = 0, j = 10; i < j; i++)
	{
		memset(buffer, 0, SHM_SIZE);
		shared_memory_read(shm, buffer);
		printf("================\n");

		char* token = strtok(buffer, "\n");
		while (token)
		{
			printf("%s\n", token);
			token = strtok(NULL, "\n");
		}

#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}

#ifndef _WIN32
void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("\nReleasing shared memory...\n");
		shared_memory_release(&shm);
		exit(1);
	}
}
#endif
