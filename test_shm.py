import mmap
import sys
import time
import os

SHM_NAME = "/my_shared_memory"
SHM_SIZE = 1024
SEM_NAME = "/my_semaphore"

def init_shared_memory(mode):
    if sys.platform == "win32":
        import msvcrt
        from mmap import ACCESS_READ, ACCESS_WRITE, mmap
        import win32event

        if mode == 'read':
            hMapFile = mmap(-1, SHM_SIZE, tagname=SHM_NAME, access=ACCESS_READ)
        elif mode == 'write':
            hMapFile = mmap(-1, SHM_SIZE, tagname=SHM_NAME, access=ACCESS_WRITE)
        else:
            raise ValueError("Invalid mode. Use 'read' or 'write'.")

        hEvent = win32event.OpenEvent(win32event.EVENT_ALL_ACCESS, False, SEM_NAME)

        return hMapFile, hEvent
    else:
        import posix_ipc
        import mmap

        if mode == 'read':
            shm = posix_ipc.SharedMemory(SHM_NAME)
            m = mmap.mmap(shm.fd, SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ)
            sem = posix_ipc.Semaphore(SEM_NAME)
            shm.close_fd()
        elif mode == 'write':
            shm = posix_ipc.SharedMemory(SHM_NAME)
            m = mmap.mmap(shm.fd, SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
            sem = posix_ipc.Semaphore(SEM_NAME)
            shm.close_fd()
        else:
            raise ValueError("Invalid mode. Use 'read' or 'write'.")

        return m, sem

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
        import win32event
        win32event.CloseHandle(event)
    else:
        shm.close()
        event.close()

def test_read(shm, event):
    if sys.platform == "win32":
        import win32event
        while True:
            win32event.WaitForSingleObject(event, win32event.INFINITE)
            message = read_from_shared_memory(shm)
            if message:
                print(f"Received message: {message}")
    else:
        while True:
            event.acquire()
            message = read_from_shared_memory(shm)
            if message:
                print(f"Received message: {message}")

def test_write(shm, event):
    idx = 0
    try:
        if sys.platform == "win32":
            import win32event
            while True:
                message_to_write = f"Hello from Python! {idx}"
                write_to_shared_memory(shm, message_to_write)
                idx += 1

                win32event.SetEvent(event)
                time.sleep(0.001)
        else:
            while True:
                message_to_write = f"Hello from Python! {idx}"
                write_to_shared_memory(shm, message_to_write)
                idx += 1

                event.release()
                time.sleep(0.001)
    except KeyboardInterrupt:
        print("Write test stopped by user")

def main(mode):
    shm, event = init_shared_memory(mode)
    try:
        if mode == 'read':
            test_read(shm, event)
        elif mode == 'write':
            test_write(shm, event)
        else:
            raise ValueError("Invalid mode. Use 'read' or 'write'.")
    finally:
        release_shared_memory(shm, event)

if __name__ == "__main__":
    #mode = 'write'
    mode = 'read'
    main(mode)
