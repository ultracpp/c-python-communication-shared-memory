'''
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
'''
import mmap
import sys
import os

SHM_NAME = "/my_shared_memory"
SHM_SIZE = 4096
SEM_READ_NAME = "/my_semaphore_read"
SEM_WRITE_NAME = "/my_semaphore_write"

def init_shared_memory(mode):
	if sys.platform == "win32":
		import mmap
		#from mmap import ACCESS_READ, ACCESS_WRITE
		from mmap import ACCESS_DEFAULT
		import win32event

		#hMapFile = mmap.mmap(-1, SHM_SIZE, tagname=SHM_NAME, access=ACCESS_READ if mode == 'read' else ACCESS_WRITE)
		hMapFile = mmap.mmap(-1, SHM_SIZE, tagname=SHM_NAME, access=ACCESS_DEFAULT)
		hEventRead = win32event.CreateEvent(None, False, True, SEM_READ_NAME)
		hEventWrite = win32event.CreateEvent(None, False, False, SEM_WRITE_NAME)

		return hMapFile, hEventRead, hEventWrite
	else:
		import posix_ipc
		import mmap

		# 공유 메모리 크기를 처음부터 설정
		shm = posix_ipc.SharedMemory(SHM_NAME, posix_ipc.O_CREAT, size=SHM_SIZE)
		sem_read = posix_ipc.Semaphore(SEM_READ_NAME, posix_ipc.O_CREAT, initial_value=0)
		sem_write = posix_ipc.Semaphore(SEM_WRITE_NAME, posix_ipc.O_CREAT, initial_value=0)

		if mode == 'read':
			m = mmap.mmap(shm.fd, SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ)
			sem_write.release()
			return m, sem_read, sem_write
		elif mode == 'write':
			m = mmap.mmap(shm.fd, SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
			sem_read.release()
			return m, sem_read, sem_write
		else:
			raise ValueError("Invalid mode. Use 'read' or 'write'.")

		shm.close_fd()  # 매핑 후 파일 디스크립터 닫기

def read_from_shared_memory(shm):
	try:
		message = shm.read(SHM_SIZE).rstrip(b'\x00')
		shm.seek(0)
		return message.decode('utf-8')
	except Exception as e:
		print(f"Error reading shared memory: {e}")
		return None

def write_to_shared_memory(shm, message):
	try:
		message_bytes = message.encode('utf-8')
		shm.seek(0)
		shm.write(message_bytes.ljust(SHM_SIZE, b'\x00'))
		shm.seek(0)
	except Exception as e:
		print(f"Error writing to shared memory: {e}")

def release_shared_memory(shm, event):
	if sys.platform == "win32":
		shm.close()
	else:
		shm.close()
		event.close()
