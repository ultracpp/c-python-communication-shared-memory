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
from shared_memory import init_shared_memory, read_from_shared_memory, write_to_shared_memory, release_shared_memory

def test_read(shm, sem_read, sem_write):
    if sys.platform == "win32":
        import win32event
        while True:
            win32event.WaitForSingleObject(sem_write, win32event.INFINITE)
            message = read_from_shared_memory(shm)
            win32event.SetEvent(sem_read)
            if message:
                print(f"Received message: {message}")
    else:
        while True:
            sem_write.acquire()
            message = read_from_shared_memory(shm)
            sem_read.release()
            if message:
                print(f"Received message: {message}")

def test_write(shm, sem_read, sem_write):
    idx = 0
    try:
        if sys.platform == "win32":
            import win32event
            while True:
                message_to_write = f"Hello from Python! {idx}"
                win32event.WaitForSingleObject(sem_read, win32event.INFINITE)
                write_to_shared_memory(shm, message_to_write)
                win32event.SetEvent(sem_write)
                idx += 1
                time.sleep(0.001)
        else:
            while True:
                message_to_write = f"Hello from Python! {idx}"
                sem_read.acquire()
                write_to_shared_memory(shm, message_to_write)
                sem_write.release()
                idx += 1
                time.sleep(0.001)
    except KeyboardInterrupt:
        print("Write test stopped by user")

def main(mode):
    shm, sem_read, sem_write = init_shared_memory(mode)
    try:
        if mode == 'read':
            test_read(shm, sem_read, sem_write)
        elif mode == 'write':
            test_write(shm, sem_read, sem_write)
        else:
            raise ValueError("Invalid mode. Use 'read' or 'write'.")
    finally:
        release_shared_memory(shm, sem_read)

if __name__ == "__main__":
    mode = 'read'
    main(mode)
