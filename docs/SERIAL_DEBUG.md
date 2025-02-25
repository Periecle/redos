# Serial Debugging for RedOS

This document explains how to use the serial debugging feature in RedOS to capture kernel debug messages in QEMU.

## Overview

RedOS now supports outputting debug and kernel messages to the serial port. This is particularly useful for:

1. Capturing boot output
2. Debugging kernel panics and failures
3. Logging detailed system activity
4. Debugging when the VGA console is not available or corrupted

The implementation redirects all kernel output to both the VGA console and COM1 serial port, which can be captured by QEMU to a file or displayed in real-time.

## Debug Levels

The debug subsystem supports the following log levels:

- `DEBUG_LEVEL_NONE` (0): No debugging output
- `DEBUG_LEVEL_ERROR` (1): Critical errors only
- `DEBUG_LEVEL_WARNING` (2): Errors and warnings
- `DEBUG_LEVEL_INFO` (3): Informational messages (default)
- `DEBUG_LEVEL_DEBUG` (4): Detailed debug information
- `DEBUG_LEVEL_TRACE` (5): Very verbose trace messages

## Output Targets

Debug output can be directed to:

- `DEBUG_TARGET_NONE` (0x00): No output
- `DEBUG_TARGET_VGA` (0x01): VGA text console only
- `DEBUG_TARGET_SERIAL` (0x02): Serial port only
- `DEBUG_TARGET_ALL` (0xFF): All available targets

## Usage in Code

### Basic Logging

Use the appropriate function for your log level:

```c
debug_error("Critical error: %s", error_message);
debug_warning("Warning: %s", warning_message);
debug_info("System initialized with %d MB RAM", ram_mb);
debug_debug("Memory area: %x - %x", start_addr, end_addr);
debug_trace("Function entered with args: %d, %d", arg1, arg2);
```

### Generic Logging

You can also use the generic log function with an explicit level:

```c
debug_log(DEBUG_LEVEL_INFO, "Message with explicit level");
```

### Hex Dumps

For memory dumps:

```c
debug_hex_dump(memory_ptr, size);
```

### Controlling Debugging

You can control the debug level and output targets:

```c
// Set the debug level
debug_set_level(DEBUG_LEVEL_DEBUG);

// Set the debug output target
debug_set_target(DEBUG_TARGET_SERIAL);

// Get current settings
int current_level = debug_get_level();
int current_target = debug_get_target();
```

## Building and Running

### Building the kernel

No special build flags are needed; the serial debug feature is built in by default:

```
mkdir -p build && cd build
cmake ..
make
```

### Running with QEMU

Several custom targets have been added to simplify running QEMU with serial output:

1. **Run without serial logging**:
   ```
   make qemu
   ```

2. **Run with serial output to file**:
   ```
   make qemu-serial
   ```
   This redirects COM1 output to `build/serial.log`

3. **Run with serial output to console**:
   ```
   make qemu-serial-console
   ```
   This displays the serial output directly in the terminal

4. **Run in debug mode with serial logging**:
   ```
   make qemu-debug
   ```
   This enables additional QEMU debugging features

### Managing Log Files

1. **Clear log file**:
   ```
   make clear-log
   ```

2. **View log file**:
   ```
   make view-log
   ```

## QEMU Serial Output Options

When running QEMU manually, you can use these options to redirect serial output:

- **Output to file**:
  ```
  qemu-system-i386 -cdrom redos.iso -serial file:output.log
  ```

- **Output to console**:
  ```
  qemu-system-i386 -cdrom redos.iso -serial mon:stdio
  ```

- **Output to null device** (disable):
  ```
  qemu-system-i386 -cdrom redos.iso -serial none
  ```

- **Output to named pipe** (for real-time monitoring):
  ```
  mkfifo /tmp/serial-pipe
  qemu-system-i386 -cdrom redos.iso -serial pipe:/tmp/serial-pipe
  ```
  Then in another terminal:
  ```
  cat /tmp/serial-pipe
  ```

## Troubleshooting

- **No serial output**: Make sure the serial port is initialized correctly. Check if `serial_init_com1()` returns `true`.
- **Corrupted output**: Check baud rate settings. The default is 115200.
- **Output missing newlines**: Remember that serial terminals expect CRLF (`\r\n`) for proper line breaks.
