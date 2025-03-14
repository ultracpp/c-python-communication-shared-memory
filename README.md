# C and Python Communication using Shared Memory

This project demonstrates interprocess communication (IPC) between C and Python using shared memory. Shared memory allows for high-speed data transfer between processes, making it suitable for applications requiring efficient communication.

## Components

### C Program (`shared_memory`)

The C program consists of two source files:

- **`shared_memory.c`**: Initializes shared memory and a semaphore for synchronization. Provides functions to write data to and read data from shared memory.
- **`main.c`**: Contains main functionalities related to data processing and shared memory interactions.

#### Functions in `shared_memory.c`:

- **shared_memory_init**: Initializes shared memory and semaphore.
- **shared_memory_write**: Writes a message to shared memory.
- **shared_memory_read**: Reads a message from shared memory.
- **shared_memory_release**: Releases shared memory and semaphore resources.

#### Functions in `main.c`

- **test_write**: Continuously writes messages to shared memory.
- **test_read**: Continuously reads messages from shared memory.
- **batch_write**: Write messages continuously in batches to shared memory.
- **batch_read**: Read messages continuously in batches from shared memory.

### Python Program (`shared_memory.py`)

The Python script (`shared_memory.py`) interacts with the shared memory created by the C program. It uses the `mmap` module for memory mapping and `posix_ipc` module for semaphore handling. For Windows environments, `win32event` is used for event-based synchronization.

- **init_shared_memory**: Initializes shared memory and semaphore based on the platform (Windows or POSIX).
- **write_to_shared_memory**: Writes a message to shared memory.
- **read_from_shared_memory**: Reads a message from shared memory.
- **release_shared_memory**: Releases shared memory and semaphore resources when done with IPC operations.

### Python Program (`main.py`)

The Python script (`main.py`) contains main functionalities related to data processing and shared memory interactions.

- **test_write**: Continuously writes messages to shared memory.
- **test_read**: Continuously reads messages from shared memory.
- **batch_write**: Write messages continuously in batches to shared memory.
- **batch_read**: Read messages continuously in batches from shared memory.

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

1. **Compile C Programs**:
   - **POSIX (Linux)**: Ensure `shared_memory.c` and `main.c` are compiled and built (`gcc -o shared_memory shared_memory.c main.c -lrt`).
   - **Windows (MSVC)**: Use Visual Studio Command Prompt or Developer Command Prompt for Visual Studio to compile (`cl shared_memory.c main.c`) and link (`link shared_memory.obj main.obj`) the program.

2. **Run Python Script**:
   - Install required Python packages (`pip install posix_ipc` for POSIX systems and `pip install pywin32` for Windows), and additional libraries for Kafka or MQTT integration.
   - Execute `main.py` based on the required functionalities.

3. **Cross-Compilation between Windows and Linux**:
   - This project supports cross-compilation, enabling seamless operation across both Windows and Linux platforms. Ensure appropriate toolchains and dependencies are configured for each platform.

4. **Integration with Kafka or MQTT**:
   - Follow respective library documentation for setting up Kafka or MQTT topics and message passing.
   - Ensure both C and Python programs are configured to correctly produce and consume messages.

## Notes

- **Performance Considerations**: 
  - The C program ensures efficient shared memory handling and synchronization using platform-specific methods (`mmap`, `semaphore`).
  - Python's Global Interpreter Lock (GIL) limitation is bypassed by leveraging separate processes for C and Python.

- **Contributions and Issues**:
  - Contributions, issues, and feedback are welcome. Feel free to fork this repository, make improvements, and submit pull requests.
