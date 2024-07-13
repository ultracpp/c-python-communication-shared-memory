# C and Python Communication using Shared Memory

This project demonstrates interprocess communication (IPC) between C and Python using shared memory. Shared memory allows for high-speed data transfer between processes, making it suitable for applications requiring efficient communication.

## Components

### C Program (`shared_memory`)

The C program (`shared_memory.c` and `shared_memory.h`) initializes shared memory and a semaphore for synchronization. It provides functions to write data to and read data from shared memory.

- **shared_memory_init**: Initializes shared memory and semaphore.
- **shared_memory_write**: Writes a message to shared memory.
- **shared_memory_read**: Reads a message from shared memory.
- **shared_memory_release**: Releases shared memory and semaphore resources.

### Python Program (`test_shm.py`)

The Python script (`test_shm.py`) interacts with the shared memory created by the C program. It uses the `mmap` module for memory mapping and `posix_ipc` module for semaphore handling.

- **init_shared_memory**: Initializes shared memory and semaphore based on the platform (Windows or POSIX).
- **write_to_shared_memory**: Writes a message to shared memory.
- **read_from_shared_memory**: Reads a message from shared memory.
- **test_write**: Continuously writes messages to shared memory.
- **test_read**: Continuously reads messages from shared memory.

### Integration with Kafka or MQTT

#### Kafka Integration

To integrate with Kafka for message passing between C and Python:

- **C Program**: Utilize a Kafka C library such as librdkafka to produce or consume messages from Kafka topics.
- **Python Program**: Use the `confluent-kafka-python` library to consume or produce messages from Kafka topics in Python.

#### MQTT Integration

To integrate with MQTT for message passing between C and Python:

- **C Program**: Utilize an MQTT C library such as Eclipse Paho MQTT C to publish or subscribe to MQTT topics.
- **Python Program**: Use the `paho-mqtt` library to publish or subscribe to MQTT topics in Python.

### Machine Learning or Deep Learning Integration

After receiving and preprocessing messages from Kafka or MQTT, the Python program can perform machine learning or deep learning tasks using libraries such as TensorFlow, PyTorch, or scikit-learn.

## Usage

1. **Compile and Run C Program**:
   - Ensure `shared_memory.c` and `shared_memory.h` are compiled and built (`gcc -o shared_memory shared_memory.c -lrt`).
   - Execute the compiled C program. This program should be run first before running the Python script.

2. **Run Python Script**:
   - Install required Python packages (`pip install posix_ipc` for POSIX systems and additional libraries for Kafka or MQTT integration).
   - Execute `test_shm.py` with appropriate permissions (`python test_shm.py`).

3. **Execution Order**:
   - It is crucial to execute the C program (`shared_memory`) before running the Python script (`test_shm.py`). The C program initializes the shared memory region and semaphore, which the Python script then interacts with.

4. **Cross-Compilation between Windows and Linux**:
   - This project supports cross-compilation, enabling seamless operation across both Windows and Linux platforms. Ensure appropriate toolchains and dependencies are configured for each platform.

5. **Integration with Kafka or MQTT**:
   - Follow respective library documentation for setting up Kafka or MQTT topics and message passing.
   - Ensure both C and Python programs are configured to correctly produce and consume messages.

## Notes

- **Performance Considerations**: 
  - The C program ensures efficient shared memory handling and synchronization using platform-specific methods (`mmap`, `semaphore`).
  - Python's Global Interpreter Lock (GIL) limitation is bypassed by leveraging separate processes for C and Python.

- **Contributions and Issues**:
  - Contributions, issues, and feedback are welcome. Feel free to fork this repository, make improvements, and submit pull requests.
