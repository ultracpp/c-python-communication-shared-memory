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
import sys
import time
from shared_memory import init_shared_memory, release_shared_memory
from shared_memory import read_from_shared_memory, write_to_shared_memory
from shared_memory import read_binary_from_shared_memory, write_binary_to_shared_memory
from shared_memory import SHM_SIZE

def wait_semaphore(sem, is_win32):
	if is_win32:
		import win32event
		win32event.WaitForSingleObject(sem, win32event.INFINITE)
	else:
		sem.acquire()

def signal_semaphore(sem, is_win32):
	if is_win32:
		import win32event
		win32event.SetEvent(sem)
	else:
		sem.release()

def test_read(shm, sem_read, sem_write, is_win32):
	while True:
		wait_semaphore(sem_write, is_win32)
		message = read_from_shared_memory(shm)
		signal_semaphore(sem_read, is_win32)
		if message:
			print(f"Received message: {message}")

def test_write(shm, sem_read, sem_write, is_win32):
	idx = 0
	try:
		while True:
			message_to_write = f"Hello from Python! {idx}"
			wait_semaphore(sem_read, is_win32)
			write_to_shared_memory(shm, message_to_write)
			signal_semaphore(sem_write, is_win32)
			idx += 1
			time.sleep(0.001)
	except KeyboardInterrupt:
		print("Write test stopped by user")

def test_read_binary(shm, sem_read, sem_write, is_win32):
	while True:
		wait_semaphore(sem_write, is_win32)
		data = read_binary_from_shared_memory(shm)
		signal_semaphore(sem_read, is_win32)
		if data:
			print(f"Received binary data: {list(data)} (length: {len(data)})")

def test_write_binary(shm, sem_read, sem_write, is_win32):
	idx = 0
	try:
		while True:
			message_to_write = bytes([idx % 256, 0xAA, 0xBB, 0xCC])
			wait_semaphore(sem_read, is_win32)
			write_binary_to_shared_memory(shm, message_to_write)
			signal_semaphore(sem_write, is_win32)
			idx += 1
			time.sleep(0.001)
	except KeyboardInterrupt:
		print("Binary write test stopped by user")

def batch_write(shm, sem_read, sem_write, is_win32):
	buffer = []
	buffer_size = 0
	last_flush_time = time.time()
	idx = 0

	try:
		while True:
			message = f"Hello from Python! {idx}\n"
			msg_len = len(message)

			if buffer_size + msg_len + 1 >= SHM_SIZE or (time.time() - last_flush_time) >= 1.0:
				wait_semaphore(sem_read, is_win32)
				write_to_shared_memory(shm, ''.join(buffer) + '\0')  # null 문자 추가
				signal_semaphore(sem_write, is_win32)
				buffer = []
				buffer_size = 0
				last_flush_time = time.time()

			buffer.append(message)
			buffer_size += msg_len
			idx += 1
			time.sleep(0.001)
	except KeyboardInterrupt:
		if buffer_size > 0:
			wait_semaphore(sem_read, is_win32)
			write_to_shared_memory(shm, ''.join(buffer) + '\0')  # null 문자 추가
			signal_semaphore(sem_write, is_win32)

def batch_read(shm, sem_read, sem_write, is_win32):
	while True:
		wait_semaphore(sem_write, is_win32)
		message = read_from_shared_memory(shm)
		signal_semaphore(sem_read, is_win32)
		if message:
			print("================")
			print(f"{message}")
			for line in message.split('\n'):
				if line:
					print(f"{line}")

def main(mode):
	if mode == 'bin_read' or mode == 'batch_read':
		init_mode = 'read'
	elif mode == 'bin_write' or mode == 'batch_write':
		init_mode = 'write'
	else:
		init_mode = mode

	is_win32 = sys.platform == "win32"
	shm, sem_read, sem_write = init_shared_memory(init_mode)
	try:
		if mode == 'read':
			test_read(shm, sem_read, sem_write, is_win32)
		elif mode == 'write':
			test_write(shm, sem_read, sem_write, is_win32)
		elif mode == 'bin_read':
			test_read_binary(shm, sem_read, sem_write, is_win32)
		elif mode == 'bin_write':
			test_write_binary(shm, sem_read, sem_write, is_win32)
		elif mode == 'batch_write':
			batch_write(shm, sem_read, sem_write, is_win32)
		elif mode == 'batch_read':
			batch_read(shm, sem_read, sem_write, is_win32)
		else:
			raise ValueError("Invalid mode. Use 'read', 'write', 'batch_read', or 'batch_write'.")
	finally:
		release_shared_memory(shm, sem_read)

if __name__ == "__main__":
	mode = 'batch_read'  # 기본 모드 설정
	#mode = 'batch_write'
	#mode = 'bin_read'
	#mode = 'bin_write'
	#mode = 'read'
	#mode = 'write'
	main(mode)

